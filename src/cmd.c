#include "cmd.h"

enum cmd_builtin
identify_cmd (const char * cmd) {

    if ( !cmd ) {
        return C_UNKNOWN;
    }

    for ( size_t i = 0; i < C_UNKNOWN; ++i ) {
        size_t len = strlen(builtin_name[i]);
        if ( !strncmp(builtin_name[i], cmd, len) ) {
            return (enum cmd_builtin )i;
        }
    }

    return C_UNKNOWN;
}
