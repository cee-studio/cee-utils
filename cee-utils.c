#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <math.h> //for round()
#include <limits.h>
#define _POSIX_THREAD_SAFE_FUNCTIONS
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include "cee-utils.h"
#include "json-scanf.h"
#include "json-actor-boxed.h" /* ja_str and functions */
#include "json-actor.h"
#include "debug.h"


char*
cee_load_whole_file_fp(FILE *fp, size_t *len)
{
  fseek(fp, 0, SEEK_END);
  long f_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *str = malloc(f_size);
  fread(str, 1, f_size, fp);

  if (len) {
    *len = f_size;
  }

  return str;
}

char*
cee_load_whole_file(const char filename[], size_t *len)
{
  FILE *fp = fopen(filename,"rb");
  VASSERT_S(NULL != fp, "%s '%s'\n", strerror(errno), filename);
  char *str = cee_load_whole_file_fp(fp, len);
  fclose(fp);
  return str;
}

int
cee_dati_from_fjson(
  char filename[], 
  void *p_data, 
  void (from_json_cb)(char *str, size_t len, void *p_data))
{
  size_t len;
  char *json = cee_load_whole_file(filename, &len);
  if (NULL == json) return 0;

  from_json_cb(json, len, p_data);

  free(json);

  return 1;
}

static char *
stat_to_type(const struct stat *st)
{
  switch (st->st_mode & S_IFMT) {
  case S_IFREG: return "file";
  case S_IFDIR: return "folder";
  default:      return NULL;
  }
}

int
cee_iso8601_to_unix_ms(char *timestamp, size_t len, void *p_data)
{
  uint64_t *recipient = (uint64_t*)p_data;
  ASSERT_S(NULL != recipient, "No recipient provided by user");

  struct tm tm;
  double seconds = 0;
  memset(&tm, 0, sizeof(tm));

  /* Creating a temporary buffer and copying the string, because
  sscanf receives a null-terminated string, and there's not
  "snscanf" or something like that */
  char *buf = malloc(len + 1);

  memcpy(buf, timestamp, len);
  buf[len] = '\0';

  char tz_operator = 'Z';
  int tz_hour = 0, tz_min = 0;
  sscanf(buf, "%d-%d-%dT%d:%d:%lf%c%d:%d", // ISO-8601 complete format
    &tm.tm_year, &tm.tm_mon, &tm.tm_mday, // Date
    &tm.tm_hour, &tm.tm_min, &seconds, // Time
    &tz_operator, &tz_hour, &tz_min); // Timezone

  free(buf);

  tm.tm_mon--; // struct tm takes month from 0 to 11
  tm.tm_year -= 1900; // struct tm takes years from 1900

  uint64_t res = (((uint64_t) mktime(&tm) - timezone) * 1000)
              + (uint64_t) round(seconds * 1000.0);
  switch (tz_operator) {
  case '+': // Add hours and minutes
      res += (tz_hour * 60 + tz_min) * 60 * 1000;
      break;
  case '-': // Subtract hours and minutes
      res -= (tz_hour * 60 + tz_min) * 60 * 1000;
      break;
  case 'Z': // UTC, don't do anything
  default: // @todo should we check for error ?
      break;
  }

  *recipient = res;

  return 1; // SUCCESS
}

int
cee_unix_ms_to_iso8601(char *str, size_t len, void *p_timestamp)
{
  ASSERT_S(NULL != p_timestamp, "Missing 'p_timestamp'");
  uint64_t timestamp = *(uint64_t*)p_timestamp;

  time_t seconds = timestamp / 1000;
  int millis = timestamp % 1000;

  seconds += timezone;
  struct tm buf;
  struct tm *tm = localtime_r(&seconds, &buf);

  return snprintf(str, len,
    "%d-%.2d-%dT%.2d:%.2d:%.2d.%.3dZ", // ISO-8601 complete format
    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, // Date
    tm->tm_hour, tm->tm_min, tm->tm_sec, millis); // Time
}

int
cee_strtoull(char *str, size_t len, void *p_data) 
{
  char *buf = malloc(len + 1);

  memcpy(buf, str, len);
  buf[len] = '\0';

  uint64_t *recipient = (uint64_t*)p_data;
  ASSERT_S(NULL != recipient, "No recipient provided by user");

  *recipient = strtoull(buf, NULL, 10);

  free(buf);

  return 1;
}

int
cee_ulltostr(char *str, size_t len, void *p_data) {
  return snprintf(str, len, "%" PRIu64 , *(uint64_t*)p_data);
}

void
cee_sleep_ms(const int64_t delay_ms)
{
  if (delay_ms < 0) return; /* EARLY RETURN */

  const struct timespec t = {
    .tv_sec = delay_ms / 1000,
    .tv_nsec = (delay_ms % 1000) * 1000000
  };
  nanosleep(&t, NULL);
}

/* returns current timestamp in milliseconds */
uint64_t
cee_timestamp_ms()
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME, &t);
  return (uint64_t)t.tv_sec*1000 + (uint64_t)t.tv_nsec/1000000;
}

char*
cee_timestamp_str(char *p_str, int len)
{
  time_t t = time(NULL);
  struct tm buf;
  struct tm *tm = localtime_r(&t, &buf);

  int ret = strftime(p_str, len, "%c", tm);
  ASSERT_S(ret != 0, "Could not retrieve string timestamp");

  return p_str;
}

/* this can be used for checking if a user-given string does not
 *  exceeds a arbitrary threshold length */
ssize_t
cee_str_bounds_check(const char *str, const size_t threshold_len)
{
  if (!str) return -1; // Missing string

  for (size_t i=0; i < threshold_len; ++i) {
    if ('\0' == str[i]) return i; // bound check succeeded
  }
  return 0; // bound check failed
}

char* 
cee_join_strings(char** strings, const size_t nmemb, const char delim[], const size_t wordlen, const size_t maxlen)
{
  char *buf = malloc(maxlen);
  char *cur = buf, * const end = cur + maxlen;

  for (size_t i=0; i < nmemb; ++i) {
    VASSERT_S(cee_str_bounds_check(strings[i], wordlen) > 0, \
        "'%s' exceeds threshold of %zu characters", strings[i], wordlen);
    cur += snprintf(cur, end-cur, "%s%s", strings[i], delim);
    ASSERT_S(cur < end, "Out of bounds write attempt");
  }
  *(cur - strlen(delim)) = '\0';

  return buf;
}

void cee_gen_readlink(char *linkbuf, size_t linkbuf_size)
{
  ssize_t r;
  r = readlink("/proc/self/exe", linkbuf, linkbuf_size);
  if (r < 0) {
    perror("readlink");
    exit(EXIT_FAILURE);
  }

  if (r > linkbuf_size) {
    fprintf(stderr, "symlink size is greater than %zu\n", linkbuf_size);
    exit(EXIT_FAILURE);
  }
  linkbuf[r]='\0';

  return;
}

void cee_gen_dirname(char *linkbuf) {
  *strrchr(linkbuf, '/')='\0';
}
