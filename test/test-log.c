// -*- mode: c -*-
/****************************************************************
COPYRIGHT_BEGIN
Copyright (C) 2015, 0x1 Consulting.
All rights reserved.
COPYRIGHT_END
****************************************************************/

#include "config.h"
#include "cobaro-log0/log.h"
#include "greatest.h"
#include "messages.h"

#define SEND_COUNT (1000)
#define NUM_PRODUCERS (4)

// Global for your testing convenience
cobaro_loghandle_t lh;

static void setup_cb(void *data)
{
    lh = cobaro_log_init(cobaro_messages_en);
}

static void teardown_cb(void *data)
{
    cobaro_log_fini(lh);
}

// consume on 0, produce on 1
void *consumer_main(void *rock)
{
    int received = 0, loopcount = 0;
    cobaro_loghandle_t lh = (cobaro_loghandle_t) rock;
    cobaro_log_t log;

    while (received < SEND_COUNT * NUM_PRODUCERS) {
        if ((log = cobaro_log_next(lh))) {
            cobaro_log_return(lh, log);
            received++;
        }
        loopcount++;
        usleep(1);
    }

    fprintf(stderr,"Received %d, looped %d\n", received, loopcount);
    pthread_exit(rock);
}

// produce on 0, consume on 1
void *producer_main(void *rock)
{
    cobaro_loghandle_t lh = (cobaro_loghandle_t) rock;
    cobaro_log_t log;
    int sent = 0, loopcount = 0;

    while (sent < SEND_COUNT) {
        log = cobaro_log_claim(lh);
        if (!log) {
            loopcount++;
            usleep(1);
        } else {
            cobaro_log_publish(lh, log);
            sent++;
        }
    }
    fprintf(stderr,"Sent %d, looped %d\n", sent, loopcount);
    pthread_exit(rock);
}
GREATEST_TEST log_size() {
    struct cobaro_log log;
    
    GREATEST_ASSERT(512 == sizeof(log));

    GREATEST_PASS();
}

GREATEST_TEST log_communication() {
    pthread_t thread[NUM_PRODUCERS + 1]; 
    pthread_attr_t attr;
    void *retval;

    GREATEST_ASSERT_NOT_NULL(lh);
    
    // Create one consumer and three producer threads
    GREATEST_ASSERT(0 == pthread_attr_init(&attr));
    GREATEST_ASSERT(0 == pthread_create(&thread[0], &attr,
                                        consumer_main,
                                        (void *)lh));
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        GREATEST_ASSERT(0 == pthread_create(&thread[i+1], &attr,
                                            producer_main,
                                            (void *)lh));
    }

    // clean up
    GREATEST_ASSERT(0 == pthread_join(thread[0], &retval));
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        GREATEST_ASSERT(0 == pthread_join(thread[i+1], &retval));
    }

    GREATEST_PASS();
}


#define TEST_OUT1 "string literal"
#define TEST_OUT2 "s:string literal, i:42, f:42, ip:42.0.0.0, percent:%"

GREATEST_TEST log_messages() {
    struct cobaro_log log;
    char s[256];
    
    log.code = COBARO_TEST_MESSAGE_NULL;
    log.level = COBARO_LOG_WARNING;
    log.p[0].type = COBARO_STRING;
    strncpy(log.p[0].s, TEST_OUT1, sizeof(log.p[0].s));
    log.p[1].type = COBARO_INTEGER;
    log.p[1].i = 42;
    log.p[2].type = COBARO_REAL;
    log.p[2].f = 42.0;
    log.p[3].type = COBARO_IPV4;
    log.p[3].ipv4 = 42;

    // simple string literal
    GREATEST_ASSERT(true == cobaro_log_to_string(lh, &log, s, sizeof(s)));
    GREATEST_ASSERT(0 == strcmp(TEST_OUT1, s));

    // Not enough room in s
    GREATEST_ASSERT(false == cobaro_log_to_string(lh, &log, s, (size_t) 5));

    // One of each type
    log.code = COBARO_TEST_MESSAGE_TYPES;
    GREATEST_ASSERT(cobaro_log_to_string(lh, &log, s, sizeof(s)));
    GREATEST_ASSERT(0 == strcmp(TEST_OUT2, s));


    // Now check that without the space for the trailing null
    GREATEST_ASSERT(false == cobaro_log_to_string(lh, &log, s, strlen(TEST_OUT2)));
    // Now check that with a space for the trailing null
    GREATEST_ASSERT(true == cobaro_log_to_string(lh, &log, s, strlen(TEST_OUT2) + 1));


    // Log that to file
    GREATEST_ASSERT(true == cobaro_log_to_file(lh, &log, stdout));

    // Log to syslog
    GREATEST_ASSERT(true == cobaro_log_syslog_set(lh, "test-log",
                                                  LOG_PID, LOG_LOCAL0));

    strncpy(log.p[0].s, "no", sizeof(log.p[0].s));
    GREATEST_ASSERT(true == cobaro_log_loglevel_set(lh, LOG_EMERG));
    GREATEST_ASSERT(true == cobaro_log(lh, &log));

    strncpy(log.p[0].s, "yes", sizeof(log.p[0].s));
    GREATEST_ASSERT(true == cobaro_log_loglevel_set(lh, LOG_DEBUG));
    GREATEST_ASSERT(true == cobaro_log_to_syslog(lh, &log));


    // Switch back to file
    strncpy(log.p[0].s, "stdout", sizeof(log.p[0].s));
    cobaro_log_file_set(lh, stdout);
    GREATEST_ASSERT(true == cobaro_log_loglevel_set(lh, LOG_CRIT));
    GREATEST_ASSERT(true == cobaro_log(lh, &log));
    
    GREATEST_PASS();
}

GREATEST_SUITE(cobaro_test_log) {
    SET_SETUP(setup_cb, NULL);
    SET_TEARDOWN(teardown_cb, NULL);

    GREATEST_RUN_TEST(log_size);
    GREATEST_RUN_TEST(log_messages);
    GREATEST_RUN_TEST(log_communication);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int
main(
    int argc,
    char **argv)
{
    GREATEST_MAIN_BEGIN();      /* init & parse command-line args */
    GREATEST_RUN_SUITE(cobaro_test_log);
    GREATEST_MAIN_END();        /* display results */
}
