#include "cee-utils.h"
#include "ntl.h"
#include "json-actor.h"

#include "greatest.h"


TEST expect_unescaped_equal_original(char str[])
{
  size_t len = strlen(str);

  char *estr=NULL, *unstr=NULL;
  size_t estr_size=0, unstr_size=0;

  ASSERTm("Couldn't escape", estr = json_string_escape(&estr_size, str, len));
  ASSERTm("Couldn't unescape", json_string_unescape(&unstr, &unstr_size, estr, estr_size));
  ASSERT_FALSEm("Unescaped and escaped are the same", 0 == strcmp(estr, unstr));
  ASSERT_STRN_EQm("Unescaped doesn't match original", str, unstr, len);
  PASS();
}

TEST expect_escaped_equal_original(char str[])
{
  size_t len = strlen(str);

  char *estr=NULL, *unstr=NULL;
  size_t estr_size=0, unstr_size=0;

  ASSERTm("Couldn't unescape", json_string_unescape(&unstr, &unstr_size, str, len));
  ASSERTm("Couldn't escape", estr = json_string_escape(&estr_size, unstr, unstr_size));
  ASSERT_FALSEm("Unescaped and escaped are the same", 0 == strcmp(estr, unstr));
  ASSERT_STRN_EQm("Escaped doesn't match original", str, estr, len);
  PASS();
}

SUITE(check_json_string)
{
  int i;
  char scp_list[][2][256] = {
    {"string", "Hello\bThere\n"},
    {"UTF8", "🆗🅰️ЈÐ"}
  };
  char unscp_list[][2][256] = { 
    {"string", "Hello\\bThere\\n"},
    {"UTF8", "\\ud83d\\udcac" }
  };

  for (i=0; i < sizeof scp_list / sizeof *scp_list; ++i) {
    greatest_set_test_suffix(scp_list[i][0]);
    RUN_TEST1(expect_unescaped_equal_original, scp_list[i][1]);
  }
  for (i=0; i < sizeof unscp_list / sizeof *unscp_list; ++i) {
    greatest_set_test_suffix(unscp_list[i][0]);
    RUN_TEST1(expect_escaped_equal_original, unscp_list[i][1]);
  }
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
  GREATEST_MAIN_BEGIN();

  RUN_SUITE(check_json_string);

  GREATEST_MAIN_END();
}
