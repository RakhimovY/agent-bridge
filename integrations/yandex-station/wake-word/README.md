# Text-configured wake phrases: experimental

This directory contains a small phrase preparation tool, not a working wake-word installation. Starting a Dot call without the station's built-in wake word is not implemented or enabled. The detector, microphone mute, call handoff, and false-trigger behavior still need verification on the device.

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
