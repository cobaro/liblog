// -*- mode: c -*-
#ifndef COBARO_LOG0_LOG_H
#define COBARO_LOG0_LOG_H

/****************************************************************
COPYRIGHT_BEGIN
Copyright (C) 2015, cobaro.org
All rights reserved.
COPYRIGHT_END
****************************************************************/

// All available in c99
# include <stdarg.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdint.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>


/// Type discriminator for parameter values in logs.
enum cobaro_log_types {
    /// Value is a string.
    COBARO_STRING = 1,

    /// Value is a signed integer.
    COBARO_INTEGER = 2,

    /// Value is a double.
    COBARO_REAL = 3,

    /// Value is an IP address (in 32 bit network format)
    COBARO_IPV4 = 4
};

/// Loglevels as they come from syslog
enum cobaro_log_levels {
    /* system is unusable */
    COBARO_LOG_EMERG = 0,

    /* action must be taken immediately */
    COBARO_LOG_ALERT,

    /* critical conditions */
    COBARO_LOG_CRIT,

    /* error conditions */
    COBARO_LOG_ERR,

    /* warning conditions */
    COBARO_LOG_WARNING,

    /* normal but significant condition */
    COBARO_LOG_NOTICE,

    /* informational */
    COBARO_LOG_INFO,

    /* debug-level messages */
    COBARO_LOG_DEBUG,

    /* iterator useful. */
    COBARO_LOG_LEVELS_COUNT
};


#define COBARO_LOG_PARAM_MAX (8) // Search log.c for PARAM_MAX_FIX if changed

/// Log information structure.
///
/// Can carry an log code, plus four parameters of arbitrary types.
typedef struct cobaro_log {
    // Queue
    struct cobaro_log *next;

    /// Log code.  Zero is successful.
    uint32_t code;

    /// Value type discriminant.
    uint8_t level;

    // make us fit into 512 bytes exactly
    char pad[48];

    /// Array of parameters relevant to this log.
    struct {
        /// Value type discriminant.
        uint8_t type;

        /// Value
        union {
            /// String value.
            char s[48];
            
            /// Integer value.
            int64_t i;

            /// Floating point value.
            double f;

            /// IPv4 address (32 bit network format)
            uint32_t ipv4;
        };
    } p[COBARO_LOG_PARAM_MAX];

} *cobaro_log_t;

// log->code       is the numeric identifier for a specific log.
// log->p[0].type  is the type of the value.
// log->p[0].s     is a reference to a string value.


/// Log handle type.
typedef struct cobaro_loghandle *cobaro_loghandle_t;


/// Initialize the logging infrastructure
///
/// Must be called once before logging
/// Bye default logging is to stdout at level LOG_INFO
///
/// @param[in] messages
///     array of message format strings. See test/messages.[ch] for examples
///
/// @returns
///     Valid log handle on success, NULL on failure
cobaro_loghandle_t cobaro_log_init(char **messages);

/// Set the message catlog in use (in case you want to change language)
///
/// @param[in] lh
///     loghandle to set emssages catalog for
///
/// @param[in] messages
///     array of message format strings. See test/messages.[ch] for examples
///
/// @returns
///     Valid log handle on success, NULL on failure
void cobaro_log_messages_set(cobaro_loghandle_t lh, char **messages);

/// Finalize the logging infrastructure
///
/// @param[in] lh
///     loghandle to clean up
///
/// Cleans up logging before program exit
void cobaro_log_fini(cobaro_loghandle_t lh);

/// Claim a log_t. At this point you have control over it's memory.
///
/// @param[in] lh
///     loghandle to fetch from
///
/// @returns
///    Pointer to log object on success
///    NOTE: This may return NULL in which case you are unable to log
///          temporarily. This is to ensure we never block and is by design.
cobaro_log_t cobaro_log_claim(cobaro_loghandle_t lh);

/// Helper function for setting a string parameter.
///
/// Ensures that the string is copied and terminated properly in a
/// single line of code, rather than three.
///
/// @param[in] log
///    Log structure to populate.
/// @param[in] argnum
///    Argument number (as in, %n, being array index + 1).
/// @param[in] source
///    C-string to copy to log structure's parameters.
void cobaro_log_set_string(cobaro_log_t log, int argnum, const char *source);

/// Publish a log_t relinqushing it's memory.
///
/// @param[in] lh
///     loghandle to publish to
///
/// @param[in] log
///    Pointer to log object. This relinquishes control of the memory.
void cobaro_log_publish(cobaro_loghandle_t lh, cobaro_log_t log);

/// Receive a log_t if available.
///
/// @param[in] lh
///     loghandle to receive a log from
///
/// @returns
///     valid log_t or NULL if nothing waiting
cobaro_log_t cobaro_log_next(cobaro_loghandle_t lh);

/// Return a log_t to the free list
///
/// @param[in] lh
///     loghandle to receive a log from
///
/// @param[in] log
///    Pointer to log to be returned.
void cobaro_log_return(cobaro_loghandle_t lh, cobaro_log_t log);

/// Emit a log to the loghandle's destination
///
/// @param[in] lh
///     loghandle to log with
///
/// @param[in] log
///    Pointer to log object
bool cobaro_log(cobaro_loghandle_t lh, cobaro_log_t log);

/// Set the log level below which we should ignore logs
///
/// @param[in] lh
///     loghandle in use
///
/// @param[in] level
///    level to log below, see man (3) syslog. Default: LOG_INFO
///
/// @returns
///    true on success, false on failure
bool cobaro_log_loglevel_set(cobaro_loghandle_t lh, int level);

/// Log a message to file
///
/// @param[in] lh
///     loghandle in use
///
/// @param[in] log
///    log to print
///
/// @param[in] f
///    filehandle to print to
///
/// @returns
///    true on success, false on failure
bool cobaro_log_to_file(cobaro_loghandle_t lh, cobaro_log_t log, FILE *f);

/// Set the log destination to be a file handle
///
/// @param[in] lh
///     loghandle in use
///
/// @param[in] f
///     filehandle to log to
///
/// @returns
///    true on success, false on failure
bool cobaro_log_file_set(cobaro_loghandle_t, FILE *f);

/// Set the log 
/// syslog handle remains open until we switch to a file, call
/// syslog_set again, ot close the loghandle via fini.
///
/// @param[in] lh
///     loghandle in use
///
/// @param[in] ident
///     ident string prepended to each message, see man (3) syslog
///
/// @param[in] option
///     option flags, see man (3) syslog
///
/// @returns
///    true on success, false on failure
bool cobaro_log_syslog_set(cobaro_loghandle_t, char *ident,
                           int option, int facility);

/// Log a message to syslog
///
/// This assumes an open syslogger connection
///
/// @param[in] lh
///     loghandle to receive a log from

/// @param[in] log
///    log data
///
/// @returns
///    true on success, false on failure
bool cobaro_log_to_syslog(cobaro_loghandle_t lh, cobaro_log_t log);

/// Log a message to a string
///
/// @param[in] lh
///     loghandle in use
///
/// @param[in] log
///    log data
///
/// @param[in,out] s
///     pre-allocated string for messages to be formatted in to
///
/// @param[in]
///    length of s
///
/// @returns
///    true on success, false on failure
bool cobaro_log_to_string(cobaro_loghandle_t lh, cobaro_log_t log,
                          char *s, size_t s_len);


#endif /* COBARO_LOG0_LOG_H */
