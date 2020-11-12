#include "cmd.h"

enum cmd_builtin
identify_cmd (const char * cmd) {

    for ( size_t i = 0; i <= C_DISCONNECT; ++i ) {
        if ( !strcmp(cmd, builtin_name[i]) ) {
            return (enum cmd_builtin )i;
        }
    }

    return C_UNKNOWN;
}
