#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h> /* strcasecmp() */
#include <stdarg.h>
#include <pthread.h> /* pthread_self() */
#include <unistd.h> /* getpid() */

#include "logconf.h"

#include "cee-utils.h"
#include "json-actor.h"


static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static size_t g_counter;


static int
get_log_level(char level[])
{
  if (0 == strcasecmp(level, "TRACE")) return LOG_TRACE;
  if (0 == strcasecmp(level, "DEBUG")) return LOG_DEBUG;
  if (0 == strcasecmp(level, "INFO"))  return LOG_INFO;
  if (0 == strcasecmp(level, "WARN"))  return LOG_WARN;
  if (0 == strcasecmp(level, "ERROR")) return LOG_ERROR;
  if (0 == strcasecmp(level, "FATAL")) return LOG_FATAL;
  ERR("Log level doesn't exist: %s", level);
  return 0;// make compiler happy
}

static void
log_nocolor_cb(log_Event *ev)
{
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';

  fprintf(
    ev->udata, "%s|%010u %-5s %s:%d: ",
    buf, (unsigned)pthread_self(), level_strings[ev->level], ev->file, ev->line);

  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}

static void 
log_color_cb(log_Event *ev)
{
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';

  int tid_color;
  if (main_tid == pthread_self())
    tid_color = 31;
  else
    tid_color = 90;

  fprintf(
    ev->udata, "%s|\x1b[%dm%010u\x1b[0m %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
    buf, tid_color, (unsigned)pthread_self(), level_colors[ev->level], level_strings[ev->level],
    ev->file, ev->line);

  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}

void
log_http(
  struct logconf *config, 
  struct loginfo *p_info,
  char url[],
  struct sized_buffer header,
  struct sized_buffer body,
  char label_fmt[], ...)
{
  pthread_mutex_lock(&g_lock);
  size_t counter = ++g_counter;
  pthread_mutex_unlock(&g_lock);

  if (!config || !config->http.f) return;

  // Build 'label' string
  char label[512];
  va_list label_args;
  va_start(label_args, label_fmt);
  size_t ret = vsnprintf(label, sizeof(label), label_fmt, label_args);
  ASSERT_S(ret < sizeof(label), "Out of bounds write attempt");
  va_end(label_args);

  // Get timestamp string
  uint64_t tstamp_ms = cee_timestamp_ms();
  char timestr[64];
  cee_unix_ms_to_iso8601(timestr, sizeof(timestr), &tstamp_ms);

  // Print to output
  fprintf(config->http.f, 
    "%s [%s #TID%u] - %s - %s\n"
    "%.*s%s%.*s\n"
    "@@@_%zu_@@@\n",
// 1st LINE ARGS
    label, 
    config->tag, 
    (unsigned)pthread_self(), 
    timestr, 
    url,
// 2nd LINE ARGS
    (int)header.size, header.start,
    header.size ? "\n" : "",
    (int)body.size, body.start,
// 3rd LINE ARGS
    counter);

  fflush(config->http.f);

  // extract logging info if wanted
  if (p_info) {
    *p_info = (struct loginfo){
      .counter = counter,
      .tstamp_ms = tstamp_ms
    };
  }
}

void
logconf_setup(struct logconf *config, const char tag[], FILE* fp)
{
  memset(config, 0, sizeof *config);

  int ret = snprintf(config->tag, LOGCONF_TAG_LEN, "%s", tag);
  ASSERT_S(ret < LOGCONF_TAG_LEN, "Out of bounds write attempt");

  if (!fp) return; /* EARLY RETURN */


  struct {
    char level[16];
    bool quiet, use_color, overwrite;
    bool http_enable;
  } logging={0};

  config->file.start = cee_load_whole_file_fp(fp, &config->file.size);
  json_extract(config->file.start, config->file.size,
             "(logging.level):.*s"
             "(logging.filename):.*s"
             "(logging.quiet):b"
             "(logging.use_color):b"
             "(logging.overwrite):b"
             "(logging.http.enable):b"
             "(logging.http.filename):.*s"
             "(logging.http_dump.enable):b"
             "(logging.http_dump.filename):.*s",
             sizeof(logging.level), logging.level,
             sizeof(config->logger.fname), config->logger.fname,
             &logging.quiet,
             &logging.use_color,
             &logging.overwrite,
             &logging.http_enable,
             sizeof(config->http.fname), config->http.fname,
             &logging.http_enable,
             sizeof(config->http.fname), config->http.fname);

  /* SET LOGGER CONFIGS */
  if (!IS_EMPTY_STRING(config->logger.fname)) {
    config->logger.f = fopen(config->logger.fname, logging.overwrite ? "w+" : "a+");
    ASSERT_S(NULL != config->logger.f, "Could not create logger file");

    _log_add_callback(&config->L,
        logging.use_color ? &log_color_cb : &log_nocolor_cb, 
        config->logger.f, 
        get_log_level(logging.level));
  }

  /* SET HTTP DUMP CONFIGS */
  if (logging.http_enable && !IS_EMPTY_STRING(config->http.fname)) {
    config->http.f = fopen(config->http.fname, logging.overwrite ? "w+" : "a+");
    ASSERT_S(NULL != config->http.f, "Could not create http logger file");
  }

  config->pid  = getpid();

  // disable default log.c callbacks
  _log_set_quiet(&config->L, true);

  // make sure fatal still prints to stderr
  _log_add_callback(&config->L,
      logging.use_color ? &log_color_cb : &log_nocolor_cb, 
      stderr, 
      logging.quiet ? LOG_FATAL : get_log_level(logging.level));
}

void
logconf_branch(struct logconf *branch, struct logconf *orig, const char tag[]) 
{
  pthread_mutex_lock(&g_lock);
  memcpy(branch, orig, sizeof(struct logconf));
  pthread_mutex_unlock(&g_lock);

  branch->is_branch = 1;
  if (tag) {
    int ret = snprintf(branch->tag, LOGCONF_TAG_LEN, "%s", tag);
    ASSERT_S(ret < LOGCONF_TAG_LEN, "Out of bounds write attempt");
  }

  /* To avoid overwritting, child processes files must be unique,
   *    this will append the unique PID to the end of file names */
  branch->pid = getpid();
  if (branch->pid != orig->pid) {
    size_t len;

    len = strlen(orig->logger.fname);
    snprintf(branch->logger.fname + len, sizeof(branch->logger.fname) - len, "%ld", (long)branch->pid);

    len = strlen(orig->http.fname);
    snprintf(branch->http.fname + len, sizeof(branch->http.fname) - len, "%ld", (long)branch->pid);
  }
}

void
logconf_cleanup(struct logconf *config)
{
  if (!config->is_branch) {
    if (config->file.start)
      free(config->file.start);
    if (config->logger.f)
      fclose(config->logger.f);
    if (config->http.f)
      fclose(config->http.f);
  }
  memset(config, 0, sizeof *config);
}

struct sized_buffer
logconf_get_field(struct logconf *config, char *json_field)
{
  struct sized_buffer field={0};
  if (!config->file.size) return field; // empty field

  char fmt[512];
  int ret = snprintf(fmt, sizeof(fmt), "(%s):T", json_field);
  ASSERT_S(ret < sizeof(fmt), "Out of bounds write attempt");

  json_extract(config->file.start, config->file.size, fmt, &field);

  return field;
}
