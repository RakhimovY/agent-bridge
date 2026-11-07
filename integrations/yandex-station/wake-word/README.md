# Experimental wake phrase detector

This is a prototype for recognizing user-selected phrases locally, without saying the station's built-in wake word. It is **not yet an available setup path**: live recognition, reliable false-activation behavior, physical microphone mute, call handoff, and autostart have not been verified. It is not installed or enabled by the normal setup. Continue to use the existing tested command for calls.

The detector compares short utterances with private reference examples. It does not require training a model for each phrase. It uses the existing TensorFlow Lite Micro audio frontend, but does not load an AI model for matching. Several reference files can represent different phrases or speakers. A user would need to provide their own examples; matching is speaker-dependent and similarity varies with the recording environment. A selected text phrase alone does not create a working reference.

## Build

Requires Zig 0.14.1, Python 3, curl and tar. This command downloads the pinned frontend source, checks its SHA-256 checksum and builds an ARMv7 binary; it does not access a speaker or account:

```sh
export DOT_ZIG=/path/to/zig
integrations/yandex-station/wake-word/build.sh
```

The default output is `integrations/yandex-station/wake-word/.build/wake-detector`. `DOT_WAKE_BUILD_DIR` can place the build outside the repository. The build is scoped to this integration.

## Prototype interface

The detector accepts 16 kHz, mono, signed 16-bit little-endian PCM. It exposes four commands:

```text
wake-detector features INPUT.pcm
wake-detector compare REFERENCE.wake CANDIDATE.wake THRESHOLD
wake-detector enroll SOCKET NEW_REFERENCE.wake
wake-detector listen SOCKET REFERENCE.wake THRESHOLD ACTION [REFERENCE.wake ...]
```

`features` prints numeric features; redirect them to a private file with `umask 077`. `enroll` waits for one utterance on a Unix datagram socket, writes derived features in a new mode-600 file and does not save raw audio. It prints `ready` and then `saved`. `listen` accepts multiple reference files and runs the specified executable with the argument `open` on a match. Use a harmless fake executable during testing: a real action could start a call unexpectedly. Neither `enroll` nor `listen` runs unless `DOT_WAKE_MUTE_FILE` points to a readable GPIO-style file that permits microphone use. This check alone does **not** verify that the tested speaker's physical mute works.

Treat reference features as sensitive voice-derived data. Store them outside the repository, do not publish or share them, and remove them when no longer needed. The `.wake` extension is ignored by Git as a precaution; secret scanning is not a guarantee.

The threshold is an experiment-specific number, not a universal setting. Short and similar-sounding phrases, multiple speakers, background speech and noise can cause misses or false matches. Choose examples and validate both matches and ordinary conversation before considering any real action.

`run.sh` is a prototype supervisor: it checks the configured client's `status`, starts the listener while idle, and waits for the microphone socket to become available again after a simulated or actual call. Configure `DOT_WAKE_BINARY`, `DOT_WAKE_SOCKET`, `DOT_WAKE_MUTE_FILE`, `DOT_WAKE_CONTROL`, and `DOT_WAKE_THRESHOLD`, then pass one or more reference files as arguments. Its call handoff has only been tested with a fake client. Do not add it to autostart or point it at a real client as part of normal setup yet.

## Evidence

One Lite 1 ran the prototype without a real call. In isolated tests, a fake action received events for five synthetic phrase recordings and did not receive events for selected synthetic and prerecorded unrelated voices. Other tests exercised reference enrollment, several references, repeated detection, a software mute fixture, socket cleanup and microphone handoff to a fake client. These small tests do not establish real-world accuracy, compatibility with other speakers or behavior across reboot. No live speaker voice has yet been verified with this prototype.

The Yandex-scoped CI runs `test_offline.py` with generated signals, temporary sockets and a fake client. It does not connect to a station or ChatGPT. It also runs the device-independent matcher unit tests in `shared/wake/`.
