# kyaa

super hacky macro hacks for parsing arguments in C.

## prerequisites

C99 or greater.

standard library headers:
* `stdbool.h`
* `stdio.h`
* `string.h`

in addition, `kyaa_extra.h` requires `limits.h`,
or at least `LONG_MIN` to be defined.

## tutorial

ensure `argc` and `argv` are defined.
kyaa doesn't actually care if it's in `main` or not.

iterate over the arguments with `KYAA_LOOP`:

```c
int main(int argc, char *argv[]) {
    KYAA_LOOP {
        // KYAA_ITER, kyaa_name, kyaa_read_stdin, and kyaa_flag are exposed here.
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
`kyaa_read_stdin` is set to true.

arguments may be passed in three ways, consider:
* `-v42`
* `-v 42`
* `--var 42`

kyaa returns `KYAA_OKAY` when `-h` or `--help` is given,
and `KYAA_ERROR` in the event of invalid flags, missing arguments,
or invalid numbers (`KYAA_FLAG_LONG`).
kyaa uses `continue` for handling arguments.

kyaa prints error messages to `stderr`, and help text to `stdout`.

`KYAA_ITER` may be redefined to avoid name collisions.

## TODO

* support `--var=42` argument style
* fix overlapping names, e.g. `KYAA_FLAG` vs `kyaa_flag`, `KYAA_FLAG_ARG` vs `kyaa_flag_arg`, etc.
* maybe pass `argc`/`argv` manually?

## API

### kyaa.h

return type or type | name | arguments or default value
--- | --- | ---
define          | KYAA\_OKAY            | 0
define          | KYAA\_ERROR           | 1
define          | KYAA\_ITER            | i
||
macro           | KYAA\_SETUP           | *none*
macro           | KYAA\_LOOP            | *none*
macro           | KYAA\_BEGIN           | *none*
macro           | KYAA\_END             | *none*
macro           | KYAA\_DESCRIBE        | c, name, description
macro           | KYAA\_FLAG            | c, name, description
macro           | KYAA\_FLAG\_ARG       | c, name, description
macro           | KYAA\_HELP            | description
||
`char *`        | kyaa\_name            | *n/a*
`bool`          | kyaa\_read\_stdin     | *n/a*
`char *`        | kyaa\_arg             | *n/a*
|| **internal use** ||
`char`          | kyaa\_flag            | *n/a*
`bool`          | kyaa\_keep\_parsing   | *n/a*
`bool`          | kyaa\_parse\_next     | *n/a*
`bool`          | kyaa\_no\_more        | *n/a*
`bool`          | kyaa\_helping         | *n/a*
`bool`          | kyaa\_any             | *n/a*

### kyaa\_extra.h

return value or type | name | arguments or default value
--- | --- | ---
macro           | KYAA\_FLAG\_LONG      | c, name, description
||
`char *`        | kyaa\_flag\_arg       | *n/a*
||
`char *`        | kyaa\_skip\_spaces    | `char *` str
`const char *`  | kyaa\_str\_to\_long   | `char *` str, `long *` p\_out
