Process
=======

Release version numbers
-----------------------

Have a think about compatibility wrt the comments in configure.ac
regarding C:R:A. Breaking API compatibility will cause a bounce from
logN to logN+1 and you should be reluctant to do that although we
accept it will happen as genuinely needed. This will cause some
deliberate reorganization so that logN and logN+1 could be
concurrently installed.

Update configure.ac's version numbers and C:R:A accordingly.

Branches, tags, releases
------------------------
We follow a `defined git branching model
<http://nvie.com/posts/a-successful-git-branching-model/>`_ so our
releases are branches named in the style of release-x.y.z

We should develop code in one of a 'develop', 'hotfix-x.y.z', or
'feature-foo' branch and merge accordingly.

Documentation installation
--------------------------
For ease of use it's generous to have the documentation available on
the web (at cobaro.org) even though it's contained in the binary
distribution.
