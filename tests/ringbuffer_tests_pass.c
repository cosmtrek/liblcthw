#include "minunit.h"
#include <lcthw/ring_buffer.h>
#include <assert.h>
#include <lcthw/bstrlib.h>
#include <stdlib.h>

static RingBuffer *buffer = NULL;
char *test = "testdata";
char *data = NULL;

#define LENGTH 9

char *test_create()
{
    buffer = RingBuffer_create(LENGTH);
    mu_assert(buffer != NULL, "Failed to create Ring buffer.");

    return NULL;
}

char *test_destroy()
{
    RingBuffer_destroy(buffer);

    return NULL;
}

char *test_read_write()
{
    data = malloc(sizeof(test));

    debug("--- before write ---\n");
    debug("buffer has: %s", buffer->buffer);
    debug("buffer start: %d", buffer->start);
    debug("buffer end: %d", buffer->end);

    mu_assert(RingBuffer_empty(buffer) == 1, "Buffer should be empty.");

    int rc = RingBuffer_write(buffer, test, 8);
    mu_assert(rc == 8, "Failed to write data to the buffer.");
    debug("--- after write ---\n");
    debug("buffer has: %s", buffer->buffer);
    debug("buffer start: %d", buffer->start);
    debug("buffer end: %d", buffer->end);
    debug("buffer length: %d", buffer->length);

    rc = RingBuffer_read(buffer, data, 8);
    mu_assert(rc == 8, "Failed to read data from the buffer.");

    debug("--- after read ---\n");
    debug("buffer has: %s", buffer->buffer);
    debug("buffer start: %d", buffer->start);
    debug("buffer end: %d", buffer->end);
    debug("buffer length: %d", buffer->length);

    mu_assert(RingBuffer_empty(buffer) == 1, "Buffer should be empty.");

    return NULL;
}

char *all_tests()
{
    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_read_write);
    mu_run_test(test_destroy);

    return NULL;
}

RUN_TESTS(all_tests);
