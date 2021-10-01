#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "cee-utils.h"
#include "json-actor.h"

int main()
{
  int i, j, k;
  char * json = "{ \"a\":10 }";
  json_extract(json, strlen(json), "(a):d", &i);
  fprintf (stderr, "%d\n", i);
  assert(i == 10);


  json = "{ \"a\": { \"b\":11 }}";
  json_extract(json, strlen(json), "(a)(b):d", &i);
  fprintf (stderr, "%d\n", i);
  assert(i == 11);


  json = "{ \"a\": [ 13, 14, 15 ] }";
  json_extract(json, strlen(json), "(a)(0):d", &i);
  fprintf (stderr, "%d\n", i);
  assert(i == 13);


  json = "{ \"a\": [ { \"b\":123 }, 14, 15 ] }";
  json_extract(json, strlen(json), "(a)(0)(b):d", &i);
  fprintf (stderr, "%d\n", i);
  assert(i == 123);

  json = "[ 13, 14, 15 ]";

  json_extract(json, strlen(json), "[d, d, d]", &i, &j, &k);
  fprintf (stderr, "%d, %d, %d\n", i, j, k);

  char *key;
  bool  flag;
  json = "{\"a\":true}";
  json_extract(json, strlen(json), "(*):b", &key, &flag);
  fprintf (stderr, "%s:%d\n", key, flag);

  json = "{\n"
    "\t\"discord\": {\n"
    "\t\t   \"token\":\"YOUR-BOT-TOKEN\"\n"
    "\t},\n"
    "\t\"github\": {\n"
    "\t       \"username\": \"gituser\",\n"
    "\t       \"token\": \"xxxxxx\"\n"
    "\t},\n"
    "\t\"logging\": {\n"
    "\t\t   \"filename\": \"bot.log\",\n"
    "\t\t   \"level\": \"info\",\n"
    "\t\t   \"dump_json\": {\n"
    "\t\t      \"filename\": \"dump.json\",\n"
    "\t\t      \"enable\": true,\n"
    "\t\t   },\n"
    "\t\t   \"dump_curl\": {\n"
    "\t\t      \"filename\": \"dump.curl\",\n"
    "\t\t      \"enable\": true,\n"
    "\t\t   }\n"
    "\t}\n"
    "}";

  struct {
    struct {
      char token[512]; /* set it to long enough */
    } discord;
    struct {
      char username[512];
      char token[512];
    } github;
    struct {
      char filename[PATH_MAX];
      char level[128];
      struct {
        char filename[PATH_MAX];
        bool enable;
      } dump_json;
      struct {
        char filename[PATH_MAX];
        bool enable;
      } dump_curl;
    } logging;
  } settings;

  size_t ret = json_extract(json, strlen(json),
               "(discord)(token):s"
                 "(github):"
                 "{"
                 "(username):s"
                 "(token):s"
                 "}"
                 "(logging):"
                 "{"
                 "(filename):s"
                 "(level):s"
                 "(dump_json): "
                 "{"
                 "(filename):s"
                 "(enable):b"
                 "}"
                 "(dump_curl): "
                 "{"
                 "(filename):s"
                 "(enable):b"
                 "}"
               "}",
               settings.discord.token,
               settings.github.username,
               settings.github.token,
               settings.logging.filename,
               settings.logging.level,
               settings.logging.dump_json.filename,
               &settings.logging.dump_json.enable,
               settings.logging.dump_curl.filename,
               &settings.logging.dump_curl.enable);

  fprintf(stderr, "\n\nextracted %zu values\n", ret);
  fprintf(stderr, "discord.token: %s\n", settings.discord.token);
  fprintf(stderr, "github.username: %s\n", settings.github.username);
  fprintf(stderr, "github.token: %s\n", settings.github.token);
  fprintf(stderr, "logging.filename: %s\n", settings.logging.filename);
  fprintf(stderr, "logging.level: %s\n", settings.logging.level);
  fprintf(stderr, "logging.dump_json.filename: %s\n", settings.logging.dump_json.filename);
  fprintf(stderr, "logging.dump_json.filename: %d\n", settings.logging.dump_json.enable);
  fprintf(stderr, "logging.dump_curl.filename: %s\n", settings.logging.dump_curl.filename);
  fprintf(stderr, "logging.dump_curl.filename: %d\n", settings.logging.dump_curl.enable);


  ret = json_extract(json, strlen(json),
                     "(discord.token):s"
                       "(github.username):s"
                       "(github.token):s"
                       "(logging.filename):s"
                       "(logging.level):s"
                       "(logging.dump_json.filename):s"
                       "(logging.dump_json.enable):b"
                       "(logging.dump_curl.filename):s"
                       "(logging.dump_curl.enable):b",
                         settings.discord.token,
                         settings.github.username,
                         settings.github.token,
                         settings.logging.filename,
                         settings.logging.level,
                         settings.logging.dump_json.filename,
                         &settings.logging.dump_json.enable,
                         settings.logging.dump_curl.filename,
                         &settings.logging.dump_curl.enable);

  fprintf(stderr, "\n\nextracted %zu values\n", ret);
  fprintf(stderr, "discord.token: %s\n", settings.discord.token);
  fprintf(stderr, "github.username: %s\n", settings.github.username);
  fprintf(stderr, "github.token: %s\n", settings.github.token);
  fprintf(stderr, "logging.filename: %s\n", settings.logging.filename);
  fprintf(stderr, "logging.level: %s\n", settings.logging.level);
  fprintf(stderr, "logging.dump_json.filename: %s\n", settings.logging.dump_json.filename);
  fprintf(stderr, "logging.dump_json.filename: %d\n", settings.logging.dump_json.enable);
  fprintf(stderr, "logging.dump_curl.filename: %s\n", settings.logging.dump_curl.filename);
  fprintf(stderr, "logging.dump_curl.filename: %d\n", settings.logging.dump_curl.enable);

  json = "{\n"
    "\t\"ref\":\"refs/heads/master\",\n"
    "\t\"node_id\":\"MDM6UmVmMjgxNTM2NjcwOnJlZnMvaGVhZHMvbWFzdGVy\",\n"
    "\t\"url\":\"https://api.github.com/repos/cee-studio/orca/git/refs/heads/master\",\n"
    "\t\"object\":{\n"
    "\t\t\"sha\":\"37391fb67135651f83e586c49ec7a96e773ba733\",\n"
    "\t\t\"type\":\"commit\",\n"
    "\t\t\"url\":\"https://api.github.com/repos/cee-studio/orca/git/commits/37391fb67135651f83e586c49ec7a96e773ba733\"\n"
    "\t}\n"
    "}";
  char * p;
  ret = json_extract(json, strlen(json),
                     "(object.sha):?s", &p);

  fprintf (stderr, "extracted value %zu\n", ret);
  fprintf (stderr, "%s\n", p);

  struct sized_buffer tok = { .start = NULL, .size = 0 };

  ret = json_extract(json, strlen(json), "(object):T", &tok);

  fprintf (stderr, "extracted value %.*s\n", (int)tok.size, tok.start);


  fprintf(stderr, "\n");
  char * t = "(id):s_as_u64,(username):s,(discriminator):s,(avatar):s,"
    "(bot):b,(System):b,(mfa_enabled):b,(locale):s,(verified):b,"
    "(email):s,(flags):d,(premium_type):d,(public_flags):d,@arg_switches:b";
  fprintf (stderr, "input: %s\n", t);

  char *s = NULL;
  void * A[2] = {0};

  fprintf(stderr, "\n\nTesting @record_defined ...\n\n");

  fprintf(stderr, "&s = %p\n", &s);
  fprintf(stderr, "A = %p\n", A);
  ret = json_extract(json, strlen(json),
                     "(ref):?s"
                     "@record_defined",
                     &s,
                     A, sizeof(A));

  fprintf(stderr, "%s\n", s);
  fprintf(stderr, "%p\n", A);
  fprintf(stderr, "%p\n", A[0]);

  uint64_t  x = 0;
  json = "{ \"a\":\"0x00000001\" } ";

  ret = json_extract(json, strlen(json),
                     "(a):s_as_hex64",
                     &x);

  char * bb = NULL;
  json_ainject(&bb, "(a):s_as_hex64", &x);
  fprintf(stderr, "%s\n", bb);
  return 0;
}
