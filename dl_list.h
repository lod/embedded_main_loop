#ifndef DL_LIST_H
#define DL_LIST_H

/* CC0 (Public domain) - see LICENSE file for details */

#include <stddef.h> // offsetof

// Based on the Linux kernel linked list implementation
// See http://kernelnewbies.org/FAQ/LinkedLists for an introduction

struct dl_list {
	struct dl_list* next;
	struct dl_list* prev;
};

static inline void dl_list_init(struct dl_list* list) {
	list->next = list;
	list->prev = list;
}

// Add item to list
static inline void dl_list_add_next(struct dl_list* list, struct dl_list* item) {
	item->next = list->next;
	list->next->prev = item;
	list->next = item;
	item->prev = list;
}

// Add item to list
static inline void dl_list_add_prev(struct dl_list* list, struct dl_list* item) {
	item->prev = list->prev;
	list->prev->next = item;
	list->prev = item;
	item->next = list;
}

static inline void dl_list_del(struct dl_list* item) {
	item->next->prev = item->prev;
	item->prev->next = item->next;
	dl_list_init(item);
}

#define dl_list_entry(ptr, type, member) (type*)( (uint8_t *)ptr - offsetof(type,member) )

// NOTE: Doesn't iterate through the current item
#define dl_list_for_each(list, entry) \
	for (entry = (list)->next; entry != (list); entry = entry->next)


// NOTE: Doesn't iterate through the current item
#define dl_list_for_each_entry(list, entry, type, member) for ( \
		entry = dl_list_entry((list)->next, type, member); \
		&entry->member != (list); \
		entry = dl_list_entry(entry->member.next, type, member) \
	)

#endif
