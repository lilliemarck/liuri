#include <check.h>
#include <stdlib.h>
#include "liuri.h"

START_TEST(fail_test) {
    ck_abort_msg("Failed...");
}
END_TEST

Suite *liuri_suite(void) {
    Suite *s = suite_create("liuri");

    TCase *tc = tcase_create("tests");
    tcase_add_test(tc, fail_test);
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
