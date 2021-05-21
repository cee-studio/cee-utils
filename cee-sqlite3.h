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
  char * name;
  enum type {
    INT,
    INT64,
    TEXT
  } type;
  void * value;
};

struct cee_sqlite3_iu {
  char * select;
  char * update;
  char * insert;
  struct cee_sqlite3_bind_info *pairs;
};


extern int cee_sqlite3_bind_run_sql(sqlite3 *db, struct cee_sqlite3_bind_info *pairs, char *sql, sqlite3_stmt **res_p);

extern void cee_sqlite_3_insert_or_update(sqlite3 *db, struct cee_sqlite3_iu *p);

#endif // CEE_USE_SQLITE3
#endif
