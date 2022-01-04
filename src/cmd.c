#include "cmd.h"

enum cmd_builtin
identify_cmd (const char * cmd) {

    if ( !cmd ) {
        return C_UNKNOWN;
    }

    for ( size_t i = 0; i < C_UNKNOWN; ++i ) {
        if ( !strncmp(builtin_name[i], cmd, strlen(builtin_name[i])) ) {
            return (enum cmd_builtin )i;
        }
    }

    return C_UNKNOWN;
}
