Developer's Guide
=================

Quickstart
----------

liblog provides a simple threaded C logging API licensed under the MIT
license.

Log messages may be simply constructed, optionally enqueued
and read by a different thread, and logged to console/file or to
syslog. New logging backends could be simply added.

It is designed to minimize the CPU cost of logging in the current
thread to optimize performance.

Currently supported platforms include Linux and OSX.

Code level documentation is provided via doxygen.

Compilation and Linkage
-----------------------
If you can use pkgconfig, it can provide the correct compilation and
linkage flags:

pkgconfig --cflags libcobaro-log0
pkgconfig --libs libcobaro-log0

To specify things manually is also simple:

-I /usr/local/include -pthread
-L /usr/local/lib -lcobaro-log0 -pthread

Static vs Dynamic Linkage
~~~~~~~~~~~~~~~~~~~~~~~~~
When linking the logging library, you can choose to do so by including
the object code within your executable (static linking), or by
referencing an installed shared libary (dynamic linking).  The
autotools toolchain encourages dynamic linking, and makes it quite
hard to statically link a library if a shared version is installed.

So, in the Linux packages, if you install just the developer package,
no shared library is installed.  This will force autotools to use the
static library (libcobaro-log0.a).  If you install the runtime package
on your development machine, autotools will link the dynamic library
(libcobaro-log0.so) by default.

You can try to work around this by passing a path to the .a file to
the linker (without a -l flag), but ... that's in hairy territory and
having mentioned it, we'll leave you on your own with that.

General Philosphy
-----------------
The Cobaro Log package is intended for use in circumstances where a
more traditional logging package (syslog, log4c, etc) is too
CPU-time consuming.  Specifically, it's intended to perform no I/O and no
string-formatting: just to collect the relevant data, and hand it over
to something else to deal with.

The library is split into two groups of functions:
- Those that create log message.
- Those that process log messages.

Log messages are represented by a structure containing the log message
identifier (an integer), the log priority (from syslog, emergency to
debug), and a collection of parameters to describe the event being
reported.

The code that reports an event should acquire a log structure, fill
out the identifier, level, and parameters, and then hand it off to
have the text message generated and reported using a traditional
logging system.

The text message is generated from a template, quite similar to
printf().  Templates are defined in one or more arrays of strings.
Only one array is needed, but you can select between different arrays
if you need to support different languages.

Within the strings in the array, placeholders like "%1" specify a
value from the associated array of parameter values.  When the message
is formatted, the values are substituted into the template to produce
the final message.

The thread that generates the log message doesn't need to know about
the message arrays: it simply specifies a log message _number_ (an
index into the messages array), the log level, and populates the
parameters array.

A service thread can subsequently look up the text template, perform
the substitutions, and pass the generated text message on to general
purpose logging system (like syslog).

Library Initialisation (and Finalisation)
-----------------------------------------
The library provides two distinct types: cobaro_log_t, the log message
structure; and cobaro_loghandle_t, a formatting infrastructure.

Library
~~~~~~~
cobaro_log_init()
cobaro_log_fini()

Message Templates
~~~~~~~~~~~~~~~~~
cobaro_log_messages_set(handle, array)

Creating a Log Message
----------------------
The first step to creating a log message is to acquire one.  It's
normally not feasible to use stack allocation, since the log structure
is handed to another thread for formatting.  You can simply malloc()
one, use your own allocation pool, or use the pool implemented by the
log handle type.

log = cobaro_log_claim(handle);

Claim a log from the handle's collection.  If none are free, you'll
get NULL.

Set the log message code and level:

log->code = MY_APP_LOG_MESSAGE_FOO;
log->level = MY_APP_LOG_LEVEL_FOO;

Set any parameters needed:

cobaro_log_set_string(log, 1, "boom!");
cobaro_log_set_integer(log, 2, 42);
cobaro_log_set_double(log, 3, 3.1416);
cobaro_log_set_ipv4(log, 4, ipaddr);

Note that the index parameter in these functions matches the parameter
number in the template strings: it starts from 1, and is the index
into the parameter array + 1.

Of these functions, set_string() does the most work, copying the
string contents up to the size of the parameter array's strings, and
terminating them correctly.

At this point you have a fully populated log structure, and need to
decide what to do with it.

Publishing a Log Message
------------------------
The Cobaro Log log handle type has an in-built inter-thread queue,
suitable for publishing log messages to a background thread for
formatting and reporting via eg. syslog.

Alternatively, you can use your own inter-thread communications
channels to hand over the log_t pointer to a service thread.

cobaro_log_publish(handle, log)

This function queues the provided log structure for processing by
another thread sharing this handle handle.

The other thread should call

log = cobaro_log_next(handle)

to retrieve log messages from this queue, process them, and then call

cobaro_log_return(handle, log)

to return the structure to the handle's allocation pool (for use by
future calls to cobaro_log_claim()).

Using your own Queue
~~~~~~~~~~~~~~~~~~~~
To use your own communication channel between the source thread and
the reporting thread, you can take advantage of the cobaro_log_t->id
header.  This is a four-byte field at the start of the log_t structure
that has no use in the Cobaro Log system, and is intended to be
populated with header information for an external communications
system if required.

For instance, if you have a queue between multiple threads already in
use for control messages, usage reporting, etc, log messages can also
be passed via this path.  In some cases, the pointer could be used
directly together with the id header to identify this pointer as a log
message, rather than a control message.  In other cases, it'll be
necessary to wrap the log_t pointer in a suitable envelope structure.

log->id = MY_APP_LOG_MESSAGE;
my_queue_append(my_queue, (void *)log)

Note that in this case you also need to ensure that the memory
management is taken care of.  The log handle's free list is small (to
reduce cache pressure), so you need to ensure that cobaro_log_return()
is called as soon as possible if you're using the log handle's
allocation pool.

Reporting a Log Message
-----------------------
The log handle has support for logging to file, to syslog, and a
generic function for formatting the message string for use with any
logging system.

In the most simple configuration, you select the target system

cobaro_log_file_set()
cobaro_log_syslog_set()

And then call

cobaro_log(handle, log)

to actually report a log message.

If you want more flexibility, you can call the underlying functions
directly.

Logging to File
~~~~~~~~~~~~~~~
cobaro_log_to_file(handle, log)

Logging to syslog
~~~~~~~~~~~~~~~~~
cobaro_log_to_syslog(handle, log)

Logging to String
~~~~~~~~~~~~~~~~~
cobaro_log_to_string(handle, log, buffer, buflen)

Defining Log Templates
----------------------
<suggest array structures here>

Licensing
---------
Cobaro Log is licensed using the MIT license.  You are free to include
this code in commercial products, and to modify it as you require::

   MIT License
   -----------
   
   Copyright (C) 2015, Cobaro.org.
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.


Contributions to the project are welcomed.  Please create a GitHub
issue with patch attached, or send a pull request.


References
----------
See also:

Reference Guide (doxygen)
Install Guide
README
github

