#!/bin/sh
# Local Station Lite service. It is not installed unless explicitly enabled.
set -eu
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
pidfile=${DOT_WAKE_PIDFILE:-/tmp/dot-wake-service.pid}
meminfo=${DOT_WAKE_MEMINFO:-/proc/meminfo}
owned() {
  [ -r "$pidfile" ] || return 1
  pid=$(cat "$pidfile")
  case "$pid" in ''|*[!0-9]*) return 1 ;; esac
  [ -r "/proc/$pid/cmdline" ] || return 1
  tr '\000' ' ' < "/proc/$pid/cmdline" | grep -Fq "$base/service.sh"
}
available() {
  [ -r "$meminfo" ] || { echo 0; return; }
  awk '/^MemAvailable:/ {print $2; found=1} END {if (!found) print 0}' "$meminfo"
}
case "${1:-}" in
  start)
    if owned; then exit 0; fi
    umask 077
    nohup "$base/service.sh" watch > /tmp/dot-wake-service.log 2>&1 </dev/null &
    printf '%s\n' "$!" > "$pidfile"
    ;;
  stop)
    if owned; then kill "$pid"; fi
    ;;
  status)
    if owned && kill -0 "$pid" 2>/dev/null; then echo running; else echo stopped; exit 1; fi
    ;;
  watch)
    DOT_WAKE_CLIENT=${DOT_WAKE_CLIENT:-/data/opt/alisa-standalone/bin/alisa-standalone}
    DOT_WAKE_STATE=${DOT_WAKE_STATE:-/data/opt/alisa-standalone/state}
    DOT_WAKE_SOCKET=${DOT_WAKE_SOCKET:-/tmp/alisa-standalone/microphone.sock}
    export DOT_WAKE_CLIENT DOT_WAKE_STATE DOT_WAKE_SOCKET
    child=
    cleanup() {
      if [ -n "$child" ]; then kill "$child" 2>/dev/null || true; wait "$child" 2>/dev/null || true; fi
    }
    trap cleanup EXIT
    trap 'exit 1' HUP INT TERM
    while :; do
      while [ "$(available)" -lt 60000 ]; do sleep 1; done
      "$base/run.sh" &
      child=$!
      low=0
      while kill -0 "$child" 2>/dev/null; do
        if [ "$(available)" -lt 40000 ]; then
          low=1
          kill "$child" 2>/dev/null || true
          break
        fi
        sleep 1
      done
      if wait "$child"; then code=0; else code=$?; fi
      child=
      [ "$low" -eq 1 ] || exit "$code"
    done
    ;;
  *) exit 2 ;;
esac
