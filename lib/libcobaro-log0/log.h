// -*- mode: c -*-
#ifndef COBARO_LOG0_LOG_H
#define COBARO_LOG0_LOG_H

/****************************************************************
COPYRIGHT_BEGIN
Copyright (C) 2015, cobaro.org
All rights reserved.
COPYRIGHT_END
****************************************************************/

// All available in C99
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/// @file
/// Light-weight inter-thread log messages.

/// Type discriminator for parameter values in logs.
enum cobaro_log_types {
    COBARO_STRING = 1,  ///< Value is a string.
    COBARO_INTEGER = 2,  ///< Value is a signed integer.
    COBARO_REAL = 3,  ///< Value is a double.
    COBARO_IPV4 = 4  ///< Value is an IP address (in 32 bit network format).
};


/// Log levels as they come from syslog(3).
enum cobaro_log_levels {
    COBARO_LOG_EMERG = 0,  ///< System is unusable.
    COBARO_LOG_ALERT,  ///< Action must be taken immediately.
    COBARO_LOG_CRIT,  ///< Critical conditions.
    COBARO_LOG_ERR,  ///< Error conditions.
    COBARO_LOG_WARNING,  ///< Warning conditions.
    COBARO_LOG_NOTICE,  ///< Normal but significant condition.
    COBARO_LOG_INFO,  ///< Informational.
    COBARO_LOG_DEBUG,  ///< Debug-level messages.
    COBARO_LOG_LEVELS_COUNT  ///< Iterator useful.
};


/// Number of parameters in a log message structure.
#define COBARO_LOG_PARAM_MAX (8) // Search log.c for PARAM_MAX_FIX if changed


/// Log information structure.
///
/// Can carry a log code, plus @ref COBARO_LOG_PARAM_MAX parameters of
/// variant types.
struct cobaro_log {

    /// Opaque header for use in identifying this object as a log
    /// message, when multiple structure types are eg. delivered via
    /// the same queue.
    uint32_t id;

    /// Log code.  Zero is successful.
    uint32_t code;

    /// Queue.
    struct cobaro_log *next;

    /// Log level, from @ref cobaro_log_levels enumeration.
    uint8_t level;

    // 3 spare bytes here.

    /// Pad to make us fit into 512 bytes exactly.
    char pad[44];

    /// Array of parameters relevant to this log.
    struct {
        /// Value type discriminant.  Legal values are defined in the
        /// @ref cobaro_log_types enumeration.
        uint8_t type;

        // 7 spare bytes here.

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
        } v;
    } p[COBARO_LOG_PARAM_MAX];

};

// log->code       is the numeric identifier for a specific log.
// log->p[0].type  is the type of the value.
// log->p[0].v.s   is a reference to a string value.


/// Log structure pointer type.
typedef struct cobaro_log *cobaro_log_t;

/// Opaque log handle type.
typedef struct cobaro_loghandle *cobaro_loghandle_t;



/// Printable version number.
char *cobaro_log_version(void);

/// Initialize  the logging infrastructure.
///
/// Must be called once before logging.  By default logging is to
/// stdout at level LOG_INFO.
///
/// @param[in] messages
///    Array of message format strings. See test/messages.[ch] for
///    examples.
///
/// @returns
///    Valid log handle on success, @c NULL on failure.
cobaro_loghandle_t cobaro_log_init(char **messages);

/// Set the message catalog in use (in case you want to change language).
///
/// @param[in] lh
///    Log handle to set messages catalog for.
///
/// @param[in] messages
///    Array of message format strings. See test/messages.[ch] for
///    examples.
///
/// @returns
///    Valid log handle on success, @c NULL on failure.
void cobaro_log_messages_set(cobaro_loghandle_t lh, char **messages);

/// Finalize the logging infrastructure.
///
/// @param[in] lh
///    Log handle to clean up.
void cobaro_log_fini(cobaro_loghandle_t lh);

/// Acquire a log structure from the handle's free list.
///
/// The log handle maintains set of log structures that can be passed
/// between threads to forward a log message for reporting.  This
/// function acquires a log structure from this free list.  On return,
/// you have control over its memory.
///
/// @param[in] lh
///    Log handle to fetch from.
///
/// @returns
///    Pointer to log structure on success.  @c NULL if no log
///    structures are available from the handle.  This means that the
///    logging system is congested, and the caller should simply
///    continue its work without logging.
cobaro_log_t cobaro_log_claim(cobaro_loghandle_t lh);

/// Helper function for setting a string parameter.
///
/// Ensures that the string is copied and terminated properly in a
/// single line of code, rather than three.
///
/// @param[in] log
///    Log structure to populate.
///
/// @param[in] argnum
///    Argument number (as in, %n, being array index + 1).
///
/// @param[in] source
///    C-string to copy to log structure's parameters.
void cobaro_log_set_string(cobaro_log_t log, int argnum, const char *source);

/// Helper function for setting an integer parameter.
///
/// @param[in] log
///    Log structure to populate.
///
/// @param[in] argnum
///    Argument number (as in, %n, being array index + 1).
///
/// @param[in] source
///    Integer value to set.
void cobaro_log_set_integer(cobaro_log_t log, int argnum, int64_t source);

/// Helper function for setting an double parameter.
///
/// @param[in] log
///    Log structure to populate.
///
/// @param[in] argnum
///    Argument number (as in, %n, being array index + 1).
///
/// @param[in] source
///    Double/real value to set.
void cobaro_log_set_double(cobaro_log_t log, int argnum, double source);

/// Helper function for setting an IPv4 parameter.
///
/// @param[in] log
///    Log structure to populate.
///
/// @param[in] argnum
///    Argument number (as in, %n, being array index + 1).
///
/// @param[in] source
///    IPv4 address, in **host** byte order.
void cobaro_log_set_ipv4(cobaro_log_t log, int argnum, uint32_t source);

/// Publish a log_t relinqushing its memory.
///
/// @param[in] lh
///    Log handle to publish to.
///
/// @param[in] log
///    Pointer to log object. This relinquishes control of the memory.
void cobaro_log_publish(cobaro_loghandle_t lh, cobaro_log_t log);

/// Receive a cobaro_log_t if available.
///
/// @param[in] lh
///    Log handle to receive a log from.
///
/// @returns
///    @c NULL if nothing waiting, otherwise pointer to cobaro_log
///    structure to be processed.
cobaro_log_t cobaro_log_next(cobaro_loghandle_t lh);

/// Return a log structure to the free list.
///
/// @param[in] lh
///    Log handle to receive a log from.
///
/// @param[in] log
///    Pointer to log structure to be returned to free list.
void cobaro_log_return(cobaro_loghandle_t lh, cobaro_log_t log);

/// Emit a log to the loghandle's default destination.
///
/// Only the first 1024 bytes of a log message will be written. If
/// that is insufficient then use cobaro_log_to_string() and log
/// directly.
///
/// @param[in] lh
///    Log handle to log with.
///
/// @param[in] log
///    Pointer to log structure.
///
/// @returns
///    @c true on success, @c false on any failure.
bool cobaro_log(cobaro_loghandle_t lh, cobaro_log_t log);

/// Log a message to syslog.
///
/// This call assumes your application has established and configured
/// a syslog session using openlog(3).
///
/// Only the first 1024 bytes of a log message will be written. If
/// that is insufficient then use cobaro_log_to_string() and a
/// suitable buffer and call syslog(3) directly.
///
/// @param[in] lh
///    Log handle to receive a log from.
///
/// @param[in] log
///    Log data.
void cobaro_log_to_syslog(cobaro_loghandle_t lh, cobaro_log_t log);

/// Log a message to file.
///
/// Write the formatted log message, prepended with a timestamp, to
/// the specified file.  The timestamp uses the local timezone, and
/// microsecond precision.
///
/// Only the first 1024 bytes of a log message will be written. If
/// that is insufficient then use cobaro_log_to_string() and a
/// suitable buffer and call syslog(3) directly.
///
/// @param[in] lh
///    Log handle in use.
///
/// @param[in] log
///    Log to write.
///
/// @param[in] f
///    File handle to write to, eg. stdout, stderr, or any other FILE
///    pointer.
///
/// @returns
///    If successful, returns the number of characters written to the
///    file.  If nothing was written (because the configured log level
///    is too high), it returns zero.
///
///    If there was an error, -1 is returned and an error code is
///    available from @c errno.  @c ENOSPC indicates that the
///    formatted message (including timestamp) exceeded 1024 bytes.
///    See fprintf(3) and write(2) for other possible @c errno values.
int cobaro_log_to_file(cobaro_loghandle_t lh, cobaro_log_t log, FILE *f);

/// Log a message to a string.
/// 
/// The function will not write more than @p buflen bytes and will
/// always NUL-terminate the string even if that means truncating the
/// formatted message.
///
/// @param[in] lh
///    Log handle in use.
///
/// @param[in] log
///    Log data.
///
/// @param[in,out] buffer
///    Write the formatted log message to this character buffer.
///
/// @param[in] buflen
///    Length of @p buffer in bytes.
///
/// @returns
///    Number of characters that would have been written including
///    the terminating NUL had space been available.
///
///    If this value is more than 1024, @p buffer contains a
///    truncated, but safely terminated, message.
int cobaro_log_to_string(cobaro_loghandle_t lh, cobaro_log_t log,
			 char *buffer, size_t buflen);

/// Set the log level below which we should ignore logs.
///
/// @param[in] lh
///     Log handle in use.
///
/// @param[in] level
///    Level to log below, see syslog(3) for defined values. Default
///    is LOG_INFO.
///
/// @returns
///    @c true on success, @c false on failure.
bool cobaro_log_loglevel_set(cobaro_loghandle_t lh, int level);

/// Set the default log destination to be a file handle.
///
/// @param[in] lh
///     Log handle in use.
///
/// @param[in] f
///     File handle to log to.
///
/// @returns
///    @c true on success, @c false on failure.
bool cobaro_log_file_set(cobaro_loghandle_t lh, FILE *f);

/// Set the default log destination to be syslog.
///
/// Caller is responsible for calling openlog(ident, option, facility)
/// and closelog(), see syslog(3).
///
/// @returns
///    @c true on success, @c false on failure.
bool cobaro_log_syslog_set(cobaro_loghandle_t lh);



#endif /* COBARO_LOG0_LOG_H */
