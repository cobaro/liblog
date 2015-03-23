// -*- mode: c -*-
/****************************************************************
COPYRIGHT_BEGIN
Copyright (C) 2015, cobaro.org
All rights reserved.
COPYRIGHT_END
****************************************************************/

#include "config.h"
#include "cobaro-log0/log.h"

#define COBARO_LOG_SLOTS (16) // Keep it small as we have limited cache

typedef struct cobaro_loghandle {
    cobaro_log_t free;
    cobaro_log_t busy;
    pthread_spinlock_t lock;
    cobaro_log_t blocks; // memory
    bool syslog_opened;  // Have we opened a syslog handle
    int level;           // messages higher than this are not logged
    char **messages;    // Array of format strings
 } *cobaro_loghandle_t;

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
     lh->syslog_opened = false;
     lh->level = LOG_NOTICE; // By default
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
     lh->messages = lh->messages;
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

 bool cobaro_log_to_file(cobaro_loghandle_t lh, cobaro_log_t log, FILE *f)
 {
     char s[1024];

     // loglevel test
     if (log->level > lh->level) {
         return true;
     }

     if (!cobaro_log_to_string(lh, log, s, sizeof(s))) {
         return false;
     }

     fprintf(f, "%s\n", s);

     return true;
 }

 bool cobaro_log_syslog_init(cobaro_loghandle_t lh, char *ident,
                          int option, int facility)
 {
     openlog(ident, option, facility);
     lh->syslog_opened = true;
     return true;
 }    

 void cobaro_log_syslog_fini(cobaro_loghandle_t lh)
 {
     lh->syslog_opened = false;
     closelog();
 }

 bool cobaro_log_to_syslog(cobaro_loghandle_t lh, cobaro_log_t log)
 {
     char s[1024];

     // loglevel test
     if (log->level > lh->level) {
         return true;
     }

     if (!lh->syslog_opened) {
         return false;
     }

     if (!cobaro_log_to_string(lh, log, s, sizeof(s))) {
         return false;
     }

     syslog(log->level, "%s", s);

     return true;
 }

 bool cobaro_log_to_string(cobaro_loghandle_t lh, cobaro_log_t log,
                           char *s, size_t s_len)
 {
     size_t written = 0;
     char *format_i18n;
     int arg;
     char addr[INET_ADDRSTRLEN];

     if (!(format_i18n = lh->messages[log->code])) {
        return false;
    }

    while (*format_i18n && (written < s_len)) {
        if (*format_i18n == '%') {

            format_i18n++; // move along

            // We have 8 arguments max: PARAM_MAX_FIX
            if (*format_i18n > '0' && *format_i18n < '9') {
                arg = *format_i18n - '1'; // args count from 1
                format_i18n++; // move along

                switch (log->p[arg].type) {
                case COBARO_STRING:
                    written += snprintf(&s[written], s_len - written,
                                        "%s", log->p[arg].s);
                    break;
                case COBARO_INTEGER:
                    written += snprintf(&s[written], s_len - written,
                                        "%ld", log->p[arg].i);
                    break;
                case COBARO_REAL:
                    written += snprintf(&s[written], s_len - written,
                                        "%g", log->p[arg].f);
                    break;
                case COBARO_IPV4:
                    inet_ntop(AF_INET, &log->p[arg].ipv4,
                              addr, sizeof(addr));

                    written += snprintf(&s[written], s_len - written,
                                        "%s", addr); 
                    break;
                }
                // Did we run out of space
                if (written > s_len) {
                    return false;
                }
            } else {
                // %% prints a percentage sign
                if (*format_i18n == '%') {
                    s[written++] = '%';
                    format_i18n++; // move along
                    // Write the trailing null if possible to mirror
                    // the snprintf behaviour above
                    if (written < s_len) {
                        s[written++] = '\0';
                    } else {
                        return false;
                    }
                }
            }
        } else {
            s[written] = *format_i18n++;
            written++;
        }
    }
    
    return true;
}

