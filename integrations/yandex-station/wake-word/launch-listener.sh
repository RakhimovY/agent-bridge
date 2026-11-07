#!/bin/sh
set -eu
: "${DOT_WAKE_RUNTIME:?Set the isolated runtime directory}"
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ulimit -c 0
ulimit -s 1024
ulimit -v 150000
ulimit -d 65000
exec "$DOT_WAKE_RUNTIME/ld-linux-armhf.so.3" --library-path "$DOT_WAKE_RUNTIME:/lib:/usr/lib" "$base/listener" "$@"
