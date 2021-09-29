#ifndef CEE_SQLITE3_H
#define CEE_SQLITE3_H

#if defined(CEE_USE_SQLITE3)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "sqlite3.h"


extern sqlite3* cee_sqlite3_init_db(char *dbname, char *sqlstmts);

struct cee_sqlite3_bind_info {
  char *var_name;
  char *col_name;
  enum type {
    CEE_SQLITE3_INT,
    CEE_SQLITE3_INT64,
    CEE_SQLITE3_TEXT,
    CEE_SQLITE3_BLOB
  } type;
  void *value;
  size_t size;
};

struct cee_sqlite3_iu {
  char * select;
  char * update;
  char * insert;
  struct cee_sqlite3_bind_info *pairs;
};

extern int cee_sqlite3_bind_run_sql(sqlite3 *db, 
                                    struct cee_sqlite3_bind_info *pairs, 
                                    char *sql, sqlite3_stmt **res_p);

extern int cee_sqlite3_insert_or_update(sqlite3 *db, struct cee_sqlite3_iu *p);

#endif // CEE_USE_SQLITE3
#endif
