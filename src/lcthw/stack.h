#ifndef _Stack_h
#define _Stack_h

#include <lcthw/list.h>

typedef List Stack;

#define Stack_create() (List_create())
#define Stack_destroy(S) (List_destroy((S)))
#define Stack_push(S, V) (List_push((S), (V)))
#define Stack_pop(S) (List_pop((S)))
#define Stack_peek(S) (List_last((S)))
#define Stack_count(S) (List_count((S)))
#define STACK_FOREACH(S, V) LIST_FOREACH((S), first, next, (V))

#endif
