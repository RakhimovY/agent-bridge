#!/bin/sh
# Experimental: do not enable until the full call path has been verified.
set -eu
: "${DOT_WAKE_CLIENT:?Set the client executable}"
: "${DOT_WAKE_STATE:?Set the client state directory}"
: "${DOT_WAKE_LISTENER:?Set the listener executable}"
: "${DOT_WAKE_SOCKET:?Set the microphone socket path}"
[ "$#" -gt 0 ] || { echo 'Provide at least one model group' >&2; exit 1; }
event=$(mktemp "${TMPDIR:-/tmp}/dot-wake-event.XXXXXX")
child=
stop() {
  if [ -n "$child" ]; then
    kill "$child" 2>/dev/null || true
    wait "$child" 2>/dev/null || true
  fi
  rm -f "$event"
  exit 0
}
trap stop INT TERM
trap 'rm -f "$event"' EXIT
idle() {
  status=$("$DOT_WAKE_CLIENT" status "$DOT_WAKE_STATE" 2>/dev/null) || return 1
  printf '%s\n' "$status" | grep -Eq '"phase"[[:space:]]*:[[:space:]]*"idle"' &&
    printf '%s\n' "$status" | grep -Eq '"agent_connected"[[:space:]]*:[[:space:]]*true'
}
memory_ok() {
  meminfo=${DOT_WAKE_MEMINFO:-/proc/meminfo}
  if [ ! -r "$meminfo" ]; then
    [ "$meminfo" = /proc/meminfo ] && [ "$(uname -s)" != Linux ] && return 0
    return 1
  fi
  available=$(awk '/^MemAvailable:/ {print $2}' "$meminfo")
  [ -n "$available" ] && [ "$available" -ge 45000 ]
}
prewarm=0
while :; do
  if [ "$prewarm" -eq 1 ] && memory_ok; then
    DOT_WAKE_PREWARM=1 "$DOT_WAKE_LISTENER" "$DOT_WAKE_SOCKET" "$@" > "$event" &
    child=$!
    while kill -0 "$child" 2>/dev/null; do
      if ! memory_ok; then
        kill "$child" 2>/dev/null || true
        wait "$child" 2>/dev/null || true
        child=
        prewarm=0
        break
      fi
      if grep -q '^WARM$' "$event" && idle; then
        kill -USR1 "$child" 2>/dev/null || true
        break
      fi
      sleep 0.2
    done
    if [ -z "$child" ]; then sleep 1; continue; fi
  elif idle && memory_ok; then
    prewarm=0
    "$DOT_WAKE_LISTENER" "$DOT_WAKE_SOCKET" "$@" > "$event" &
    child=$!
  else
    sleep 1
    continue
  fi
    if wait "$child"; then code=0; else code=$?; fi
    child=
    if [ "$code" -eq 0 ]; then
      key=$(sed -n 's/^MATCH \(K[0-9][0-9]*\)$/\1/p' "$event")
      [ -n "$key" ] || exit 1
      target=default
      if [ -n "${DOT_WAKE_ACTIONS:-}" ]; then
        [ -r "$DOT_WAKE_ACTIONS" ] || exit 1
        target=$(awk -v key="$key" '$1 == key && NF == 2 { n++; value=$2 } END { if (n == 1) print value }' "$DOT_WAKE_ACTIONS")
      fi
      case "$target" in
        default) action=open ;;
        dot) action=open-dot ;;
        chat) action=open-chat ;;
        *) exit 1 ;;
      esac
      if idle && "$DOT_WAKE_CLIENT" "$action" "$DOT_WAKE_STATE" >/dev/null 2>&1; then
        prewarm=1
        continue
      else
        prewarm=0
      fi
    elif [ "$code" -ne 1 ]; then
      exit "$code"
    else
      prewarm=0
    fi
  sleep 1
done
