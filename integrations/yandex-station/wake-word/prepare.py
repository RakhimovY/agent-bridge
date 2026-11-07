#!/usr/bin/env python3
"""Turn text phrases into private sherpa-onnx keyword files."""
import argparse
import json
import os
import sys
from pathlib import Path



def normalize_choices(phrases):
    result = []
    for index, item in enumerate(phrases):
        if isinstance(item, str):
            phrase, target = item, 'default'
        elif isinstance(item, dict):
            phrase, target = item.get('phrase'), item.get('target')
        else:
            phrase, target = None, None
        if not isinstance(phrase, str) or not phrase.strip():
            raise ValueError(f'phrase {index + 1} is empty or not text')
        if target not in ('default', 'dot', 'chat'):
            raise ValueError(f'phrase {index + 1} has an unsupported target')
        result.append((phrase.strip(), target))
    return result


def compile_actions(phrases):
    return [f'K{index} {target}' for index, (_, target) in enumerate(normalize_choices(phrases))]


def compile_keywords(phrases, models):
    result = {name: [] for name in models}
    for index, (phrase, _) in enumerate(normalize_choices(phrases)):
        for name, model in models.items():
            for candidate in dict.fromkeys((phrase, phrase.lower(), phrase.upper())):
                ids = model.encode(candidate, out_type=int)
                if ids and model.unk_id() not in ids:
                    pieces = [model.id_to_piece(token) for token in ids]
                    result[name].append(' '.join(pieces) + f' @K{index}')
                    break
            else:
                continue
            break
        else:
            raise ValueError(f'phrase {index + 1} cannot be encoded by the selected models')
    return result


def write_keywords(phrases, models, output_dir: Path):
    compiled = compile_keywords(phrases, models)
    actions = compile_actions(phrases)
    for name in ('ru', 'en'):
        compiled.setdefault(name, [])
    output_dir.mkdir(mode=0o700, parents=True, exist_ok=True)
    files = {f'wake-keywords-{name}.txt': lines for name, lines in compiled.items()}
    files['wake-actions.txt'] = actions
    for filename, lines in files.items():
        fd = os.open(output_dir / filename, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
        with os.fdopen(fd, 'w', encoding='utf-8') as out:
            os.fchmod(out.fileno(), 0o600)
            if lines:
                out.write('\n'.join(lines) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ru', type=Path, help='Russian model directory')
    parser.add_argument('--en', type=Path, help='English model directory')
    parser.add_argument('--out', type=Path, required=True, help='private output directory')
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
        write_keywords(phrases, models, args.out)
    except (ImportError, OSError, ValueError, RuntimeError) as exc:
        print(f'Could not prepare wake phrases: {exc}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
