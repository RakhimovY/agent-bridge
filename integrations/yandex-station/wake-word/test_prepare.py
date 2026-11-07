import importlib.util
import pathlib
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('prepare', pathlib.Path(__file__).with_name('prepare.py'))
prepare = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prepare)


class Model:
    def __init__(self, valid):
        self.valid = valid

    def encode(self, text, out_type=int):
        return [1] if text == self.valid else [0]

    def unk_id(self):
        return 0

    def id_to_piece(self, token):
        return '▁WORD' if token == 1 else '<unk>'


class KeywordPreparation(unittest.TestCase):
    def test_phrases_choose_a_compatible_model_and_normalize_case(self):
        models = {'ru': Model('привет'), 'en': Model('HELLO')}
        result = prepare.compile_keywords(['Привет', 'hello'], models)
        self.assertEqual(result, {'ru': ['▁WORD @K0'], 'en': ['▁WORD @K1']})

    def test_distinct_phrases_select_distinct_targets(self):
        models = {'ru': Model('привет'), 'en': Model('HELLO')}
        choices = [{'phrase': 'Привет', 'target': 'dot'}, {'phrase': 'hello', 'target': 'chat'}]
        with tempfile.TemporaryDirectory() as root:
            out = pathlib.Path(root)
            prepare.write_keywords(choices, models, out)
            self.assertEqual((out / 'wake-keywords-ru.txt').read_text(), '▁WORD @K0\n')
            self.assertEqual((out / 'wake-keywords-en.txt').read_text(), '▁WORD @K1\n')
            self.assertEqual((out / 'wake-actions.txt').read_text(), 'K0 dot\nK1 chat\n')
            self.assertEqual((out / 'wake-actions.txt').stat().st_mode & 0o777, 0o600)

    def test_unknown_target_is_rejected_before_writing(self):
        choices = [{'phrase': 'hello', 'target': 'unknown'}]
        with tempfile.TemporaryDirectory() as root:
            with self.assertRaises(ValueError):
                prepare.write_keywords(choices, {'en': Model('HELLO')}, pathlib.Path(root))
            self.assertEqual(list(pathlib.Path(root).iterdir()), [])

    def test_output_is_private_and_replaces_stale_phrases(self):
        models = {'ru': Model('привет'), 'en': Model('HELLO')}
        with tempfile.TemporaryDirectory() as root:
            out = pathlib.Path(root)
            (out / 'wake-keywords-en.txt').write_text('old phrase')
            prepare.write_keywords(['Привет'], models, out)
            self.assertEqual((out / 'wake-keywords-ru.txt').read_text(), '▁WORD @K0\n')
            self.assertEqual((out / 'wake-keywords-en.txt').read_text(), '')
            self.assertEqual((out / 'wake-keywords-ru.txt').stat().st_mode & 0o777, 0o600)
            self.assertEqual((out / 'wake-keywords-en.txt').stat().st_mode & 0o777, 0o600)

    def test_removing_a_model_clears_its_old_phrases(self):
        with tempfile.TemporaryDirectory() as root:
            out = pathlib.Path(root)
            (out / 'wake-keywords-en.txt').write_text('old phrase')
            prepare.write_keywords(['Привет'], {'ru': Model('привет')}, out)
            self.assertEqual((out / 'wake-keywords-en.txt').read_text(), '')

    def test_unknown_phrase_is_rejected_before_writing_any_files(self):
        models = {'ru': Model('привет'), 'en': Model('HELLO')}
        with tempfile.TemporaryDirectory() as root:
            with self.assertRaisesRegex(ValueError, 'phrase 2'):
                prepare.write_keywords(['Привет', 'unknown'], models, pathlib.Path(root))
            self.assertEqual(list(pathlib.Path(root).iterdir()), [])


if __name__ == '__main__':
    unittest.main()
