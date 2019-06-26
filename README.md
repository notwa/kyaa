# kyaa

super hacky macro hacks for parsing arguments in C.

## FAQ

*Fairly Arbitrary Questions*

### Why?

kyaa exists for when the "quick 'n' dirty" isn't dirty enough.
also, it keeps argument parsing *in* main,
instead of delegating with callbacks and whatnot.

### How?

kyaa secretly wraps flag-handling in if-else statements with curly braces.
it's not a switch-case method!
kyaa instead uses `continue` to handle certain arguments.

### Should I use this?

honestly, no.

### What's in a name?

i have no idea why i called it "kyaa,"
other than it being the first thing that came to mind
that wasn't already in use by some other software.

### What do I need?

the prerequisites are:

* C99 (or greater)

* `stdbool.h`

* `stdio.h` (can be avoided by overriding `KYAA_OUT` and `KYAA_ERR`)

* `string.h`

* `limits.h` (just for `kyaa_extra.h`, can be avoided by defining `LONG_MIN`)

## tutorial

ensure `argc` and `argv` are defined in a function that returns an `int`.
kyaa doesn't actually care if it's in `main` or not.

iterate over the arguments with `KYAA_LOOP`:

```c
int main(int argc, char **argv) {
    KYAA_LOOP {
        // kyaa_iter, kyaa_name, and kyaa_read_stdin are exposed here.
        // [other code goes here]
    }
    return 0;
}
```

use `KYAA_BEGIN` and `KYAA_END` to begin parsing arguments.
put unrelated code above `KYAA_BEGIN` or below `KYAA_END`, but not within:

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

use `KYAA_FLAG` for defining flags that don't take an argument,
and `KYAA_FLAG_ARG` or `KYAA_FLAG_LONG` for flags that do:

```c
    bool use_feature = false;
    const char *log_fn = "log.txt";
    long my_var = 0;
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
    }
    return 0;
```

kyaa handles `-h` and `--help` for printing help text.
additional help text may be defined using `KYAA_HELP`:

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

kyaa interprets an argument of `-` as a request to enable reading from stdin:
`kyaa_read_stdin` is set to true. this may be reset by the user if desired.

flag values may be specified in four ways; the following are equivalent:
* `-v42`
* `-v 42`
* `--var 42`
* `--var=42`

kyaa returns `KYAA_OKAY` when `-h` or `--help` is given,
or `KYAA_FAIL` in the event of invalid flags, missing arguments,
or invalid numbers (`KYAA_FLAG_LONG`).

kyaa prints help text to `stdout` and error messages to `stderr`.
this can be overridden by specifying `KYAA_OUT` and `KYAA_ERR` respectively.

## API

### kyaa.h

signature | name | arguments or default value | description
--- | --- | --- | ---
define          | KYAA\_OKAY            | 0                                 | value to return upon a successful early exit
define          | KYAA\_FAIL            | 1                                 | value to return upon encountering an error
define          | KYAA\_OUT             | `printf(__VA_ARGS__)`             | printf-like function for help text
define          | KYAA\_ERR             | `fprintf(stderr, __VA_ARGS__)`    | printf-like function for error text
macro           | KYAA\_SETUP           | *none*                            | performs a sanity check and initializes variables
macro           | KYAA\_LOOP            | *none*                            | begins iterating over arguments
macro           | KYAA\_BEGIN           | *none*                            | begins parsing arguments
macro           | KYAA\_END             | *none*                            | finishes parsing arguments
macro           | KYAA\_DESCRIBE        | c, name, description              | describes a flag for help text
macro           | KYAA\_FLAG            | c, name, description              | handles a flag that takes no arguments
macro           | KYAA\_FLAG\_ARG       | c, name, description              | handles a flag that takes a string argument
macro           | KYAA\_HELP            | description                       | prints additional text for help text
`const char *`  | kyaa\_name            | *n/a*                             | the name of the program (usually `argv[0]`)
`int`           | kyaa\_iter            | *n/a*                             | the index of the argument being parsed
`const char *`  | kyaa\_arg             | *n/a*                             | the whole argument being parsed
`const char *`  | kyaa\_etc             | *n/a*                             | an argument to a flag, or `NULL`
`bool`          | kyaa\_read\_stdin     | *n/a*                             | set when an argument is `-` (remains set unless reset by user)

#### internal use

signature | name | arguments or default value | description
--- | --- | --- | ---
macro           | KYAA\_IS\_LONG        | arg, name                         | tests whether `arg` describes a long flag called `name`
`char`          | kyaa\_char            | *n/a*                             | the character assigned to the current flag being parsed
`bool`          | kyaa\_parsing         | *n/a*                             | whether parsing occurs (`--` cancels parsing)
`bool`          | kyaa\_skip            | *n/a*                             | set when a flag needs another argument
`bool`          | kyaa\_helping         | *n/a*                             | set when `-h` or `--help` is encountered
`bool`          | kyaa\_any             | *n/a*                             | set when any flag has been matched
`const char *`  | kyaa\_equals          | *n/a*                             | a pointer to the first `=` character in a long flag, or `NULL`

### kyaa\_extra.h

signature | name | arguments or default value
--- | --- | ---
macro           | KYAA\_FLAG\_LONG      | c, name, description
`char *`        | kyaa\_flag\_arg       | *n/a*
`char *`        | kyaa\_skip\_spaces    | `char *` str
`const char *`  | kyaa\_str\_to\_long   | `char *` str, `long *` p\_out
