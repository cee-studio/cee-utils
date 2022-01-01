#if defined(CEE_USE_SQLITE3)

#include "cee-sqlite3.h"

sqlite3* cee_sqlite3_init_db(char *dbname, char *sqlstmts)
{
  sqlite3_stmt *res;
  sqlite3 *db;

  int rc = sqlite3_open(dbname, &db);
  if (rc != SQLITE_OK) 
  {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    db = NULL;
    return NULL;
  }

  char *err_msg=NULL;
  rc = sqlite3_exec(db, "begin transaction;", 0, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
    return NULL;
  }

  rc = sqlite3_exec(db, sqlstmts, 0, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
    return NULL;
  }

  rc = sqlite3_exec(db, "commit;", 0, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
    return NULL;
  }
  return db;
}

int cee_sqlite3_bind_run_sql(sqlite3 *db, struct cee_sqlite3_bind_info *pairs,
                             char *sql, sqlite3_stmt **res_p)
{
  sqlite3_stmt *res;
  int rc = sqlite3_prepare_v2(db, sql, -1, res_p, 0);
  res = *res_p;
  int idx = 0;

  if (rc == SQLITE_OK) {
    if (pairs) {
      for(int i = 0; pairs[i].name; i++) {
        idx = sqlite3_bind_parameter_index(res, pairs[i].name);
        if (idx <= 0) continue;
        switch(pairs[i].type) 
        {
          case CEE_SQLITE3_INT:
            sqlite3_bind_int(res, idx, *(int *)pairs[i].value);
            break;
          case CEE_SQLITE3_INT64:
            sqlite3_bind_int64(res, idx, *(int64_t *)pairs[i].value);
            break;
          case CEE_SQLITE3_TEXT:
            sqlite3_bind_text(res, idx, (char*)pairs[i].value, pairs[i].size == 0 ? -1: pairs[i].size, SQLITE_STATIC);
            break;
          case CEE_SQLITE3_BLOB:
            sqlite3_bind_blob(res, idx, (void*)pairs[i].value, pairs[i].size, SQLITE_STATIC);
            break;
        }
      }
    }
    return sqlite3_step(res);
  }
  else {
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    return rc;
  }
}

int cee_sqlite3_insert_or_update(sqlite3 *db, struct cee_sqlite3_iu *p)
{
  sqlite3_stmt *res;
  int rc = 0, step;

  sqlite3_exec(db, "begin transaction;", NULL, NULL, NULL);

  step = cee_sqlite3_bind_run_sql(db, p->pairs, p->select, &res);
  sqlite3_finalize(res);
  if (step == SQLITE_ROW) {
    step = cee_sqlite3_bind_run_sql(db, p->pairs, p->update, &res);
    if (step != SQLITE_DONE) {
      fprintf(stderr, "execution failed: %s\n", sqlite3_errmsg(db));
      rc = -1;
    }
    sqlite3_finalize(res);
  }
  else {
    step = cee_sqlite3_bind_run_sql(db, p->pairs, p->insert, &res);
    if (step != SQLITE_DONE) {
      fprintf(stderr, "execution failed: %s\n", sqlite3_errmsg(db));
      rc = -1;
    }
    sqlite3_finalize(res);
  }
  sqlite3_exec(db, "end transaction;", NULL, NULL, NULL);
  return rc;
}

#endif
