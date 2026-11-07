# Text-configured wake phrases: experimental

This directory contains phrase preparation and an experimental listener, not a working wake-word installation. Starting a Dot call without the station's built-in wake word is not implemented or enabled. The detector, microphone mute, call handoff, and false-trigger behavior still need verification on the device.

`prepare.py` converts a JSON array of text phrases from standard input into private, model-specific sherpa-onnx keyword files. It uses SentencePiece model files and does not use a user's voice samples or identify speakers. It selects the first supplied model that can encode each phrase, trying original, lowercase and uppercase forms. The current experimental models use Russian and English vocabularies; an unsupported or mixed-script phrase is rejected instead of silently dropped. Encoding does not guarantee recognition of a word or pronunciation.

For local development, install `sentencepiece` in a virtual environment and obtain compatible Russian and English model directories with their `bpe.model` files. Keep the phrase JSON and output files private. The following generic example only prepares files; it does not contact a speaker or start a call:

```sh
printf '["Привет друг", "hello friend"]' | python3 prepare.py \
  --ru /path/to/russian-model --en /path/to/english-model --out /private/path/keywords
```

`wake-keywords-ru.txt` and `wake-keywords-en.txt` contain tokenized phrases and are created with mode 600. A generated alias such as `K0` refers to the corresponding input position. Output files are written only after all input phrases have been encoded. Do not commit generated keyword files, models, recordings, account data, or private device details.

Run the preparation tests without models or a speaker:

```sh
python3 integrations/yandex-station/wake-word/test_prepare.py
```

## Experimental listener

`listener.cpp` receives mono 16 kHz PCM on a Unix datagram socket. It buffers short packets, detects a speech segment, and checks it against one or more keyword models. On a match it releases its socket, prints `MATCH`, and exits successfully. It does not start calls. If the socket is replaced or the process is stopped, it exits without a match and does not remove the new owner's socket.

Build using the `sherpa-onnx` C API header and matching library from the same release. For example, with an appropriate C++ compiler and a local installation:

```sh
c++ -std=c++17 -O2 -I"$SHERPA/include" listener.cpp \
  -L"$SHERPA/lib" -lsherpa-onnx-c-api -o /tmp/dot-wake-listener
```

The listener accepts a socket path followed by one or more groups of five paths: keyword file, encoder, decoder, joiner, and tokens. Provide only groups with nonempty keyword files. On the tested Station Lite, both a Russian 4-bit model and an English 8-bit model loaded with one inference thread each; two synthetic phrases were detected in a limited-memory, fake-socket test. This does not establish live accuracy or long-term resource safety. During inference, incoming packets may be lost; continuous speech and false activations are not yet characterized.

Use a fake socket and fake action for development. Do not enable this listener on a live microphone or connect its output to a call until physical mute, live recognition, resource limits, and call handoff have been verified. The listener has no microphone-mute check.

The Yandex-scoped offline tests compile against a fake recognizer to exercise short packets, multiple model groups, and socket replacement. They use neither a device nor a ChatGPT account:

```sh
python3 integrations/yandex-station/wake-word/test_listener.py
```
