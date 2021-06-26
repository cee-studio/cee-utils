#include <string.h>

#include "logconf.h"
#include "cee-utils.h"

const char bot_config[] =           \
"{"                                 \
"  \"logging\": {"                  \
"    \"level\": \"trace\","         \
"    \"filename\": \"bot.log\","    \
"    \"quiet\": true,"              \
"    \"overwrite\": false,"         \
"    \"use_color\": true,"          \
"    \"http_dump\": {"              \
"      \"enable\": true,"           \
"      \"filename\": \"dump.json\"" \
"    }"                             \
"  }"                               \
"}";

int main(void)
{
  // initialize and link conf to a .config file
  struct logconf conf={0};
  logconf_setup(&conf, NULL);
  conf.contents = strdup(bot_config);
  conf.len = sizeof(bot_config);

  char *varA = "Hello"; 
  int varB = 1337;
  struct { int x; } varC = { .x = 707 };

  // assign tags to variable unique mem address
  logconf_add_id(&conf, &varA, "INT A");
  logconf_add_id(&conf, &varB, "CHAR B");
  logconf_add_id(&conf, &varC, "STRUCT C");

  // get some JSON field written in .config file
  struct sized_buffer level = logconf_get_field(&conf, "logging.level");
  // print the field
  log_trace("Logging level: %.*s", (int)level.size, level.start);

  // print the tag by referencing to the variable unique address
  log_trace("char varA tag: %s", logconf_tag(&conf, &varA));
  log_trace("char varB tag: %s", logconf_tag(&conf, &varC));
  log_trace("char varC tag: %s", logconf_tag(&conf, &varB));

  // print to 'logging.filename' (@todo better function name?)
  log_http(
    &conf, 
    NULL,
    &varA, 
    "TITLE1", 
    (struct sized_buffer){"HEADER1", 7},
    (struct sized_buffer){"BODY1", 5},
    "%s", varA);

  log_http(
    &conf, 
    NULL,
    &varB, 
    "TITLE2", 
    (struct sized_buffer){"HEADER2", 7},
    (struct sized_buffer){"BODY2", 5},
    "%d", varB);

  log_http(
    &conf, 
    NULL,
    &varC, 
    "TITLE3", 
    (struct sized_buffer){"HEADER3", 7},
    (struct sized_buffer){"BODY3", 5},
    "%d", varC.x);

  // cleanup conf resources
  logconf_cleanup(&conf);
}
