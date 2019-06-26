#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "greatest.h"

GREATEST_MAIN_DEFS();

// use non-default return codes to test that they actually change.
// this also allows distinguishing KYAA_OKAY from the user's main() returning 0.
#define KYAA_OKAY 123
#define KYAA_FAIL -666

// capture stdout and stderr for later testing.
#define CAPTURE_SIZE 2048
static char global_buf[CAPTURE_SIZE] = {0};
static char global_out[CAPTURE_SIZE] = {0};
static char global_err[CAPTURE_SIZE] = {0};
static int out_offset = 0;
static int err_offset = 0;
static bool out_overflown = false;
static bool err_overflown = false;

static int capture_stdout(int printf_result) {
    if (printf_result < 0 || printf_result >= CAPTURE_SIZE) goto fail;
    for (int i = 0; i < printf_result; i++) {
        // copy as much as possible before capping out.
        if (out_offset >= CAPTURE_SIZE) goto fail;
        global_out[out_offset] = global_buf[i];
        out_offset++;
    }
    return printf_result;

fail:
    out_overflown = true;
    return printf_result;
}

static int capture_stderr(int printf_result) {
    if (printf_result < 0 || printf_result >= CAPTURE_SIZE) goto fail;
    for (int i = 0; i < printf_result; i++) {
        // copy as much as possible before capping out.
        if (err_offset >= CAPTURE_SIZE) goto fail;
        global_err[err_offset] = global_buf[i];
        err_offset++;
    }
    return printf_result;

fail:
    err_overflown = true;
    return printf_result;
}

#define KYAA_OUT(...) capture_stdout(snprintf(global_buf, CAPTURE_SIZE, __VA_ARGS__))
#define KYAA_ERR(...) capture_stderr(snprintf(global_buf, CAPTURE_SIZE, __VA_ARGS__))

#include "kyaa.h"
#include "kyaa_extra.h"

#define main(a, b) subject(a, const b)
#include "test_subject.c"
#undef main

// testing begins here!

#define ASSERT_ZERO(ret) ASSERT_EQ(ret, 0)
#define ASSERT_OKAY(ret) ASSERT_EQ(ret, KYAA_OKAY)
#define ASSERT_FAIL(ret) ASSERT_EQ(ret, KYAA_FAIL)
#define ASSERT_OUT(str) do { \
    ASSERT_FALSE(out_overflown); \
    ASSERT_STR_EQ(str, global_out); \
} while (0)
#define ASSERT_ERR(str) do { \
    ASSERT_FALSE(err_overflown); \
    ASSERT_STR_EQ(str, global_err); \
} while (0)

static void reset() {
    use_feature = false;
    log_fn = "log.txt";
    my_var = 0;
    read_stdin = false;

    out_offset = 0;
    err_offset = 0;
    memset(global_buf, 0, CAPTURE_SIZE);
    memset(global_out, 0, CAPTURE_SIZE);
    memset(global_err, 0, CAPTURE_SIZE);
    return;
}

int kyaa(int argc, const char **argv) {
    reset();
    return subject(argc, argv);
}

int kyaa_va(int argc, ...) {
    static const char *argv[256] = {0};
    va_list args;
    assert(argc <= 256);
    va_start(args, argc);
    for (int i = 0; i < argc; i++) {
        argv[i] = va_arg(args, const char *);
    }
    va_end(args);
    return kyaa(argc, argv);
}

static const char *blank = "";
static const char *basic_argv[] = {"kyaa"};
static const char *help_text = "usage:\n\
  -x  --enable-feature\n\
        enable some feature\n\
  -l  --log-file\n\
        use a given filename for the log file\n\
  -v  --var\n\
        set an integer variable\n\
        default: 0\n\
";

#define ASSERT_VALUES_UNCHANGED() do { \
    ASSERT(!use_feature); \
    ASSERT_STR_EQ("log.txt", log_fn); \
    ASSERT_EQ(0, my_var); \
} while (0)

#define ASSERT_VALUES_CHANGED() do { \
    ASSERT(use_feature); \
    ASSERT_STR_EQ("/var/log/kyaa", log_fn); \
    ASSERT_EQ(1337, my_var); \
} while (0)

TEST no_arguments() { // (as if invoked by a shell)
    ASSERT_ZERO(kyaa(1, basic_argv));
    ASSERT_VALUES_UNCHANGED();
    PASS();
}

TEST one_flag() {
    ASSERT_ZERO(kyaa_va(2, "kyaa", "-x"));
    ASSERT(use_feature);
    PASS();
}

TEST arg_flag() {
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-l", "/var/log/kyaa"));
    ASSERT_EQ("/var/log/kyaa", log_fn);
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "1337"));
    ASSERT_EQ(1337, my_var);
    PASS();
}

TEST undefined_flag() {
    ASSERT_FAIL(kyaa_va(2, "kyaa", "-p"));
    ASSERT_ERR("unknown flag: -p\n");
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-o", "outfile"));
    ASSERT_ERR("unknown flag: -o\n");
    PASS();
}

TEST many_flags() {
    ASSERT_ZERO(kyaa_va(6, "kyaa", "-v", "1337", "-l", "/var/log/kyaa", "-x")); ASSERT_VALUES_CHANGED();
    ASSERT_ZERO(kyaa_va(6, "kyaa", "-v", "1337", "-x", "-l", "/var/log/kyaa")); ASSERT_VALUES_CHANGED();
    ASSERT_ZERO(kyaa_va(6, "kyaa", "-l", "/var/log/kyaa", "-v", "1337", "-x")); ASSERT_VALUES_CHANGED();
    ASSERT_ZERO(kyaa_va(6, "kyaa", "-l", "/var/log/kyaa", "-x", "-v", "1337")); ASSERT_VALUES_CHANGED();
    ASSERT_ZERO(kyaa_va(6, "kyaa", "-x", "-v", "1337", "-l", "/var/log/kyaa")); ASSERT_VALUES_CHANGED();
    ASSERT_ZERO(kyaa_va(6, "kyaa", "-x", "-l", "/var/log/kyaa", "-v", "1337")); ASSERT_VALUES_CHANGED();
    PASS();
}

TEST empty_flag() {
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-l", ""));
    ASSERT_STR_EQ("", log_fn);
    PASS();
}

TEST empty_long_flag() {
    ASSERT_ZERO(kyaa_va(3, "kyaa", "--log-file", ""));
    ASSERT_STR_EQ("", log_fn);
    PASS();
}

TEST empty_long_flag_int() {
    ASSERT_FAIL(kyaa_va(3, "kyaa", "--var", ""));
    PASS();
}

TEST long_flag() {
    ASSERT_ZERO(kyaa_va(2, "kyaa", "--enable-feature"));
    PASS();
}

TEST long_flag_arg() {
    ASSERT_ZERO(kyaa_va(3, "kyaa", "--log-file", "/var/log/kyaa"));
    ASSERT_ZERO(kyaa_va(3, "kyaa", "--var", "1337"));
    PASS();
}

TEST long_flag_equals() {
    ASSERT_ZERO(kyaa_va(2, "kyaa", "--log-file=/var/log/kyaa"));
    ASSERT_ZERO(kyaa_va(2, "kyaa", "--var=1337"));
    PASS();
}

TEST long_flag_equals_empty() {
    ASSERT_ZERO(kyaa_va(2, "kyaa", "--log-file="));
    PASS();
}

TEST long_flag_help_arg() {
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-l", "-h"));
    ASSERT_STR_EQ("-h", log_fn);
    ASSERT_OUT(blank);
    ASSERT_ZERO(kyaa_va(3, "kyaa", "--log-file", "--help"));
    ASSERT_STR_EQ("--help", log_fn);
    ASSERT_OUT(blank);
    PASS();
}

TEST long_flag_equals_empty_int() {
    ASSERT_FAIL(kyaa_va(2, "kyaa", "--var="));
    PASS();
}

TEST undefined_long_flag() {
    ASSERT_FAIL(kyaa_va(2, "kyaa", "--print"));
    ASSERT_ERR("unknown flag: --print\n");
    ASSERT_FAIL(kyaa_va(3, "kyaa", "--out", "outfile"));
    ASSERT_ERR("unknown flag: --out\n");
    ASSERT_FAIL(kyaa_va(3, "kyaa", "--log", "/var/log/kyaa"));
    ASSERT_ERR("unknown flag: --log\n");
    ASSERT_FAIL(kyaa_va(2, "kyaa", "--log=/var/log/kyaa"));
    ASSERT_ERR("unknown flag: --log=/var/log/kyaa\n");
    PASS();
}

TEST dangling_arg() {
    ASSERT_FAIL(kyaa_va(2, "kyaa", "-l"));
    ASSERT_OUT(blank);
    ASSERT_ERR("expected an argument for --log-file (-l)\n");

    ASSERT_FAIL(kyaa_va(2, "kyaa", "-v"));
    ASSERT_OUT(blank);
    ASSERT_ERR("expected an argument for --var (-v)\n");

    PASS();
}

TEST valid_int() {
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "123456789"));
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "-1"));
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "0"));
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "1"));
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "-2147483648"));
    ASSERT_EQ(-2147483648, my_var);
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "2147483647"));
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "0xBADF00D"));
    ASSERT_EQ(0xBADF00D, my_var);
    ASSERT_ZERO(kyaa_va(3, "kyaa", "-v", "0755"));
    ASSERT_EQ(0755, my_var);
    PASS();
}

TEST invalid_int() {
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", "hey"));
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", " "));
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", "0xG00DF00D"));
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", "123abc"));
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", "0123456789"));
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", "--"));
    PASS();
}

TEST huge_int() {
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", "2147483648"));
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", "-2147483649"));
    PASS();
}

TEST help() {
    static const char *argv1[] = {"kyaa", "-h"};
    static const char *argv2[] = {"kyaa", "--help"};
    /* static const char *argv3[] = {"kyaa", "-?"}; */
    /* static const char *argv4[] = {"kyaa", "/?"}; */

    ASSERT_OKAY(kyaa(2, argv1));
    ASSERT_OUT(help_text);
    ASSERT_ERR(blank);

    ASSERT_OKAY(kyaa(2, argv2));
    ASSERT_OUT(help_text);
    ASSERT_ERR(blank);

    PASS();
}

TEST use_stdin() {
    ASSERT_ZERO(kyaa_va(2, "kyaa", "-"));
    ASSERT(read_stdin);
    ASSERT_OUT("-\n");
    PASS();
}

TEST stop_parsing() {
    ASSERT_ZERO(kyaa_va(2, "kyaa", "--"));
    ASSERT_FALSE(read_stdin);
    ASSERT_ZERO(kyaa_va(5, "kyaa", "--", "blah", "--blah", "-b"));
    ASSERT_OUT("blah\n--blah\n-b\n");
    ASSERT_ZERO(kyaa_va(3, "kyaa", "--", "-x"));
    ASSERT_VALUES_UNCHANGED();
    ASSERT_ZERO(kyaa_va(4, "kyaa", "--", "-h", "--help"));
    ASSERT_OUT("-h\n--help\n");
    ASSERT_ERR(blank);
    PASS();
}

TEST keep_parsing() {
    ASSERT_ZERO(kyaa_va(4, "kyaa", "-l", "--", "-x"));
    ASSERT_STR_EQ("--", log_fn);
    ASSERT(use_feature);
    PASS();
}

TEST dangling_arg_oob() {
    // intentionally passing an inaccurate argc here:
    ASSERT_FAIL(kyaa_va(2, "kyaa", "-l", "don't read this"));
    ASSERT_OUT(blank);
    ASSERT_ERR("expected an argument for --log-file (-l)\n");

    ASSERT_FAIL(kyaa_va(2, "kyaa", "-v", "don't read this"));
    ASSERT_OUT(blank);
    ASSERT_ERR("expected an argument for --var (-v)\n");

    PASS();
}

TEST arg_oob() {
    ASSERT_FAIL(kyaa_va(3, "kyaa", "-l", NULL));
    ASSERT_OUT(blank);
    ASSERT_ERR("expected an argument for --log-file (-l)\n");

    ASSERT_FAIL(kyaa_va(3, "kyaa", "-v", NULL));
    ASSERT_OUT(blank);
    ASSERT_ERR("expected an argument for --var (-v)\n");

    PASS();
}

TEST negative_argc() {
    ASSERT_FAIL(kyaa(-1, basic_argv));
    ASSERT_FAIL(kyaa(LONG_MIN, basic_argv));
    PASS();
}

TEST null_argv() {
    ASSERT_FAIL(kyaa(0, NULL));
    ASSERT_FAIL(kyaa(1, NULL));
    ASSERT_FAIL(kyaa(2, NULL));
    ASSERT_FAIL(kyaa(-1, NULL));
    ASSERT_FAIL(kyaa(LONG_MIN, NULL));
    PASS();
}

SUITE(goods) {
    RUN_TEST(no_arguments);
    RUN_TEST(one_flag);
    RUN_TEST(arg_flag);
    RUN_TEST(many_flags);
    RUN_TEST(empty_flag);
    RUN_TEST(empty_long_flag);
    RUN_TEST(long_flag);
    RUN_TEST(long_flag_arg);
    RUN_TEST(long_flag_equals);
    RUN_TEST(long_flag_equals_empty);
    RUN_TEST(long_flag_help_arg);
    RUN_TEST(valid_int);
    RUN_TEST(help);
    RUN_TEST(use_stdin);
    RUN_TEST(stop_parsing);
    RUN_TEST(keep_parsing);
}

SUITE(bads) {
    RUN_TEST(undefined_flag);
    RUN_TEST(empty_long_flag_int);
    RUN_TEST(long_flag_equals_empty_int);
    RUN_TEST(undefined_long_flag);
    RUN_TEST(dangling_arg);
    RUN_TEST(invalid_int);
    RUN_TEST(huge_int);
}

SUITE(uglies) {
    RUN_TEST(dangling_arg_oob);
    RUN_TEST(arg_oob);
    RUN_TEST(negative_argc);
    RUN_TEST(null_argv);
}

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(goods);
    RUN_SUITE(bads);
    RUN_SUITE(uglies);
    GREATEST_MAIN_END();
}
