#!/usr/bin/env python3
"""Create a private, experimental Station Lite wake bundle."""
import argparse
import json
import os
from pathlib import Path
import shutil
import sys
import tarfile

import prepare

RUNTIME_FILES = ('ld-linux-armhf.so.3', 'libc.so.6', 'libdl.so.2', 'libm.so.6',
                 'libpthread.so.0', 'librt.so.1', 'libstdc++.so.6', 'libstdc++.so.6.0.30')
LIBRARY_FILES = ('libonnxruntime.so', 'libsherpa-onnx-c-api.so')
MODEL_FILES = {
    'ru': ('encoder.int4.onnx', 'decoder.onnx', 'joiner.int8.onnx', 'tokens.txt'),
    'en': ('encoder-epoch-12-avg-2-chunk-16-left-64.int8.onnx',
           'decoder-epoch-12-avg-2-chunk-16-left-64.onnx',
           'joiner-epoch-12-avg-2-chunk-16-left-64.int8.onnx', 'tokens.txt'),
}


def create_bundle(output, compiled, models, runtime, libs, listener, model_files, actions=None):
    if output.exists():
        raise FileExistsError('output directory already exists')
    if any(len(line.split()) != 2 or line.split()[1] != 'default' for line in (actions or [])):
        raise ValueError('separate Dot and ChatGPT wake targets are not supported by the released client')
    sources = [listener] + [runtime / name for name in RUNTIME_FILES] + [libs / name for name in LIBRARY_FILES]
    for name, lines in compiled.items():
        if lines:
            sources.extend(models[name] / filename for filename in model_files[name])
    for source in sources:
        if not source.is_file():
            raise FileNotFoundError(f'missing required artifact: {source.name}')
    output.mkdir(mode=0o700, parents=True)
    with tarfile.open(output / 'runtime.tar.gz', 'w:gz', compresslevel=1) as archive:
        for name in RUNTIME_FILES:
            archive.add(runtime / name, arcname=name)
        for name in LIBRARY_FILES:
            archive.add(libs / name, arcname=name)
    for name in ('ru', 'en'):
        words = output / f'wake-keywords-{name}.txt'
        words.write_text(''.join(line + '\n' for line in compiled.get(name, [])), encoding='utf-8')
        words.chmod(0o600)
        if compiled.get(name):
            (output / name).mkdir(mode=0o700)
            for filename in model_files[name]:
                shutil.copyfile(models[name] / filename, output / name / filename)
    action_file = output / 'wake-actions.txt'
    action_file.write_text(''.join(line + '\n' for line in (actions or [])), encoding='utf-8')
    action_file.chmod(0o600)
    for source, name in [(listener, 'listener'), *((Path(__file__).with_name(name), name)
                         for name in ('supervisor.sh', 'run.sh', 'launch-listener.sh', 'service.sh'))]:
        shutil.copyfile(source, output / name)
        (output / name).chmod(0o700)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ru', type=Path)
    parser.add_argument('--en', type=Path)
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--libs', type=Path, required=True)
    parser.add_argument('--listener', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    selected = {name: path for name, path in [('ru', args.ru), ('en', args.en)] if path}
    if not selected:
        parser.error('select at least one model')
    try:
        import sentencepiece
        models = {name: sentencepiece.SentencePieceProcessor(model_file=str(path / 'bpe.model'))
                  for name, path in selected.items()}
        phrases = json.load(sys.stdin)
        if not isinstance(phrases, list) or not phrases:
            raise ValueError('input must be a nonempty JSON array of phrases')
        compiled = prepare.compile_keywords(phrases, models)
        os.umask(0o077)
        create_bundle(args.out, compiled, selected, args.runtime, args.libs, args.listener, MODEL_FILES,
                      prepare.compile_actions(phrases))
    except (ImportError, OSError, ValueError, RuntimeError) as exc:
        print(f'Could not create wake bundle: {exc}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
