#!/bin/sh
# Experimental: supply an explicitly selected client, state and socket.
set -eu
: "${DOT_WAKE_CLIENT:?Set the client executable}"
: "${DOT_WAKE_STATE:?Set the client state directory}"
: "${DOT_WAKE_SOCKET:?Set the microphone socket path}"
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
umask 077
runtime=$(mktemp -d "${TMPDIR:-/tmp}/dot-connect-wake.XXXXXX")
child=
cleanup() {
  if [ -n "$child" ]; then kill "$child" 2>/dev/null || true; wait "$child" 2>/dev/null || true; fi
  rm -r "$runtime"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
gzip -dc "$base/runtime.tar.gz" | tar -xf - -C "$runtime"
DOT_WAKE_RUNTIME=$runtime
DOT_WAKE_LISTENER=$base/launch-listener.sh
DOT_WAKE_ACTIONS=$base/wake-actions.txt
export DOT_WAKE_RUNTIME DOT_WAKE_LISTENER DOT_WAKE_ACTIONS
set --
if [ -s "$base/wake-keywords-ru.txt" ]; then
  set -- "$@" "$base/wake-keywords-ru.txt" "$base/ru/encoder.int4.onnx" "$base/ru/decoder.onnx" "$base/ru/joiner.int8.onnx" "$base/ru/tokens.txt"
fi
if [ -s "$base/wake-keywords-en.txt" ]; then
  set -- "$@" "$base/wake-keywords-en.txt" "$base/en/encoder-epoch-12-avg-2-chunk-16-left-64.int8.onnx" "$base/en/decoder-epoch-12-avg-2-chunk-16-left-64.onnx" "$base/en/joiner-epoch-12-avg-2-chunk-16-left-64.int8.onnx" "$base/en/tokens.txt"
fi
[ "$#" -gt 0 ] || { echo 'No wake phrases configured' >&2; exit 1; }
"$base/supervisor.sh" "$@" &
child=$!
if wait "$child"; then code=0; else code=$?; fi
child=
exit "$code"
