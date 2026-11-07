# Optional local stop detector

This experimental add-on recognizes the English word "stop" **locally during an active Dot call** and signals the client to close the call. It uses no laptop or additional online service during recognition. It does not implement a custom wake word. If the detector is absent or fails to start, the call can still use the existing Alice close command; the detector is inactive.

On one Lite 1, two different synthetic "stop" voices were detected; two ordinary synthetic phrases were not. Two automatic call/stop/reopen cycles also succeeded. The user also confirmed that speaking "stop" closed the call and a subsequent call opened. Long-term false-activation rates remain unmeasured. A spoken occurrence of "stop" in a longer conversation may end the call; avoid enabling it if that behavior is unwanted.

The detector is optional and only used for Dot calls when `ALISA_STOP_DIR` is configured. Its private runtime is separate from the system libraries because this speaker's installed TensorFlow Lite cannot load the stop model and the standalone ARM library requires a newer glibc. No system library is replaced.

## Build without a device

Requires macOS or Linux, Python 3, curl, `ar`, tar/xz, and Zig 0.14.1. The script downloads pinned upstream components, verifies SHA-256 checksums, and builds an isolated ARMv7 runtime; it does not connect to a speaker or an account.

```sh
export DOT_ZIG=/PATH/TO/zig
integrations/yandex-station/stop-detector/build.sh
```

The default output is `integrations/yandex-station/stop-detector/.build/stop/`. `DOT_STOP_BUILD_DIR` can select a private build directory outside the repo. The model, TensorFlow Lite library, Debian runtime and frontend are obtained from the credited upstream sources; see [NOTICE](../../../NOTICE.md).

## Install on the tested route

First complete the [native Dot client build and installation](../../../docs/02-build-and-install.md) with the current patch. The client and detector below must both be present. Check that no call is active and take backups of the current binary and `runtime.env`. With `STATION_KEY`, `SSH_PORT`, and `SSH_TARGET` configured as in [the route guide](../../../docs/03-use-and-rollback.md), create a new private directory; do not overwrite an existing one:

```sh
export STOP_BUILD_DIR="$PWD/integrations/yandex-station/stop-detector/.build"
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; test ! -e /data/opt/dot-connect-stop && mkdir -p /data/opt/dot-connect-stop'
tar -C "$STOP_BUILD_DIR/stop" -cf - . | \
  ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; tar -xf - -C /data/opt/dot-connect-stop'
```

Add the following line to the speaker's `/data/opt/alisa-standalone/runtime.env`, retaining its existing content:

```sh
export ALISA_STOP_DIR=/data/opt/dot-connect-stop
```

Restart the client while idle and test a real conversation, bare "stop", and opening the next call. `stop_detector_ready` in the active client's status indicates that the local helper reported readiness, not that every spoken phrase will be recognized. Never publish the full status output or audio. If recognition is unreliable, remove the added variable, restart, and use the original close command; leave backups intact.

## Tests and scope

The repository's Yandex CI checks the patch and generated sounds offline. It does not run this detector against a speaker or any ChatGPT account. The automated device tests above used synthetic audio sent through the test speaker's audio path; the user separately confirmed stop and reopening through the real microphone. Other integrations are unaffected unless they explicitly implement their own stop detection.
