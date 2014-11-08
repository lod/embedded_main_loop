#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

/* CC0 (Public domain) - see LICENSE file for details */

#include <stdint.h>
#include <stdbool.h>

#include "event_source.h"
#include "dl_list.h"

typedef struct event_loop {
	struct dl_list list;
	void (*destructor) (struct event_loop* this);
} *event_loop;

event_loop event_loop_new(void);
void event_loop_init(event_loop this);
void event_loop_run_once(event_loop this);

// void event_loop_destroy(event_loop this);
#define event_loop_destroy(this) ({ \
		if((this)->destructor) \
			(this)->destructor(this); \
	})


// void event_loop_add_source(event_loop this, event_source source);
#define event_loop_add_source(this, source) dl_list_add_prev(&(this)->list, &(source)->list)

// void event_loop_del_source(event_loop this, event_source source);
#define event_loop_del_source(this, source) dl_list_del(&(source)->list)

// void event_loop_run(event_loop this);
#define event_loop_run(this) do { event_loop_run_once(this); } while (1)

#endif
