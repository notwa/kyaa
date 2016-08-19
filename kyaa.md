# kyaa

super hacky macro hacks for parsing arguments in C.

## prerequisites

C99 or greater.

standard library headers:
* errno.h
* stdbool.h
* stdio.h
* string.h

## tutorial/API

ensure argc and argv are defined.
kyaa doesn't actually care if it's in main() or not.

iterate over the arguments with KYAA\_LOOP:

```c
int main(int argc, char *argv[]) {
    KYAA_LOOP {
        // i, kyaa_name, kyaa_read_stdin, and kyaa_flag are exposed here.
        // [other code goes here]
    }
    return 0;
}
```

use KYAA\_BEGIN and KYAA\_END to begin parsing arguments.
put unrelated code above KYAA\_BEGIN or below KYAA\_END, but not within:

```c
    KYAA_LOOP {
        // [other code goes here]
        KYAA_BEGIN
        // kyaa_arg and kyaa_etc are exposed here.
        // [kyaa code goes here]
        KYAA_END
        // [other code goes here]
    }
```

use KYAA\_FLAG for defining flags that don't take an argument,
and KYAA\_FLAG\_ARG or KYAA\_FLAG\_LONG for flags that do:

```c
    bool use_feature = false;
    char *log_fn = "logs.txt";
    long my_var = 0;
    KYAA_LOOP {
        // [other code goes here]
        KYAA_BEGIN

        // arguments: short flag, long flag, help description
        KYAA_FLAG("-x", "--enable-feature",
"        enable some feature")
            use_feature = true;

        // same arguments, but kyaa_etc contains the relevant string.
        KYAA_FLAG_ARG("-l", "--log-file",
"        use a given filename for the log file")
            log_fn = kyaa_etc;

        // same arguments, kyaa_etc is set, but kyaa_flag_arg is also set.
        KYAA_FLAG_LONG("-v", "--var",
"        set an integer variable\n"
"        default: 0")
            my_var = kyaa_flag_arg;

        KYAA_END
        // [other code goes here]
    }
```

kyaa secretly wraps flag handling in if/else statements with {} blocks.
do not confuse it for a switch/case method.

kyaa handles -h and --help for printing help text.
additional help text may be defined using KYAA\_HELP:

```c
    KYAA_LOOP {
        KYAA_BEGIN
        KYAA_HELP(
"  {files...}\n"
"        do things with files.")
        KYAA_END

        do_stuff(kyaa_arg);
    }
```

kyaa interprets an argument of "-" as a request to enable reading from stdin:
kyaa\_read\_stdin is set to true.

arguments may be passed in three ways, consider:
* `-v42`
* `-v 42`
* `--var 42`

kyaa returns KYAA\_OKAY when -h or --help is given,
and KYAA\_ERROR in the event of invalid flags, missing arguments,
or invalid numbers (KYAA\_FLAG\_LONG).
kyaa uses continue for handling arguments.

kyaa prints error messages to stderr, and help text to stdout.

KYAA\_ITER may be redefined to avoid name collisions.

## TODO

* support `--var=42` argument style
* rename overlapping things, e.g. KYAA\_FLAG vs kyaa\_flag, KYAA\_FLAG\_ARG vs kyaa\_flag\_arg, etc.
* move KYAA\_FLAG\_ARG to `kyaa_extend.h` or something; write similar macros.