#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h> /* strcasecmp() */
#include <stdarg.h>
#include <pthread.h> /* pthread_self() */
#include <limits.h> /* PATH_MAX */

#include "logconf.h"

#include "cee-utils.h"
#include "json-actor.h"


static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static size_t g_counter;
static bool g_first_run = true;
static pid_t g_main_pid; // initialized at logconf_setup()


static int
get_log_level(char level[])
{
  if (0 == strcasecmp(level, "TRACE")) return LOG_TRACE;
  if (0 == strcasecmp(level, "DEBUG")) return LOG_DEBUG;
  if (0 == strcasecmp(level, "INFO")) return LOG_INFO;
  if (0 == strcasecmp(level, "WARN")) return LOG_WARN;
  if (0 == strcasecmp(level, "ERROR")) return LOG_ERROR;
  if (0 == strcasecmp(level, "FATAL")) return LOG_FATAL;
  ERR("Log level doesn't exist: %s", level);
  return 0;// make compiler happy
}

void
logconf_add_id(struct logconf *config, void *addr, const char tag[])
{
  if (!config || !addr || IS_EMPTY_STRING(tag)) 
    return; /* EARLY RETURN */

  for (size_t i=0; i < MAX_LOGCONF_IDS; ++i) {
    if ( (NULL == config->ids[i].addr) || (addr == config->ids[i].addr) ) {
      config->ids[i].addr = addr;
      snprintf(config->ids[i].tag, sizeof(config->ids[i].tag), "%s", tag);
      return; /* EARLY RETURN */
    }
  }
  ERR("Reach maximum logconf_ids threshold (%d)", MAX_LOGCONF_IDS);
}

char*
logconf_tag(struct logconf *config, void *addr)
{
  for (size_t i=0; i < MAX_LOGCONF_IDS; ++i) {
    if (addr == config->ids[i].addr) {
      return config->ids[i].tag;
    }
  }
  return "NO_TAG";
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
  void *addr_id,
  char url[],
  struct sized_buffer header,
  struct sized_buffer body,
  char label_fmt[], ...)
{
  pthread_mutex_lock(&g_lock);
  size_t counter = ++g_counter;
  pthread_mutex_unlock(&g_lock);

  if (!config) return;

  char label[512];

  va_list label_args;
  va_start(label_args, label_fmt);

  size_t ret = vsnprintf(label, sizeof(label), label_fmt, label_args);
  ASSERT_S(ret < sizeof(label), "Out of bounds write attempt");

  va_end(label_args);

  struct loginfo info = {
    .counter = counter,
    .tstamp_ms = cee_timestamp_ms()
  };

  char timestr[64];
  cee_unix_ms_to_iso8601(timestr, sizeof(timestr), &info.tstamp_ms);

  fprintf(config->http.f, 
    "%s [%s #TID%u] - %s - %s\n%.*s%s%.*s\n@@@_%zu_@@@\n",
    label,
    logconf_tag(config, addr_id), 
    (unsigned)pthread_self(),
    timestr,
    url,
    (int)header.size, header.start,
    header.size ? "\n" : "",
    (int)body.size, body.start,
    info.counter);
  fflush(config->http.f);

  if (p_info) *p_info = info;
}

void
logconf_setup(struct logconf *config, const char config_file[])
{
  ASSERT_S(NULL != config, "Missing 'struct logconf'");

  if (IS_EMPTY_STRING(config_file)) {
    config->http.f = stderr;
    return; /* EARLY RETURN */
  }

  struct {
    char level[16];
    char filename[PATH_MAX];
    bool quiet, use_color, overwrite;
    struct {
      bool enable;
      char filename[PATH_MAX];
    } http;
  } logging={0}; // set all as zero


  if (config->contents) {
    free(config->contents);
    config->len = 0;
  }

  config->contents = cee_load_whole_file(config_file, &config->len);
  json_extract(config->contents, config->len,
             "(logging.level):s"
             "(logging.filename):s"
             "(logging.quiet):b"
             "(logging.use_color):b"
             "(logging.overwrite):b"
             "(logging.http_dump.enable):b"
             "(logging.http_dump.filename):s",
             logging.level,
             logging.filename,
             &logging.quiet,
             &logging.use_color,
             &logging.overwrite,
             &logging.http.enable,
             logging.http.filename);

  // child processes must be written to a different file
  pid_t pid = getpid();
  if (!g_first_run && pid != g_main_pid) 
  {
    char filename_ext[128];
    char aux[PATH_MAX]={0};

    snprintf(filename_ext, sizeof(filename_ext), ".%ld", (long)pid);

    char *ext_start;
    if (NULL == (ext_start = strchr(logging.filename, '.')))
      ext_start = "";

    snprintf(aux, PATH_MAX, "%.*s%s%s", \
        (int)(ext_start - logging.filename), logging.filename, filename_ext, ext_start);
    memcpy(logging.filename, aux, PATH_MAX);

    if (NULL == (ext_start = strchr(logging.http.filename, '.')))
      ext_start = "";

    snprintf(aux, PATH_MAX, "%.*s%s%s", \
        (int)(ext_start - logging.http.filename), logging.http.filename, filename_ext, ext_start);
    memcpy(logging.http.filename, aux, PATH_MAX);
  }

  /* SET LOGGER CONFIGS */
  if (!IS_EMPTY_STRING(logging.filename)) {
    if (g_first_run && logging.overwrite)
      config->logger.f = fopen(logging.filename, "w+");
    else
      config->logger.f = fopen(logging.filename, "a+");
    log_add_callback(
        logging.use_color ? &log_color_cb : &log_nocolor_cb, 
        config->logger.f, 
        get_log_level(logging.level));
    ASSERT_S(NULL != config->logger.f, "Could not create logger file");
  }

  /* SET HTTP DUMP CONFIGS */
  if (logging.http.enable && !IS_EMPTY_STRING(logging.http.filename)) {
    if (g_first_run && logging.overwrite)
      config->http.f = fopen(logging.http.filename, "w+");
    else
      config->http.f = fopen(logging.http.filename, "a+");
    ASSERT_S(NULL != config->http.f, "Could not create dump file");
  }

  if (g_first_run) {
    g_main_pid = getpid();

    log_set_quiet(true); // disable default log.c callbacks
    if (logging.quiet) // make sure fatal still prints to stderr
      log_add_callback(
          logging.use_color ? &log_color_cb : &log_nocolor_cb, 
          stderr, 
          LOG_FATAL);
    else
      log_add_callback(
          logging.use_color ? &log_color_cb : &log_nocolor_cb, 
          stderr, 
          get_log_level(logging.level));

    g_first_run = false;
  }
}

void
logconf_cleanup(struct logconf *config)
{
  if (config->contents)
    free(config->contents);
  if (config->logger.f)
    fclose(config->logger.f);
  if (config->http.f)
    fclose(config->http.f);
}

struct sized_buffer
logconf_get_field(struct logconf *config, char *json_field)
{
  struct sized_buffer field={0};
  if (!config->len) return field; // empty field

  char fmt[512];
  int ret = snprintf(fmt, sizeof(fmt), "(%s):T", json_field);
  ASSERT_S(ret < sizeof(fmt), "Out of bounds write attempt");

  json_extract(config->contents, config->len, fmt, &field);

  return field;
}
