liblog
========
.. image:: http://jenkins.0x1.org:8080/buildStatus/icon?job=cobaro-liblog0

liblog provides a simple threaded C logging API licensed under the `MIT
license <LICENSE.txt>`_.

Log messages may be simply constructed, optionally enqueued
and read by a different thread, and logged to console/file or to
syslog. New logging backends could be simply added.

It is designed to minimize the CPU cost of logging in the current
thread to optimize performance.

Currently supported platforms include Linux and OSX.

See the `Developers Guide <doc/DeveloperGuide.rst>`_ for information
on how compile and use liblog.

Contributing
------------
Contributions are welcome via `github
<https://github.com/cobaro/liblog>`_.  Please submit proposed changes as
a pull request, or attach a patch to a GitHub issue.




