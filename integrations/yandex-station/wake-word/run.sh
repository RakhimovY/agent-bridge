#!/bin/sh
# Experimental wake listener. Not enabled by the normal installation.
set -eu
: "${DOT_WAKE_BINARY:?Set the wake detector executable}"
: "${DOT_WAKE_SOCKET:?Set the microphone socket path}"
: "${DOT_WAKE_MUTE_FILE:?Set the microphone state file}"
: "${DOT_WAKE_CONTROL:?Set the voice client executable}"
: "${DOT_WAKE_THRESHOLD:?Set the calibrated match threshold}"
[ "$#" -gt 0 ] || { echo 'Provide one or more private reference files' >&2; exit 1; }
first=$1
shift
child=
stop() {
  if [ -n "$child" ]; then
    kill "$child" 2>/dev/null || true
    wait "$child" 2>/dev/null || true
  fi
  exit 0
}
trap stop INT TERM
while :; do
  if "$DOT_WAKE_CONTROL" status 2>/dev/null | grep -Eq '"phase"[[:space:]]*:[[:space:]]*"idle"'; then
    "$DOT_WAKE_BINARY" listen "$DOT_WAKE_SOCKET" "$first" "$DOT_WAKE_THRESHOLD" "$DOT_WAKE_CONTROL" "$@" &
    child=$!
    wait "$child" || true
    child=
  fi
  sleep 1
done
