/* CC0 (Public domain) - see LICENSE file for details */

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "../dl_list.h"

START_TEST (test_walk) {
	struct dl_list _a, _b, _c, _d;
	struct dl_list* a = &_a;
	struct dl_list* b = &_b;
	struct dl_list* c = &_c;
	struct dl_list* d = &_d;

	dl_list_init(a);
	ck_assert_int_eq(a->next, a);
	ck_assert_int_eq(a->prev, a);

	// A-B-A
	dl_list_add_prev(a, b);
	ck_assert_int_eq(a->next, b);
	ck_assert_int_eq(b->next, a);
	ck_assert_int_eq(a->prev, b);
	ck_assert_int_eq(b->prev, a);

	// A-B-C-A
	dl_list_add_prev(a, c);
	ck_assert_int_eq(a->next, b);
	ck_assert_int_eq(b->next, c);
	ck_assert_int_eq(c->next, a);
	ck_assert_int_eq(a->prev, c);
	ck_assert_int_eq(c->prev, b);
	ck_assert_int_eq(b->prev, a);

	// A-D-B-C-A
	dl_list_add_next(a, d);
	ck_assert_int_eq(a->next, d);
	ck_assert_int_eq(d->next, b);
	ck_assert_int_eq(b->next, c);
	ck_assert_int_eq(c->next, a);
	ck_assert_int_eq(a->prev, c);
	ck_assert_int_eq(c->prev, b);
	ck_assert_int_eq(b->prev, d);
	ck_assert_int_eq(d->prev, a);
} END_TEST

START_TEST (test_foreach) {
	struct dl_list _a, _b, _c, _d;
	struct dl_list* a = &_a;
	struct dl_list* b = &_b;
	struct dl_list* c = &_c;
	struct dl_list* d = &_d;

    dl_list_init(a);
	dl_list_add_prev(a, b);
	dl_list_add_prev(a, c);
	dl_list_add_prev(a, d);

	struct dl_list* expected_arr[] = {a, b, c, d};
	int expected = 1; // Don't include the first item in the loop
	struct dl_list* item;
	ck_assert_int_ne(a, item);
	dl_list_for_each(a, item) {
		ck_assert_int_eq(item, expected_arr[expected++]);
	}
	ck_assert_int_eq(expected, 4);
} END_TEST

struct test_struct {
    struct dl_list first;
    int val_first_mid;
    struct dl_list mid;
    int val_mid_last;
    struct dl_list last;
};

START_TEST (test_entry) {
    struct test_struct ts;
    ck_assert_int_eq(dl_list_entry(&ts.first, struct test_struct, first), &ts);
    ck_assert_int_eq(dl_list_entry(&ts.mid, struct test_struct, mid), &ts);
    ck_assert_int_eq(dl_list_entry(&ts.last, struct test_struct, last), &ts);

    // Ensures typecast is correct
    struct test_struct* tsp = dl_list_entry(&ts.last, struct test_struct, last);
    ck_assert_int_eq(tsp, &ts);
} END_TEST

START_TEST (test_foreach_entry) {
	struct test_struct _a, _b, _c, _d;
	struct test_struct* a = &_a;
	struct test_struct* b = &_b;
	struct test_struct* c = &_c;
	struct test_struct* d = &_d;

    dl_list_init(&a->first);
	dl_list_add_prev(&a->first, &b->first);
	dl_list_add_prev(&a->first, &c->first);
	dl_list_add_prev(&a->first, &d->first);
    /*
	dl_list_add_prev(a.first, a.mid);
	dl_list_add_prev(a.first, b.mid);
	dl_list_add_prev(a.first, c.mid);
	dl_list_add_prev(a.first, d.mid);
	dl_list_add_prev(a.first, a.last);
	dl_list_add_prev(a.first, b.last);
	dl_list_add_prev(a.first, c.last);
	dl_list_add_prev(a.first, d.last);
    */

	struct test_struct* expected_arr[] = {a, b, c, d /*, a, b, c, d, a, b, c, d*/};
	int expected = 1; // Don't include the first item in the loop
	struct test_struct* item;
	dl_list_for_each_entry(&a->first, item, struct test_struct, first) {
		ck_assert_int_eq(item, expected_arr[expected++]);
	}
	ck_assert_int_eq(expected, 4);
} END_TEST


Suite * money_suite (void) {
	Suite *s = suite_create ("dl_list");

	/* Core test case */
	TCase *tc_core = tcase_create ("Core");
	tcase_add_test (tc_core, test_walk);
	tcase_add_test (tc_core, test_foreach);
	tcase_add_test (tc_core, test_entry);
	tcase_add_test (tc_core, test_foreach_entry);
	suite_add_tcase (s, tc_core);

	return s;
}

int main (void) {
	Suite *s = money_suite ();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
