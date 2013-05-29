
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sqlite3.h>

#include "auto_mem.hh"

struct callback_state {
    int cs_row;
};

static int sql_callback(void *ptr,
                        int ncols,
                        char **colvalues,
                        char **colnames)
{
    struct callback_state *cs = (struct callback_state *)ptr;

    printf("Row %d:\n", cs->cs_row);
    for (int lpc = 0; lpc < ncols; lpc++) {
        printf("  Column %10s: %s\n", colnames[lpc], colvalues[lpc]);
    }

    cs->cs_row += 1;
    
    return 0;
}

extern "C" {
int RegisterExtensionFunctions(sqlite3 *db);
}

int register_network_extension_functions(sqlite3 *db);
int register_fs_extension_functions(sqlite3 *db);

int main(int argc, char *argv[])
{
    int retval = EXIT_SUCCESS;
    auto_mem<sqlite3> db(sqlite3_close);

    if (argc != 2) {
        fprintf(stderr, "error: expecting an SQL statement\n");
        retval = EXIT_FAILURE;
    }
    else if (sqlite3_open(":memory:", db.out()) != SQLITE_OK) {
        fprintf(stderr, "error: unable to make sqlite memory database\n");
        retval = EXIT_FAILURE;
    }
    else {
        auto_mem<char> errmsg(sqlite3_free);
        struct callback_state state;

        {
            int register_collation_functions(sqlite3 * db);

            RegisterExtensionFunctions(db.in());
            register_network_extension_functions(db.in());
            register_fs_extension_functions(db.in());
            register_collation_functions(db.in());
        }

        if (sqlite3_exec(db.in(),
            argv[1],
            sql_callback,
            &state,
            errmsg.out()) != SQLITE_OK) {
            fprintf(stderr, "error: sqlite3_exec failed -- %s\n", errmsg.in());
            retval = EXIT_FAILURE;
        }
    }

    return retval;
}