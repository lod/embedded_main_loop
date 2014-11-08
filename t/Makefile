# CC0 (Public domain) - see LICENSE file for details

CFLAGS=-std=gnu99 -Wall -fms-extensions -g
LDFLAGS=-lrt -lpthread -lcheck -lm
LDLIBS=/usr/lib/x86_64-linux-gnu/libcheck.a

dl_list.t: dl_list.t.o
event_loop.t: event_loop.t.o ../event_loop.o ../event_source.o
event_source.t: event_source.t.o ../event_source.o

%: %.t
	./$<

%.gdb: %.t
	CK_FORK=no LANG=C gdb ./$<

.PHONY: % %.gdb
