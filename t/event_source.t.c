/* CC0 (Public domain) - see LICENSE file for details */

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "../event_source.h"

#define TEST_INIT_ARGS (bool(*)(struct event_source*))23, (bool(*)(struct event_source*))2323, 85

START_TEST (test_new) {
	int start_mem = mallinfo().uordblks;

	event_source es = event_source_new(TEST_INIT_ARGS);
	ck_assert_int_ne(start_mem, mallinfo().uordblks);

	ck_assert_int_eq(es->check, 23);
	ck_assert_int_eq(es->dispatch, 2323);
	ck_assert_int_eq(es->priority, 85);
	ck_assert_int_eq(es->ready_time, 0);
	ck_assert_int_eq(es->list.next, &es->list);
	ck_assert_int_eq(es->list.prev, &es->list);

	event_source_destroy(es);
	ck_assert_int_eq(start_mem, mallinfo().uordblks);
} END_TEST

START_TEST (test_destructor) {
	// Destructor should remove us from a list

	struct event_source _es;

	int start_mem = mallinfo().uordblks;

	event_source es2 = &_es;
	event_source_init(es2, TEST_INIT_ARGS);

	event_source es1 = event_source_new(TEST_INIT_ARGS);
	event_source es3 = event_source_new(TEST_INIT_ARGS);

	dl_list_add_prev(&es1->list, &es2->list);
	dl_list_add_prev(&es1->list, &es3->list);

	ck_assert_int_eq(es1->list.next, &es2->list);
	ck_assert_int_eq(es2->list.next, &es3->list);
	ck_assert_int_eq(es3->list.next, &es1->list);

	event_source_destroy(es2);

	ck_assert_int_eq(es1->list.next, &es3->list);
	ck_assert_int_eq(es3->list.next, &es1->list);

	ck_assert_int_eq(es2->list.next, &es2->list);
	ck_assert_int_eq(es2->list.prev, &es2->list);

	event_source_destroy(es1);

	ck_assert_int_eq(es3->list.next, &es3->list);
	ck_assert_int_eq(es3->list.prev, &es3->list);

	event_source_destroy(es3);

	ck_assert_int_eq(start_mem, mallinfo().uordblks);
} END_TEST

START_TEST (test_from_list) {
	event_source es = event_source_new(TEST_INIT_ARGS);
	ck_assert_int_eq(event_source_from_list(&es->list), es);
	event_source_destroy(es);
} END_TEST

START_TEST (test_init) {
	struct event_source _es;
	event_source es = &_es;
	int start_mem = mallinfo().uordblks;

	event_source_init(es, TEST_INIT_ARGS);
	ck_assert_int_eq(start_mem, mallinfo().uordblks);

	ck_assert_int_eq(es->check, 23);
	ck_assert_int_eq(es->dispatch, 2323);
	ck_assert_int_eq(es->priority, 85);
	ck_assert_int_eq(es->ready_time, 0);
	ck_assert_int_eq(es->list.next, &es->list);
	ck_assert_int_eq(es->list.prev, &es->list);

	event_source_destroy(es);
	ck_assert_int_eq(start_mem, mallinfo().uordblks);
} END_TEST

Suite * mysuite (void) {
	Suite *s = suite_create("event_source");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_new);
	tcase_add_test(tc_core, test_init);
	tcase_add_test(tc_core, test_destructor);
	tcase_add_test(tc_core, test_from_list);
	suite_add_tcase(s, tc_core);

	return s;
}

int main (void) {
	Suite *s = mysuite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
