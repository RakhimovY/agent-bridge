# Experimental text-configured wake phrases

On one rooted Yandex Station Lite 1, a local wake phrase opened the configured Dot without saying Alice. The owner also confirmed a voice reply, English `stop`, another call, and a call after reboot. The speaker does the work itself; the setup computer is not needed during calls. Recognition is imperfect and the next wake can take several seconds after a call. The listener may unload under memory pressure and reload when it is safe.

This is an experimental, device-specific path, not a one-click installer. Root access and the existing native Dot client must already work. The listener reads the device's microphone stream locally and does not save speech. It checks the physical mute input and refuses to recognize while muted; the input was checked on one device and mute behavior has automated tests, but a live muted-voice test remains outstanding. The listener does not identify speakers and requires no voice samples.

Phrases are text, encoded using the selected Russian or English model. Encoding a phrase does not guarantee recognition, and unsupported or mixed-script phrases can be rejected. Test your own phrases and false activations. The current released client opens its configured target for every phrase. Separate Dot and ordinary ChatGPT targets are not yet supported by the released client. The separate stop detector recognizes English `stop`, not arbitrary stop words.

## Prerequisites

Complete the [station and native client setup](../README.md), including SSH, a verified working call, the [stop detector](../stop-detector/README.md), and a private backup. This procedure was exercised on Lite 1 with `YNDX-00025`, `yandexmicro`, and firmware `0.369.1.46.4778262442.20261013.297`. Other devices, firmware, and long-term stability are not verified. Make sure there is enough space for the bundle and rollback. Do not replace the system C library or the working client.

The example below builds a Russian bundle on macOS or Linux. It requires Python 3.12, Git, curl, tar, Zig 0.14.1, and the isolated ARM runtime built by the [stop detector build](../stop-detector/README.md). Install dependencies into a private virtual environment:

```sh
python3.12 -m venv /PRIVATE/PATH/dot-wake-venv
/PRIVATE/PATH/dot-wake-venv/bin/pip install sentencepiece==0.2.3 onnx==1.23.2 onnxruntime==1.31.1
```

Download the following upstream archives into a new private working directory. Verify the checksums before extracting. The Russian encoder is converted locally to 4-bit weights; the resulting file is about 16 MB.

```sh
mkdir -p /PRIVATE/PATH/dot-wake-sources
cd /PRIVATE/PATH/dot-wake-sources
curl -fL -o sherpa.tar.bz2 https://github.com/k2-fsa/sherpa-onnx/releases/download/v1.12.21/sherpa-onnx-v1.12.21-linux-arm-gnueabihf-shared.tar.bz2
curl -fL -o ru-fp32.tar.bz2 https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-streaming-zipformer-small-ru-vosk-2025-08-16.tar.bz2
curl -fL -o ru-int8.tar.bz2 https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-streaming-zipformer-small-ru-vosk-int8-2025-08-16.tar.bz2
printf '%s\n' \
  '31c60fadea7ec662da3c85e75da7b1dce4fe184678a4d6ff961090fe4291f468  sherpa.tar.bz2' \
  'cc2e99ed0c67cae8801170e7b7539b4cac00b716076af86f974bf5b888d9370c  ru-fp32.tar.bz2' \
  '6ba68a01ff3c5445aaf2d61e9b97b026f1149dcc9049d11af3f44f55176341d8  ru-int8.tar.bz2' | shasum -a 256 -c -
tar -xjf sherpa.tar.bz2
tar -xjf ru-fp32.tar.bz2
tar -xjf ru-int8.tar.bz2
mkdir -p include/sherpa-onnx/c-api
curl -fL -o include/sherpa-onnx/c-api/c-api.h https://raw.githubusercontent.com/k2-fsa/sherpa-onnx/v1.12.21/sherpa-onnx/c-api/c-api.h
printf '%s\n' '4b547326b6888a9d5c4366d0b7090efcb6c59d7099dba2deaf828ec61e8587b4  include/sherpa-onnx/c-api/c-api.h' | shasum -a 256 -c -
```

The [Russian model](https://huggingface.co/alphacep/vosk-model-small-streaming-ru) and [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) have their own licenses. See also [NOTICE](../../../NOTICE.md). English model preparation can be used through `bundle.py --en` with the [GigaSpeech model](https://github.com/k2-fsa/sherpa-onnx/releases/download/kws-models/sherpa-onnx-kws-zipformer-gigaspeech-3.3M-2024-01-01.tar.bz2); the reproducible builder below covers Russian only.

## Build a private bundle

From the repository root, set paths to the extracted files, Zig, and the isolated runtime produced by the stop detector. Supply a JSON array of phrases on stdin. This example uses a generic phrase; choose and test your own. Avoid putting private phrases in shared shell history or the repository.

```sh
cd /PATH/TO/dot-connect
export DOT_ZIG=/PATH/TO/zig
export DOT_WAKE_PYTHON=/PRIVATE/PATH/dot-wake-venv/bin/python
export DOT_WAKE_RU_INT8=/PRIVATE/PATH/dot-wake-sources/sherpa-onnx-streaming-zipformer-small-ru-vosk-int8-2025-08-16
export DOT_WAKE_RU_FP32=/PRIVATE/PATH/dot-wake-sources/sherpa-onnx-streaming-zipformer-small-ru-vosk-2025-08-16
export DOT_WAKE_LIBS=/PRIVATE/PATH/dot-wake-sources/sherpa-onnx-v1.12.21-linux-arm-gnueabihf-shared/lib
export DOT_WAKE_HEADERS=/PRIVATE/PATH/dot-wake-sources/include
export DOT_WAKE_RUNTIME=/PATH/TO/stop-detector/.build/stop/runtime
export DOT_WAKE_BUILD_DIR=/PRIVATE/PATH/new-dot-wake-build
integrations/yandex-station/wake-word/build.sh <<'PHRASES'
["Привет друг"]
PHRASES
```

The build directory must not exist. Its `bundle/` contains the ARM listener, runtime, model and private keyword files. It contains no account credentials; nevertheless keep the bundle private. Do not add model files, logs, sessions, private phrases, or device identifiers to Git.

## Install, verify, and roll back

With the speaker idle, make a backup of its current client and boot configuration. Configure `STATION_KEY`, `SSH_PORT` and `SSH_TARGET` as in the [client guide](../../../docs/03-use-and-rollback.md). This creates a new directory and refuses to overwrite an existing one:

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; test ! -e /data/opt/dot-wake && mkdir /data/opt/dot-wake'
tar -C "$DOT_WAKE_BUILD_DIR/bundle" -cf - . | \
  ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; tar -xf - -C /data/opt/dot-wake'
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" '/data/opt/dot-wake/service.sh start'
```

The service waits for available memory, launches the listener, and releases it when memory is critically low; it retries when enough memory returns. It does not impose a conversation time limit. Check `service.sh status`, then test your phrase, the Dot reply, bare `stop`, and a later repeat call. A working listener is not proof that your phrase is reliable or that hardware mute works on a different device.

If a transfer is interrupted, inspect the new directory first. Remove it only if you have established that it is the incomplete bundle from this installation, then retry; do not overwrite a working installation. `service.sh status` checks the service process only: it does not prove the model is ready or the microphone is connected. Verify readiness by speaking your phrase.

For autostart, integrate `/data/opt/dot-wake/service.sh start` into **your existing** native-client boot hook after that client starts. Boot hooks differ by installation. The following example applies only if `/data/opt/alisa-cli/boot.sh` has the exact native-client start line shown. It backs up the hook and refuses to overwrite an existing backup:

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" 'sh -s' <<'REMOTE'
set -eu
boot=/data/opt/alisa-cli/boot.sh
test ! -e "$boot.dot-wake-backup"
test "$(grep -c '^        /data/opt/alisa-standalone/run.sh start$' "$boot")" -eq 1
cp -p "$boot" "$boot.dot-wake-backup"
awk '{print} /^        \/data\/opt\/alisa-standalone\/run.sh start$/ {
  print "        if [ -f \"$BASE/dot-wake-autostart\" ] && [ -x /data/opt/dot-wake/service.sh ]; then"
  print "            /data/opt/dot-wake/service.sh start || true"
  print "        fi"
}' "$boot" > "$boot.dot-wake-new"
chmod 700 "$boot.dot-wake-new"
sh -n "$boot.dot-wake-new"
mv "$boot.dot-wake-new" "$boot"
touch /data/opt/alisa-cli/dot-wake-autostart
REMOTE
```

Reboot and verify the service and a real call before claiming autostart works. If the hook differs, do not apply the example blindly.

To disable the listener, remove your wake autostart entry or `/data/opt/alisa-cli/dot-wake-autostart` marker and run `/data/opt/dot-wake/service.sh stop`. Restore your saved boot hook if needed. The existing native client, SSH, and session must remain intact. Do not delete a working backup during rollback.

## Offline checks

These tests use fake clients and a fake recognizer. They do not contact a speaker or account:

```sh
python3 integrations/yandex-station/wake-word/test_prepare.py
python3 integrations/yandex-station/wake-word/test_listener.py
python3 integrations/yandex-station/wake-word/test_bundle.py
python3 integrations/yandex-station/wake-word/test_supervisor.py
python3 integrations/yandex-station/wake-word/test_service.py
```
