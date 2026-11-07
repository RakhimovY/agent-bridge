import importlib.util
import pathlib
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('bundle', HERE / 'bundle.py')
bundle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bundle)


class BundleTest(unittest.TestCase):
    def test_bundle_rejects_unavailable_separate_targets(self):
        with tempfile.TemporaryDirectory() as root:
            out = pathlib.Path(root) / 'bundle'
            with self.assertRaises(ValueError):
                bundle.create_bundle(out, {}, {}, None, None, None, {}, ['K0 chat'])
            self.assertFalse(out.exists())

    def test_never_overwrites_an_existing_directory(self):
        with tempfile.TemporaryDirectory() as root:
            output = pathlib.Path(root) / 'bundle'
            output.mkdir()
            (output / 'existing').write_text('keep')
            with self.assertRaises(FileExistsError):
                bundle.create_bundle(output, {}, {}, None, None, None, {})
            self.assertEqual((output / 'existing').read_text(), 'keep')

    def test_bundle_contains_private_keywords_and_required_files(self):
        with tempfile.TemporaryDirectory() as root:
            root = pathlib.Path(root)
            model = root / 'model'
            model.mkdir()
            for name in ('encoder.int4.onnx', 'decoder.onnx', 'joiner.int8.onnx', 'tokens.txt'):
                (model / name).write_bytes(b'fixture')
            runtime = root / 'runtime'
            runtime.mkdir()
            for name in bundle.RUNTIME_FILES:
                (runtime / name).write_bytes(b'fixture')
            libs = root / 'libs'
            libs.mkdir()
            for name in bundle.LIBRARY_FILES:
                (libs / name).write_bytes(b'fixture')
            listener = root / 'listener'
            listener.write_bytes(b'fixture')
            out = root / 'result'
            bundle.create_bundle(out, {'ru': ['▁TEST @K0']}, {'ru': model}, runtime, libs, listener,
                                 {'ru': ('encoder.int4.onnx', 'decoder.onnx', 'joiner.int8.onnx', 'tokens.txt')}, ['K0 default'])
            self.assertTrue((out / 'runtime.tar.gz').is_file())
            self.assertTrue((out / 'ru' / 'encoder.int4.onnx').is_file())
            self.assertTrue((out / 'run.sh').is_file())
            self.assertEqual((out / 'service.sh').stat().st_mode & 0o777, 0o700)
            self.assertEqual((out / 'wake-keywords-ru.txt').stat().st_mode & 0o777, 0o600)
            self.assertEqual((out / 'wake-keywords-en.txt').read_text(), '')
            self.assertEqual((out / 'wake-actions.txt').read_text(), 'K0 default\n')
            self.assertEqual((out / 'wake-actions.txt').stat().st_mode & 0o777, 0o600)
            self.assertEqual((out / 'listener').stat().st_mode & 0o777, 0o700)


if __name__ == '__main__':
    unittest.main()
