#include <string.h>

#include "logconf.h"
#include "cee-utils.h"

#define CONFIG_FILE "test-logconf.json"

int main(void)
{
  FILE *fp = fopen(CONFIG_FILE, "rb");

  // initialize and link conf to a .config file
  struct logconf confA, confB, confC;
  logconf_setup(&confA, "TAG A", fp);
  logconf_branch(&confB, &confA, "BRANCH B");
  logconf_branch(&confC, &confA, "BRANCH C");

  // get some JSON field written in .config file
  struct sized_buffer level = logconf_get_field(&confA, "logging.level");
  logconf_trace(&confA, "Logging level: %.*s", (int)level.size, level.start);

  // test each conf
  logconf_error(&confA, "A");
  logconf_debug(&confB, "B");
  logconf_fatal(&confC, "C");

  // print to 'logging.filename' (@todo better function name?)
  logconf_http(
    &confA, 
    NULL,
    "TITLE1", 
    (struct sized_buffer){"HEADER_A", 8},
    (struct sized_buffer){"BODY_A", 6},
    "%s", "Hello");

  logconf_http(
    &confB, 
    NULL,
    "TITLE2", 
    (struct sized_buffer){"HEADER_B", 8},
    (struct sized_buffer){"BODY_B", 6},
    "%d", 1337);

  logconf_http(
    &confC, 
    NULL,
    "TITLE3", 
    (struct sized_buffer){"HEADER_C", 8},
    (struct sized_buffer){"BODY_C", 6},
    "%c", '7');

  // cleanup conf resources
  logconf_cleanup(&confA);
  logconf_cleanup(&confB);
  logconf_cleanup(&confC);

  fclose(fp);
}
