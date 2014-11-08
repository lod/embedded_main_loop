/* CC0 (Public domain) - see LICENSE file for details */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "event_loop.h"
#include "event_source.h"
#include "dl_list.h"
#include "ms_count.h"

static struct event_source worst;
static void new_destructor(event_loop this);
static void init_destructor(event_loop this);

event_loop event_loop_new(void) {
	event_loop this = malloc(sizeof(struct event_loop));
	event_loop_init(this);
	this->destructor = new_destructor;
	return this;
}

void event_loop_init(event_loop this) {
	// Only set the destructor if it hasn't been set by new()
	// Allows init() to be called after new()
	if (this->destructor != new_destructor) {
		this->destructor = init_destructor;
	}

	dl_list_init(&this->list);
}

static void new_destructor(event_loop this) {
	init_destructor(this);
	free(this);
}

static void init_destructor(event_loop this) {
	// Destroy all of the list elements
	while (this->list.next != &this->list) {
		event_source element = event_source_from_list(this->list.next);
		event_source_destroy(element);
	}
}

static struct event_source worst = {
	.check      = NULL,
	.dispatch   = NULL,
	.priority   = INT16_MAX,
	.ready_time = 0,
	.list = {
		.next = &worst.list,
		.prev = &worst.list
	},
};

void event_loop_run_once(event_loop this) {
	uint64_t current_time_ms = ms_count();

	event_source best = &worst;
	struct dl_list *element_list, *start;
	for (start = &this->list, element_list = this->list.next;
			element_list != start;
			element_list = element_list->next
	) {
		event_source element = event_source_from_list(element_list);

		if (element->priority < best->priority
			&& element->ready_time <= current_time_ms
			&& (element->check == NULL || element->check(element))
		) {
			best = element;
		}
	}

	if (best->dispatch != NULL) {
		best->dispatch(best);
	}
}

event_source event_loop_add_new_source(event_loop this,
	bool (*check) (struct event_source* event_source),
	bool (*dispatch) (struct event_source* event_source),
	uint16_t priority
) {
	event_source source = event_source_new(check, dispatch, priority);
	event_loop_add_source(this, source);
	return source;
}
