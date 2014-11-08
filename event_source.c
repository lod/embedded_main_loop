/* CC0 (Public domain) - see LICENSE file for details */

#include <stdbool.h>
#include <stdint.h>

#include <stdlib.h>

#include "event_source.h"
#include "dl_list.h"

static void cleanup(struct event_source* this);
static void destroy(struct event_source* this);

struct event_source* event_source_new(
	bool (*check) (struct event_source* event_source),
    bool (*dispatch) (struct event_source* event_source),
    uint16_t priority
) {
    event_source this = malloc(sizeof(struct event_source));
    event_source_init(this, check, dispatch, priority);
    this->destructor = &destroy;
    return this;
}

void event_source_init(
    struct event_source* this,
	bool (*check) (struct event_source* event_source),
    bool (*dispatch) (struct event_source* event_source),
    uint16_t priority
) {
    this->check = check;
    this->dispatch = dispatch;
    this->priority = priority;
    this->destructor = &cleanup;
    this->ready_time = 0;
    dl_list_init(&this->list);
}

static void cleanup(struct event_source* this) {
    dl_list_del(&this->list);
}

static void destroy(struct event_source* this) {
    cleanup(this);
    free(this);
}
