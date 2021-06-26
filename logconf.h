#ifndef LOGCONF_H
#define LOGCONF_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h> /* uint64_t */
#include "ntl.h"    /* struct sized_buffer */
#include "debug.h"

#define MAX_LOGCONF_IDS 32

struct logconf {
  struct {
    void *addr;
    char tag[32];
  } ids[MAX_LOGCONF_IDS];

  char *contents; // config file contents
  size_t len; // config file len

  struct { /* the 'bot.log' and 'dump.json' file */
    FILE *f;
  } logger, http;
};

struct loginfo {
  size_t counter;
  uint64_t tstamp_ms;
};

void logconf_add_id(struct logconf *config, void *addr, const char tag[]);
char* logconf_tag(struct logconf *config, void *addr);
void logconf_setup(struct logconf*, const char config_file[]);
void logconf_cleanup(struct logconf*);
struct sized_buffer logconf_get_field(struct logconf *config, char *json_field);

void log_http(
  struct logconf *config, 
  struct loginfo *info,
  void *addr_id,
  char url[],
  struct sized_buffer header,
  struct sized_buffer body,
  char label_fmt[], ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LOGCONF_H
