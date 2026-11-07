import os
import select
from pathlib import Path
import socket
import struct
import subprocess
import tempfile
import time
import unittest

HERE = Path(__file__).resolve().parent


class ListenerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.binary = Path(cls.temp.name) / 'listener'
        subprocess.run(['c++', '-std=c++17', '-O2', '-I', str(HERE / 'tests/fake'),
                        str(HERE / 'listener.cpp'), '-o', str(cls.binary)], check=True)

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def start_listener(self, path, models=None):
        models = models or ['keywords', 'encoder', 'decoder', 'joiner', 'tokens']
        p = subprocess.Popen([str(self.binary), str(path)] + models,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        def cleanup():
            if p.poll() is None:
                p.kill()
                p.wait()
            p.stdout.close()
            p.stderr.close()
        self.addCleanup(cleanup)
        self.assertTrue(select.select([p.stdout], [], [], 5)[0], 'listener did not initialize')
        self.assertEqual(p.stdout.readline().strip(), 'READY')
        return p

    def test_short_audio_packets_produce_one_signal_and_release_socket(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'mic.sock'
            p = self.start_listener(path)
            sender = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
            self.addCleanup(sender.close)
            samples = [1000] * 1024 + [0] * 6400
            samples[490] = 10000
            for offset in range(0, len(samples), 512):
                block = samples[offset:offset + 512]
                for i in range(0, len(block), 160):
                    part = block[i:i + 160]
                    try:
                        sender.sendto(struct.pack('<' + 'h' * len(part), *part), str(path))
                    except FileNotFoundError:
                        break
                if not path.exists():
                    break
                time.sleep(0.01)
            try:
                out, err = p.communicate(timeout=5)
            except subprocess.TimeoutExpired:
                p.kill()
                p.communicate()
                self.fail('listener did not signal a match')
            self.assertEqual(p.returncode, 0, err)
            self.assertEqual(out.strip(), 'MATCH')
            self.assertFalse(path.exists())

    def test_unrelated_audio_does_not_signal_and_termination_releases_socket(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'mic.sock'
            p = self.start_listener(path)
            sender = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
            self.addCleanup(sender.close)
            for sample in [1000] * 10 + [0] * 40:
                sender.sendto(struct.pack('<160h', *([sample] * 160)), str(path))
                time.sleep(0.005)
            time.sleep(0.3)
            self.assertIsNone(p.poll(), 'unrelated audio stopped the listener')
            p.terminate()
            out, err = p.communicate(timeout=5)
            self.assertNotEqual(p.returncode, 0, err)
            self.assertNotIn('MATCH', out)
            self.assertFalse(path.exists())

    def test_second_model_can_produce_the_signal(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'mic.sock'
            models = ['negative', 'encoder', 'decoder', 'joiner', 'tokens',
                      'positive', 'encoder', 'decoder', 'joiner', 'tokens']
            p = self.start_listener(path, models)
            sender = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
            self.addCleanup(sender.close)
            for sample in [10000] * 2 + [1000] * 8 + [0] * 40:
                try:
                    sender.sendto(struct.pack('<160h', *([sample] * 160)), str(path))
                except FileNotFoundError:
                    break
                time.sleep(0.005)
            out, err = p.communicate(timeout=5)
            self.assertEqual(p.returncode, 0, err)
            self.assertEqual(out.strip(), 'MATCH')
            self.assertFalse(path.exists())

    def test_replaced_socket_cancels_a_match_in_progress(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'mic.sock'
            p = self.start_listener(path, ['slow', 'encoder', 'decoder', 'joiner', 'tokens'])
            sender = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
            self.addCleanup(sender.close)
            for sample in [10000] * 2 + [1000] * 8 + [0] * 30:
                sender.sendto(struct.pack('<160h', *([sample] * 160)), str(path))
                time.sleep(0.005)
            self.assertTrue(select.select([p.stderr], [], [], 5)[0], 'listener did not decode')
            self.assertEqual(p.stderr.readline().strip(), 'DECODING')
            os.unlink(path)
            replacement = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
            self.addCleanup(replacement.close)
            replacement.bind(str(path))
            out, err = p.communicate(timeout=5)
            self.assertNotEqual(p.returncode, 0, err)
            self.assertNotIn('MATCH', out)
            self.assertTrue(path.exists())

    def test_replaced_socket_is_left_for_the_new_owner(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / 'mic.sock'
            p = self.start_listener(path)
            os.unlink(path)
            replacement = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
            self.addCleanup(replacement.close)
            replacement.bind(str(path))
            try:
                _, err = p.communicate(timeout=5)
            except subprocess.TimeoutExpired:
                p.kill()
                p.communicate()
                self.fail('listener did not release a replaced socket')
            self.assertNotEqual(p.returncode, 0, err)
            self.assertTrue(path.exists())


if __name__ == '__main__':
    unittest.main()
