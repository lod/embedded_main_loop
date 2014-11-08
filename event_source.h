#ifndef EVENT_SOURCE_H
#define EVENT_SOURCE_H

/* CC0 (Public domain) - see LICENSE file for details */

#include <stdint.h>
#include <stdbool.h>
#include "dl_list.h"

struct event_source {
	bool (*check)    (struct event_source* event_source);
	bool (*dispatch) (struct event_source* event_source);
	void (*destructor) (struct event_source* event_source);
	int16_t priority;
	uint64_t ready_time;
	struct dl_list list;
};
typedef struct event_source* event_source;

struct event_source* event_source_new(
	bool (*check) (struct event_source* event_source),
    bool (*dispatch) (struct event_source* event_source),
    uint16_t priority
);

void event_source_init(struct event_source*,
	bool (*check) (struct event_source* event_source),
    bool (*dispatch) (struct event_source* event_source),
    uint16_t priority
);

// void event_source_destroy(struct event_source*);
#define event_source_destroy(this) ({ \
		if((this)->destructor) \
			(this)->destructor(this); \
	})

// event_source event_source_from_list(struct dl_list* list)
#define event_source_from_list(list_entry) \
	dl_list_entry((list_entry), struct event_source, list)

#endif
