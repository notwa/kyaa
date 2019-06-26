#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "kyaa.h"
#include "kyaa_extra.h"

bool use_feature = false;
const char *log_fn = "log.txt";
long my_var = 0;
bool read_stdin = false;

int main(int argc, char **argv) {
    KYAA_LOOP {
        // [other code goes here]
        KYAA_BEGIN

        // arguments: short flag, long flag, help description
        KYAA_FLAG('x', "enable-feature",
"        enable some feature")
            use_feature = true;

        // same arguments, but kyaa_etc contains the relevant string.
        KYAA_FLAG_ARG('l', "log-file",
"        use a given filename for the log file")
            log_fn = kyaa_etc;

        // same arguments, kyaa_etc is set, but kyaa_long_value is also set.
        KYAA_FLAG_LONG('v', "var",
"        set an integer variable\n"
"        default: 0")
            my_var = kyaa_long_value;

        KYAA_END

        // [other code goes here]
        read_stdin = kyaa_read_stdin;
        KYAA_OUT("%s\n", kyaa_arg);
    }
    return 0;
}
