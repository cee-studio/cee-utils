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
  char *name;     // name for the binding variable
  char *col_name; // db column name
  char *ext_name; // external name
  enum type {
    CEE_SQLITE3_INT,
    CEE_SQLITE3_INT64,
    CEE_SQLITE3_TEXT,
    CEE_SQLITE3_BLOB
  } type;
  void *value;
  size_t size;
};

/*
 * the value of binding data
 */
struct cee_sqlite3_bind_data {
  int i;
  int64_t i64;
  char *value;
  size_t size;
};

struct cee_sqlite3_iu {
  char * select;
  char * update;
  char * insert;
  char * delete_stmt;
  struct cee_sqlite3_bind_info *pairs;
};

extern int cee_sqlite3_bind_run_sql(sqlite3 *db, 
                                    struct cee_sqlite3_bind_info *pairs, 
                                    char *sql, sqlite3_stmt **res_p);

extern int cee_sqlite3_bind_run_sql2(sqlite3 *db,
				     struct cee_sqlite3_bind_info *info,
				     struct cee_sqlite3_bind_data *data,
				     char *sql, sqlite3_stmt **res_p);

extern int cee_sqlite3_insert_or_update(sqlite3 *db, struct cee_sqlite3_iu *p);


extern int cee_sqlite3_insert_or_update2(sqlite3 *db,
					 struct cee_sqlite3_bind_data *data,
					 struct cee_sqlite3_iu *p);


#endif // CEE_USE_SQLITE3
#endif
