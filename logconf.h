#ifndef LOGCONF_H
#define LOGCONF_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h> /* uint64_t */
#include "ntl.h"    /* struct sized_buffer */
#include "debug.h"

#define logconf_trace(config, ...) logconf_log(config, LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define logconf_debug(config, ...) logconf_log(config, LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define logconf_info(config, ...)  logconf_log(config, LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define logconf_warn(config, ...)  logconf_log(config, LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define logconf_error(config, ...) logconf_log(config, LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define logconf_fatal(config, ...) logconf_log(config, LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define __logconf_log(config, level, file, line, fmt, ...) _log_log(&(config)->L, level, file, line, "[%s] "fmt"%s", (config)->tag, __VA_ARGS__)
#define logconf_log(config, level, file, line, ...) __logconf_log(config, level, file, line, __VA_ARGS__, "")

#define LOGCONF_TAG_LEN 64 + 1

struct logconf {
  _Bool is_branch;
  char tag[LOGCONF_TAG_LEN];
  log_Logger L;

  struct sized_buffer file;

  struct {
    char fname[4096];
    FILE *f;
  } logger, http; // 'log_xxx()' and 'log_http()' outputs

  unsigned pid;
};

struct loginfo {
  size_t counter;
  uint64_t tstamp_ms;
};

void logconf_setup(struct logconf*, const char tag[], FILE *fp);
void logconf_branch(struct logconf *branch, struct logconf *orig, const char tag[]);
void logconf_cleanup(struct logconf*);
struct sized_buffer logconf_get_field(struct logconf *config, char *json_field);

void log_http(
  struct logconf *config, 
  struct loginfo *info,
  char url[],
  struct sized_buffer header,
  struct sized_buffer body,
  char label_fmt[], ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LOGCONF_H
