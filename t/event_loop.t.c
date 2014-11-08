/* CC0 (Public domain) - see LICENSE file for details */

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "../event_loop.h"
#include "../event_source.h"

uint64_t ms_count_time = 1000;
uint64_t ms_count(void) { // Called by event loop, defined in ms_count.h
	return ms_count_time;
}

struct test_es {
	struct event_source;
	unsigned int count;
	bool ready;
};

bool check_test_es(struct test_es* source) {
	return source->ready;
}

void run_test_es(struct test_es* source) {
	source->count++;
}

struct test_es ts1;
struct test_es ts2;
struct test_es ts3;
struct test_es ts4;
struct test_es ts5;
struct test_es ts6;
struct test_es ts7;
struct test_es ts8;
struct test_es ts9;
struct test_es ts10;

#define INIT_SOURCE(ts) event_source_init((struct event_source*)&ts, (bool(*)(struct event_source*))check_test_es, (bool(*)(struct event_source*))run_test_es, 10);
void setup(void) {
	INIT_SOURCE(ts1);
	INIT_SOURCE(ts2);
	INIT_SOURCE(ts3);
	INIT_SOURCE(ts4);
	INIT_SOURCE(ts5);
	INIT_SOURCE(ts6);
	INIT_SOURCE(ts7);
	INIT_SOURCE(ts8);
	INIT_SOURCE(ts9);
	INIT_SOURCE(ts10);
}

void teardown(void) {
}

START_TEST (test_new) {
	int start_mem = mallinfo().uordblks;

	event_loop el = event_loop_new();
	ck_assert_int_ne(start_mem, mallinfo().uordblks);

	event_loop_destroy(el);

	ck_assert_int_eq(start_mem, mallinfo().uordblks);
} END_TEST

START_TEST (test_init) {
	struct event_loop el;
	int start_mem = mallinfo().uordblks;

	event_loop_init(&el);
	ck_assert_int_eq(start_mem, mallinfo().uordblks);

	event_loop_destroy(&el);
	ck_assert_int_eq(start_mem, mallinfo().uordblks);
	
	// Ensure calling init after calling new doesn't mess the destructor
	event_loop eln = event_loop_new();
	int new_mem = mallinfo().uordblks;
	ck_assert_int_ne(start_mem, new_mem);

	event_loop_init(eln);
	ck_assert_int_eq(new_mem, mallinfo().uordblks);

	event_loop_destroy(eln);
	ck_assert_int_eq(start_mem, mallinfo().uordblks);
} END_TEST

START_TEST (test_destructor) {
	struct event_loop el;
	int start_mem = mallinfo().uordblks;
	event_loop_init(&el);

	event_loop_add_source(&el, (event_source)&ts1);
	event_loop_add_source(&el, (event_source)&ts2);
	event_loop_add_source(&el, (event_source)&ts3);

	ck_assert_int_eq(el.list.next, &ts1.list);
	ck_assert_int_eq(el.list.next->next, &ts2.list);
	ck_assert_int_eq(el.list.next->next->next, &ts3.list);
	ck_assert_int_eq(el.list.next->next->next->next, &el.list);

	event_loop_destroy(&el);

	// Every element should just be pointing to itself now
	ck_assert_int_eq(el.list.next,  &el.list);
	ck_assert_int_eq(el.list.prev,  &el.list);
	ck_assert_int_eq(ts1.list.next, &ts1.list);
	ck_assert_int_eq(ts1.list.prev, &ts1.list);
	ck_assert_int_eq(ts2.list.next, &ts2.list);
	ck_assert_int_eq(ts2.list.prev, &ts2.list);
	ck_assert_int_eq(ts3.list.next, &ts3.list);
	ck_assert_int_eq(ts3.list.prev, &ts3.list);

	ck_assert_int_eq(start_mem, mallinfo().uordblks);
} END_TEST

START_TEST (event_choice) {
	event_loop el = event_loop_new();

	// No events, should run ok (but not do much)
	ck_assert_int_eq(1, 1);
	event_loop_run_once(el);

	event_loop_add_source(el, (event_source)&ts1);
	event_loop_add_source(el, (event_source)&ts2);
	event_loop_add_source(el, (event_source)&ts3);
	event_loop_add_source(el, (event_source)&ts4);
	event_loop_add_source(el, (event_source)&ts5);
	ck_assert_int_eq(1, 1);

	// Events aren't ready
	event_loop_run_once(el);
	ck_assert_int_eq(ts1.count + ts2.count + ts3.count + ts4.count + ts5.count, 0);

	// One event ready to run
	ts2.ready = true;
	event_loop_run_once(el);
	ck_assert_int_eq(ts2.count, 1);
	ck_assert_int_eq(ts1.count + ts3.count + ts4.count + ts5.count, 0);

	// Still ready to run
	event_loop_run_once(el);
	ck_assert_int_eq(ts2.count, 2);
	ck_assert_int_eq(ts1.count + ts3.count + ts4.count + ts5.count, 0);

	// All ready to run, run highest priority (lowest number)
	ts1.priority = 0;   ts1.ready = true; ts1.count = 0; ts1.ready_time = 0;
	ts2.priority = -10; ts2.ready = true; ts2.count = 0; ts2.ready_time = 0;
	ts3.priority = 10;  ts3.ready = true; ts3.count = 0; ts3.ready_time = 0;
	ts4.priority = -20; ts4.ready = true; ts4.count = 0; ts4.ready_time = 0;
	ts5.priority = 20;  ts5.ready = true; ts5.count = 0; ts5.ready_time = 0;
	event_loop_run_once(el);
	ck_assert_int_eq(ts4.count, 1);
	ck_assert_int_eq(ts1.count + ts2.count + ts3.count + ts5.count, 0);

	// Then run next highest priority - ts2
	ts4.ready = false;
	event_loop_run_once(el);
	ck_assert_int_eq(ts2.count, 1);
	ck_assert_int_eq(ts4.count, 1);
	ck_assert_int_eq(ts1.count + ts3.count + ts5.count, 0);

	// If the ready time is in the future, it isn't ready
	ts1.priority = 0;   ts1.ready = true; ts1.count = 0; ts1.ready_time = UINT64_MAX;
	ts2.priority = -10; ts2.ready = true; ts2.count = 0; ts2.ready_time = 0;
	ts3.priority = 10;  ts3.ready = true; ts3.count = 0; ts3.ready_time = 0;
	ts4.priority = -20; ts4.ready = true; ts4.count = 0; ts4.ready_time = UINT64_MAX;
	ts5.priority = 20;  ts5.ready = true; ts5.count = 0; ts5.ready_time = UINT64_MAX;
	event_loop_run_once(el);
	ck_assert_int_eq(ts2.count, 1);
	ck_assert_int_eq(ts1.count + ts4.count + ts3.count + ts5.count, 0);

	// Then run next highest priority - ts3
	ts2.ready = false;
	event_loop_run_once(el);
	ck_assert_int_eq(ts3.count, 1);
	ck_assert_int_eq(ts2.count, 1);
	ck_assert_int_eq(ts1.count + ts4.count + ts5.count, 0);

	// If there is no check function, it is the same as returning true
	ts1.priority = 0;   ts1.ready = false; ts1.count = 0; ts1.ready_time = 0;
	ts2.priority = -10; ts2.ready = false; ts2.count = 0; ts2.ready_time = 0;
	ts3.priority = 10;  ts3.ready = false; ts3.count = 0; ts3.ready_time = 0;
	ts4.priority = -20; ts4.ready = false; ts4.count = 0; ts4.ready_time = 0;
	ts5.priority = 20;  ts5.ready = false; ts5.count = 0; ts5.ready_time = 0;
	ts2.check = NULL;
	event_loop_run_once(el);
	ck_assert_int_eq(ts2.count, 1);
	ck_assert_int_eq(ts1.count + ts4.count + ts3.count + ts5.count, 0);
	ts2.check = (bool(*)(struct event_source*))check_test_es;


	// Confusing mix of all options... just in case
	ts1.priority = 0;   ts1.ready = true;  ts1.count = 0; ts1.ready_time = 0;
	ts2.priority = -10; ts2.ready = false; ts2.count = 0; ts2.ready_time = UINT64_MAX;
	ts3.priority = 10;  ts3.ready = true;  ts3.count = 0; ts3.ready_time = UINT64_MAX;
	ts4.priority = -20; ts4.ready = false; ts4.count = 0; ts4.ready_time = UINT64_MAX;
	ts5.priority = 20;  ts5.check = NULL;  ts5.count = 0; ts5.ready_time = 0;

	// ts1 first
	event_loop_run_once(el);
	ck_assert_int_eq(ts1.count, 1);

	// ts2 is ready now
	ts2.ready = true; ts2.ready_time = 0;
	event_loop_run_once(el);
	ck_assert_int_eq(ts2.count, 1);

	// ts5 is higher priority
	ts5.priority = -55;
	event_loop_run_once(el);
	ck_assert_int_eq(ts5.count, 1);

	// Reduce ts5 priority down again
	ts5.priority = 20;
	event_loop_run_once(el);
	ck_assert_int_eq(ts2.count, 2);
} END_TEST

START_TEST (sources) {
	event_loop el = event_loop_new();
	INIT_SOURCE(ts2);
	dl_list_init(&ts3.list);

	event_loop_add_source(el, (event_source)&ts2);
	event_loop_add_source(el, (event_source)&ts1);
	event_loop_add_source(el, (event_source)&ts3);

	ck_assert_int_eq(el->list.prev, &ts3.list);
	ck_assert_int_eq(el->list.prev->prev, &ts1.list);
	ck_assert_int_eq(el->list.prev->prev->prev, &ts2.list);
	ck_assert_int_eq(el->list.prev->prev->prev->prev, &el->list);

	ck_assert_int_eq(el->list.next, &ts2.list);
	ck_assert_int_eq(el->list.next->next, &ts1.list);
	ck_assert_int_eq(el->list.next->next->next, &ts3.list);
	ck_assert_int_eq(el->list.next->next->next->next, &el->list);

	event_loop_del_source(el, (event_source)&ts1);

	ck_assert_int_eq(el->list.prev, &ts3.list);
	ck_assert_int_eq(el->list.prev->prev, &ts2.list);
	ck_assert_int_eq(el->list.prev->prev->prev, &el->list);

	ck_assert_int_eq(el->list.next, &ts2.list);
	ck_assert_int_eq(el->list.next->next, &ts3.list);
	ck_assert_int_eq(el->list.next->next->next, &el->list);

	event_loop_del_source(el, dl_list_entry(el->list.next, struct event_source, list));

	ck_assert_int_eq(el->list.prev, &ts3.list);
	ck_assert_int_eq(el->list.prev->prev, &el->list);

	ck_assert_int_eq(el->list.next, &ts3.list);
	ck_assert_int_eq(el->list.next->next, &el->list);

	dl_list_del(&ts3.list);

	ck_assert_int_eq(el->list.prev, &el->list);
	ck_assert_int_eq(el->list.next, &el->list);

	ck_assert_int_eq(ts1.list.next, &ts1.list);
	ck_assert_int_eq(ts1.list.prev, &ts1.list);
	ck_assert_int_eq(ts2.list.next, &ts2.list);
	ck_assert_int_eq(ts2.list.prev, &ts2.list);
	ck_assert_int_eq(ts3.list.next, &ts3.list);
	ck_assert_int_eq(ts3.list.prev, &ts3.list);
} END_TEST

Suite * mysuite (void) {
	Suite *s = suite_create ("event_loop");

	/* Core test case */
	TCase *tc_core = tcase_create ("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test (tc_core, test_new);
	tcase_add_test (tc_core, test_init);
	tcase_add_test (tc_core, test_destructor);
	tcase_add_test (tc_core, event_choice);
	tcase_add_test (tc_core, sources);
	suite_add_tcase (s, tc_core);

	return s;
}

int main (void) {
	Suite *s = mysuite ();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
