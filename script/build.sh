#! /bin/bash

# Assist build script debugging
env
set -x

# For convenience we set us up to fail on any error.
# and use a trapfunc to let us know abotu failure
function trapfunc {
    set +e
    echo Build failed
    exit 1
}
trap trapfunc ERR
set -e

export CFLAGS='-Wall -Werror -O2 -g -pipe -std=c99'
./bootstrap
./configure
make
make distcheck
make package
(cd test; make valgrind)

echo "Build passed"
exit 0
