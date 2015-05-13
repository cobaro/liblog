// -*- mode: c -*-
/****************************************************************
COPYRIGHT_BEGIN
Copyright (C) 2015, cobaro.org
All rights reserved.
COPYRIGHT_END
****************************************************************/

#include "config.h"
#include "libcobaro-log0/log.h"

#if defined(HAVE_ARPA_INET_H)
# include <arpa/inet.h>
#endif

#if defined(HAVE_ERRNO_H)
# include <errno.h>
#endif

#if defined(HAVE_PTHREAD_H)
# include <pthread.h>
#endif

#if defined(HAVE_NETINET_IN_H)
# include <netinet/in.h>
#endif

#if defined(HAVE_SYSLOG_H)
# include <syslog.h>
#endif

#if defined(HAVE_SYS_TIME_H)
#  include <sys/time.h>
#endif

#if defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif

#if !defined(HAVE_PTHREAD_SPINLOCKS)
#  include "spin.h"
#endif


#define COBARO_LOG_SLOTS (16) // Keep it small as we have limited cache


/// Valid logging destinations
enum cobaro_logto_t {
    COBARO_LOGTO_FILE,
    COBARO_LOGTO_SYSLOG
};

struct cobaro_loghandle {
    cobaro_log_t free;       // free logs
    cobaro_log_t busy;       // currently used logs
    pthread_spinlock_t lock; // locking

    int level;               // messages higher than this are not logged
    char **messages;         // Array of format strings
    int logto;               // log destination
    FILE *f;                 // if logging to file

    cobaro_log_t blocks;     // memory for cleanup on exit
};

 // Per-thread
 cobaro_loghandle_t cobaro_log_init(char **messages)
 {
     cobaro_loghandle_t lh;

     if (!(lh = (cobaro_loghandle_t) calloc(1, sizeof(struct cobaro_loghandle)))) {
         return NULL;
     }

     // let's get them all as a bunch in memory. After this they can
     // get jumbled up but on shutdown we can free the lot in one
     // go
     lh->blocks = (cobaro_log_t) calloc(COBARO_LOG_SLOTS, sizeof(struct cobaro_log));
     for (int i = 0; i < COBARO_LOG_SLOTS - 1; i++) {
         lh->blocks[i].next = &lh->blocks[i + 1];
     }

     lh->free = lh->blocks;
     lh->busy = NULL;
     if (pthread_spin_init(&lh->lock, 1)) {
         fprintf(stderr, "pthread_spin_init() failed\n");
         abort();
     }

     lh->logto = COBARO_LOGTO_FILE; // default
     lh->f = stdout;                // default
     lh->level = LOG_INFO;          // By default
     lh->messages = messages;
     
     return lh;
 }

 void cobaro_log_fini(cobaro_loghandle_t lh)
 {
     if (lh) {
         free(lh->blocks);
         pthread_spin_destroy(&lh->lock);
         free(lh);
         lh = NULL;
     }
     return;
 }

 void cobaro_log_messages_set(cobaro_loghandle_t lh, char **messages)
 {
     lh->messages = messages;
     return;
 }

 cobaro_log_t cobaro_log_claim(cobaro_loghandle_t lh)
 {
     int ret;
     cobaro_log_t log;

     if ((ret = pthread_spin_lock(&lh->lock))) {
         fprintf(stderr, "spin_lock failed %d\n", ret);
     }

     // We can always take from the free list
     log = lh->free;
     if (log) {
         lh->free = log->next;
     }

     if ((ret = pthread_spin_unlock(&lh->lock))) {
         fprintf(stderr, "spin_unlock failed %d\n", ret);
     }

     return log;
 }

void cobaro_log_set_string(cobaro_log_t log, int argnum, const char *source)
{
    // 'argnum' is the %n number, which is 1-based; index is for the
    // array, which is 0-based, so convert here once, and use index
    // hereafter.
    int index = argnum - 1;
    log->p[index].type = COBARO_STRING;
    strncpy(log->p[index].s, source, sizeof(log->p[index].s));
    log->p[index].s[sizeof(log->p[index].s) - 1] = '\0';
}

void cobaro_log_set_integer(cobaro_log_t log, int argnum, int64_t source)
{
    int index = argnum - 1;
    log->p[index].type = COBARO_INTEGER;
    log->p[index].i = source;
}

void cobaro_log_set_double(cobaro_log_t log, int argnum, double source)
{
    int index = argnum - 1;
    log->p[index].type = COBARO_REAL;
    log->p[index].f = source;
}

void cobaro_log_set_ipv4(cobaro_log_t log, int argnum, uint32_t ipv4)
{
    int index = argnum - 1;
    log->p[index].type = COBARO_IPV4;
    log->p[index].ipv4 = ipv4;
}

 void cobaro_log_publish(cobaro_loghandle_t lh, cobaro_log_t log)
 {
     int ret;

     log->next = NULL;

     if ((ret = pthread_spin_lock(&lh->lock))) {
         fprintf(stderr, "spin_lock failed %d\n", ret);
     }

     if (!lh->busy) {
         lh->busy = log;
     } else {
         cobaro_log_t tail = lh->busy;
         while (tail && tail->next) {
             tail = tail->next;
         }
         tail->next = log;
     }
     if ((ret = pthread_spin_unlock(&lh->lock))) {
         fprintf(stderr, "spin_unlock failed %d\n", ret);
     }

     return;
 }

 cobaro_log_t cobaro_log_next(cobaro_loghandle_t lh)
 {
     int ret;
     cobaro_log_t log = NULL;

     if ((ret = pthread_spin_lock(&lh->lock))) {
         fprintf(stderr, "spin_lock failed %d\n", ret);
     }

     if (lh->busy) {
         log = lh->busy;
         lh->busy = lh->busy->next;
     }

     if ((ret = pthread_spin_unlock(&lh->lock))) {
         fprintf(stderr, "spin_unlock failed %d\n", ret);
     }

     return log;
 }

 void cobaro_log_return(cobaro_loghandle_t lh, cobaro_log_t log)
 {
     int ret;

     if ((ret = pthread_spin_lock(&lh->lock))) {
         fprintf(stderr, "spin_lock failed %d\n", ret);
     }

     log->next = lh->free;
     lh->free = log;

     if ((ret = pthread_spin_unlock(&lh->lock))) {
         fprintf(stderr, "spin_unlock failed %d\n", ret);
     }

 }

 bool cobaro_log_loglevel_set(cobaro_loghandle_t lh, int level)
 {
     if (level < LOG_EMERG || level > LOG_DEBUG) {
         return false;
     }

     lh->level = level;
     return true;
 }

int cobaro_log_to_file(cobaro_loghandle_t lh, cobaro_log_t log, FILE *f)
 {
     char s[1024];
     size_t formatted = 0;
     struct timeval now = {0};
     static const char *time_failure = "--:--:--.------";

     errno = 0;

     // loglevel test
     if (log->level > lh->level) {
         return true;
     }

    // Start trace output with time (hh:mm:ss.mmmuuu).
    gettimeofday(&now, NULL);
    formatted = strftime(s, sizeof(s), "%T", localtime(&now.tv_sec));
    if (!formatted) {
        formatted = snprintf(s, strlen(time_failure) + 1, "%s", time_failure);
    }
    formatted += snprintf(&s[formatted], sizeof(s) - formatted,
                          ".%06ld ", now.tv_usec);
    formatted += cobaro_log_to_string(lh, log, &s[formatted], sizeof(s) - formatted);

    if (formatted <= 1024) {
        formatted = fprintf(f, "%s\n", s);
        if (formatted) {
            return formatted;
        }
    } else {
        errno = ENOSPC;
    }
    return -errno;
 }

bool cobaro_log_file_set(cobaro_loghandle_t lh, FILE *f)
 {
     lh->f = f;
     lh->logto = COBARO_LOGTO_FILE;

     return true;
 }    

 bool cobaro_log_syslog_set(cobaro_loghandle_t lh)
 {
     lh->logto = COBARO_LOGTO_SYSLOG;

     return true;
 }    

bool cobaro_log(cobaro_loghandle_t lh, cobaro_log_t log)
{
    switch (lh->logto) {
    case COBARO_LOGTO_SYSLOG:
        cobaro_log_to_syslog(lh, log);
        return true;
    case COBARO_LOGTO_FILE:
        return (cobaro_log_to_file(lh, log, lh->f) > 0);
    }

    return false;
}

void cobaro_log_to_syslog(cobaro_loghandle_t lh, cobaro_log_t log)
 {
     char s[1024];

     // loglevel test
     if (log->level <= lh->level) {
         (void) cobaro_log_to_string(lh, log, s, sizeof(s));
         syslog(log->level, "%s", s);
     }
     return;
 }

int cobaro_log_to_string(cobaro_loghandle_t lh, cobaro_log_t log,
                          char *s, size_t s_len)
{
    size_t written = 0; // How many _could_ be written
    char *format_i18n;
    int arg;
    char addr[INET_ADDRSTRLEN];

    if (!(format_i18n = lh->messages[log->code])) {
        return false;
    }

    while (*format_i18n) {
        if (*format_i18n == '%') {

            format_i18n++; // move along

            // We have 8 arguments max: PARAM_MAX_FIX
            if (*format_i18n > '0' && *format_i18n < '9') {
                arg = *format_i18n - '1'; // args count from 1
                format_i18n++; // move along

                switch (log->p[arg].type) {
                case COBARO_STRING:
                    written += snprintf(&s[written], MAX(s_len - written, 0),
                                        "%s", log->p[arg].s);
                    break;
                case COBARO_INTEGER:
                    written += snprintf(&s[written], MAX(s_len - written, 0),
                                        "%ld", log->p[arg].i);
                    break;
                case COBARO_REAL:
                    written += snprintf(&s[written], MAX(s_len - written, 0),
                                        "%g", log->p[arg].f);
                    break;
                case COBARO_IPV4:
                    inet_ntop(AF_INET, &log->p[arg].ipv4,
                              addr, sizeof(addr));

                    written += snprintf(&s[written], MAX(s_len - written, 0),
                                        "%s", addr); 
                    break;
                }
            } else {
                // %% prints a percentage sign
                if (*format_i18n == '%') {
                    // Only actually write if we have enough space
                    if (written < s_len) {
                        s[written] = '%';
                    }
                    written++;
                    format_i18n++; // move along
                }
            }
        } else {
            // Only actually write if we have enough space
            if (written < s_len) {
                s[written] = *format_i18n;
            }
            written++;
            format_i18n++; // move along
        }
    }

    // Always NULL terminate the output.
    if (written < s_len) {
        s[written] = '\0';
    } else{
        s[s_len - 1] = '\0';
    }
    written++;
    
    return written;
}

