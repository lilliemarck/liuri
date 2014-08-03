#include <check.h>
#include <stdlib.h>
#include "liuri.h"
#include <string.h>
#include <stdio.h>

START_TEST(valid_ipv4) {
    ck_assert_int_eq(LIURI_HOST_IPV4, liuri_address_type("127.0.0.1", -1));
    ck_assert_int_eq(LIURI_HOST_IPV4, liuri_address_type("0.0.0.0", -1));
    ck_assert_int_eq(LIURI_HOST_IPV4, liuri_address_type("255.255.255.255", -1));
} END_TEST

START_TEST(invalid_ipv4) {
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("0127.0.0.1", -1));
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("127.00.0.1", -1));
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("127.0.0.01", -1));
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("256.0.0.1", -1));
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("127.0.0.-1", -1));
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("127.0.0", -1));
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("1.2.3.4.5", -1));
    ck_assert_int_ne(LIURI_HOST_IPV4, liuri_address_type("name", -1));
} END_TEST

START_TEST(valid_ipv6) {
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[0:0:0:0:0:0:0:0]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[00:000:0000:F:0F:0FF:0FFF:FFFF]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[::]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[::0]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[::0:0:0:0:0:0:0]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[0::0]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[0:0:0:0:0:0:0::]", -1));
} END_TEST

START_TEST(valid_ipv6_with_ipv4) {
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[0:0:0:0:0:0:127.0.0.1]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[::0:0:0:0:0:127.0.0.1]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[::127.0.0.1]", -1));
    ck_assert_int_eq(LIURI_HOST_IPV6, liuri_address_type("[0::127.0.0.1]", -1));
} END_TEST

START_TEST(invalid_ipv6) {
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[0FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[:0:0:0:0:0:0:0:0]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[0:0:0:0:0:0:0.0:]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[::0:0:0:0:0:0:0:0]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[0:0:0:0:0:0:0:127.0.0.1]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[0:0:0:0:0:0:0:127.0]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[::0:0:0:0:0:127.X]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[::0::]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[::0:]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[:0::]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[0:]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[127.0.0.1]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[127.]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[::127.]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[0X]", -1));
    ck_assert_int_ne(LIURI_HOST_IPV6, liuri_address_type("[0", -1));
} END_TEST

START_TEST(valid_ipvfuture) {
    ck_assert_int_eq(LIURI_HOST_IPVFUTURE, liuri_address_type("[vff.address:132]", -1));
    ck_assert_int_ne(LIURI_HOST_IPVFUTURE, liuri_address_type("[v0.]", -1));
    ck_assert_int_ne(LIURI_HOST_IPVFUTURE, liuri_address_type("[v.]", -1));
} END_TEST

static int compare(char const *string, struct liuri_match const *match) {
    if (!string && !match->string) {
        return 1;
    }

    if (string && match->string) {
        return strncmp(string, match->string, match->length) == 0;
    }

    return 0;
}

/*
 * If uri is a valid uri, compares each matched component with the given strings.
 */
static int match(char const *uri, char const *scheme, char const *authority, char const *userinfo, char const *host, char const *port, char const *path, char const *query, char const *fragment) {
    struct liuri_components components;

    if (liuri_parse(uri, -1, &components)) {
        return
            compare(scheme,    &components.scheme) &&
            compare(authority, &components.authority) &&
            compare(userinfo,  &components.userinfo) &&
            compare(host,      &components.host) &&
            compare(port,      &components.port) &&
            compare(path,      &components.path) &&
            compare(query,     &components.query) &&
            compare(fragment,  &components.fragment);
    }

    return 0;
}

/*
 * Returns 1 if the uri is valid. Otherwise returns 0.
 */
static int valid(char const *uri) {
    struct liuri_components components;
    return liuri_parse(uri, -1, &components);
}

START_TEST(empty_uri) {
    ck_assert(match("", 0, 0, 0, 0, 0, "", 0, 0));
} END_TEST

START_TEST(uri_scheme) {
   ck_assert(match("",             0, 0, 0, 0, 0, "", 0, 0));
   ck_assert(match("scheme:",      "scheme", 0, 0, 0, 0, "", 0, 0));
   ck_assert(match("svn+ssl:",     "svn+ssl", 0, 0, 0, 0, "", 0, 0));
   ck_assert(match("hello.world:", "hello.world", 0, 0, 0, 0, "", 0, 0));

   ck_assert(!valid(":"));
   ck_assert(!valid("pct%65:"));
} END_TEST

START_TEST(uri_authority) {
    ck_assert(match("//",               0,        "",       0, "",       0, "", 0, 0));
    ck_assert(match("//host",           0,        "host",   0, "host",   0, "", 0, 0));
    ck_assert(match("scheme://host",    "scheme", "host",   0, "host",   0, "", 0, 0));
    ck_assert(match("scheme://h%6fst", "scheme",  "h%6fst", 0, "h%6fst", 0, "", 0, 0));

    ck_assert(!valid("scheme://ho%6st"));
} END_TEST

START_TEST(uri_userinfo) {
    ck_assert(match("//@",         0, "@",         "",       "",     0,    "",  0, 0));
    ck_assert(match("//user@",     0, "user@",     "user",   "",     0,    "",  0, 0));
    ck_assert(match("//use%72@",   0, "use%72@",   "use%72", "",     0,    "",  0, 0));
    ck_assert(match("//user@host", 0, "user@host", "user",   "host", 0,    "",  0, 0));
    ck_assert(match("//user@:80",  0, "user@:80",  "user",   "",     "80", "",  0, 0));
    ck_assert(match("//user@:",    0, "user@:",    "user",   "",     "",   "",  0, 0));
    ck_assert(match("//user@/",    0, "user@:",    "user",   "",     0,    "/", 0, 0));
} END_TEST

START_TEST(uri_host) {
    ck_assert(match("//",                           0, "",                           0,      "",                      0,    "",  0, 0));
    ck_assert(match("//host",                       0, "host",                       0,      "host",                  0,    "",  0, 0));
    ck_assert(match("//user@host",                  0, "user@host",                  "user", "host",                  0,    "",  0, 0));
    ck_assert(match("//user@127.0.0.1",             0, "user@127.0.0.1",             "user", "127.0.0.1",             0,    "",  0, 0));
    ck_assert(match("//user@[::]",                  0, "user@[::]",                  "user", "[::]",                  0,    "",  0, 0));
    ck_assert(match("//user@[0:0:0:0:0:0:0:0]",     0, "user@[0:0:0:0:0:0:0:0]",     "user", "[0:0:0:0:0:0:0:0]",     0,    "",  0, 0));
    ck_assert(match("//user@[0:0:0:0:0:0:0:0]:80",  0, "user@[0:0:0:0:0:0:0:0]:80",  "user", "[0:0:0:0:0:0:0:0]",     "80", "",  0, 0));
    ck_assert(match("//user@[0:0:0:0:0:0:0.0.0.0]", 0, "user@[0:0:0:0:0:0:0.0.0.0]", "user", "[0:0:0:0:0:0:0.0.0.0]", 0,    "",  0, 0));
    ck_assert(match("//user@h%6Fst",                0, "user@h%6Fst",                "user", "h%6Fst" ,               0,    "",  0, 0));
    ck_assert(match("//user@host:80",               0, "user@host:80",               "user", "host",                  "80", "",  0, 0));
    ck_assert(match("///",                          0, "",                           0,      "",                      0,    "/", 0, 0));
    ck_assert(match("//host/",                      0, "host",                       0,      "host",                  0,    "/", 0, 0));
    ck_assert(match("//?query",                     0, "",                           0,      "",                      0,    "",  "query", 0));
    ck_assert(match("//#fragment",                  0, "",                           0,      "",                       0,    "",  0, "fragment"));

    ck_assert(match("path",    0,      0, 0, 0, 0, "path", 0, 0));
    ck_assert(match("path:80", "path", 0, 0, 0, 0, "80",   0, 0));
    ck_assert(!valid("//[host]"));
} END_TEST

START_TEST(uri_port) {
    ck_assert(match("//host:80", 0, "host:80", 0, "host", "80", "", 0, 0));
    ck_assert(match("//:80",     0, ":80",     0, "",     "80", "", 0, 0));
    ck_assert(match("//:",       0, ":",       0, "",     "",   "", 0, 0));
} END_TEST

START_TEST(uri_path) {
    ck_assert(match("",               0,        0,       0, 0,      0, "",               0, 0));
    ck_assert(match("path",           0,        0,       0, 0,      0, "path",           0, 0));
    ck_assert(match("path/subpath",   0,        0,       0, 0,      0, "path/subpath",   0, 0));
    ck_assert(match("/path",          0,        0,       0, 0,      0, "/path",          0, 0));
    ck_assert(match("//host",         0,        "host",  0, "host", 0, "",               0, 0));
    ck_assert(match("/path/",         0,        0,       0, 0,      0, "/path/",         0, 0));
    ck_assert(match("/path//subpath", 0,        0,       0, 0,      0, "/path//subpath", 0, 0));
    ck_assert(match("scheme:",        "scheme", 0,       0, 0,      0, "",               0, 0));
    ck_assert(match("scheme:path",    "scheme", 0,       0, 0,      0, "path",           0, 0));
    ck_assert(match("scheme:p%61th",  "scheme", 0,       0, 0,      0, "p%61th",         0, 0));
    ck_assert(match("scheme:/path",   "scheme", 0,       0, 0,      0, "/path",          0, 0));
    ck_assert(match("scheme://host",  "scheme", "host",  0, "host", 0, "",               0, 0));
    ck_assert(match("//host/path",    0,        "host",  0, "host", 0, "/path",          0, 0));
    ck_assert(match("//host//path",   0,        "host",  0, "host", 0, "//path",         0, 0));

    ck_assert(!valid("//host:80path"));
    ck_assert(!valid("p%61th:with:colon:first"));
} END_TEST

START_TEST(uri_query) {
    ck_assert(match("?query",          0,        0,     0, 0,  0,    "",     "query", 0));
    ck_assert(match("path?query",      0,        0,     0, 0,  0,    "path", "query", 0));
    ck_assert(match("//:80?query",     0,        ":80", 0, "", "80", "",     "query", 0));
    ck_assert(match("scheme:?query",   "scheme", 0,     0, 0,  0,    "",     "query", 0));
    ck_assert(match("?query#fragment", 0,        0,     0, 0,  0,    "",     "query", "fragment"));
} END_TEST

START_TEST(uri_fragment) {
    ck_assert(match("#fragment",          0,        0,     0, 0,  0,    "",     0, "fragment"));
    ck_assert(match("path#fragment",      0,        0,     0, 0,  0,    "path", 0, "fragment"));
    ck_assert(match("//:80#fragment",     0,        ":80", 0, "", "80", "",     0, "fragment"));
    ck_assert(match("scheme:#fragment",   "scheme", 0,     0, 0,  0,    "",     0, "fragment"));
    ck_assert(match("#fragm%65nt",        0,        0,     0, 0,  0,    "",     0, "fragm%65nt"));
} END_TEST

Suite *liuri_suite(void) {
    Suite *s = suite_create("liuri");
    TCase *tc;

    tc = tcase_create("ipv4");
    tcase_add_test(tc, valid_ipv4);
    tcase_add_test(tc, invalid_ipv4);
    suite_add_tcase(s, tc);

    tc = tcase_create("ipv6");
    tcase_add_test(tc, valid_ipv6);
    tcase_add_test(tc, valid_ipv6_with_ipv4);
    tcase_add_test(tc, invalid_ipv6);
    suite_add_tcase(s, tc);

    tc = tcase_create("ipvfuture");
    tcase_add_test(tc, valid_ipvfuture);
    suite_add_tcase(s, tc);

    tc = tcase_create("liuri_parse");
    tcase_add_test(tc, empty_uri);
    tcase_add_test(tc, uri_scheme);
    tcase_add_test(tc, uri_authority);
    tcase_add_test(tc, uri_userinfo);
    tcase_add_test(tc, uri_host);
    tcase_add_test(tc, uri_port);
    tcase_add_test(tc, uri_path);
    tcase_add_test(tc, uri_query);
    tcase_add_test(tc, uri_fragment);
    suite_add_tcase(s, tc);

    return s;
}

int main(int argc, char **argv) {
    Suite *s = liuri_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free (sr);
    return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
