import os
from pathlib import Path
import subprocess
import tempfile
import time
import unittest

HERE = Path(__file__).resolve().parent


def wait_until(condition, message, timeout=8):
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        if condition():
            return
        time.sleep(0.05)
    raise AssertionError(message)


class ServiceTest(unittest.TestCase):
    @unittest.skipUnless(Path('/proc/self/cmdline').is_file(), 'requires Linux process information')
    def test_stop_then_start_waits_for_the_previous_service(self):
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            service = root / 'service.sh'
            service.write_text((HERE / 'service.sh').read_text())
            service.chmod(0o700)
            runner = root / 'run.sh'
            runner.write_text('''#!/bin/sh
echo start >> "$TEST_ROOT/starts"
trap 'sleep 1; exit 0' TERM INT
while :; do sleep 1; done
''')
            runner.chmod(0o700)
            mem = root / 'meminfo'
            mem.write_text('MemAvailable: 100000 kB\n')
            env = dict(os.environ, TEST_ROOT=str(root), DOT_WAKE_MEMINFO=str(mem),
                       DOT_WAKE_PIDFILE=str(root / 'service.pid'))
            def command(action):
                return subprocess.run([str(service), action], env=env, capture_output=True, timeout=8)
            starts = root / 'starts'
            try:
                self.assertEqual(command('start').returncode, 0)
                wait_until(lambda: starts.exists(), 'service did not start')
                self.assertEqual(command('stop').returncode, 0)
                self.assertEqual(command('start').returncode, 0)
                wait_until(lambda: len(starts.read_text().splitlines()) == 2,
                           'service did not restart')
                self.assertEqual(command('status').returncode, 0)
            finally:
                command('stop')

    def test_low_memory_stops_and_recovers_without_a_time_limit(self):
        with tempfile.TemporaryDirectory() as root:
            root = Path(root)
            service = root / 'service.sh'
            service.write_text((HERE / 'service.sh').read_text())
            runner = root / 'run.sh'
            runner.write_text('''#!/bin/sh
echo start >> "$TEST_ROOT/starts"
trap 'echo stop >> "$TEST_ROOT/stops"; exit 0' TERM INT
while :; do sleep 1; done
''')
            runner.chmod(0o700)
            mem = root / 'meminfo'
            mem.write_text('MemAvailable: 100000 kB\n')
            env = dict(os.environ, TEST_ROOT=str(root), DOT_WAKE_MEMINFO=str(mem))
            p = subprocess.Popen(['sh', str(service), 'watch'], env=env,
                                 stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            try:
                starts = root / 'starts'
                stops = root / 'stops'
                wait_until(lambda: starts.exists(), 'service did not start')
                mem.write_text('MemAvailable: 20000 kB\n')
                wait_until(lambda: stops.exists(), 'low memory did not stop listener')
                time.sleep(1)
                self.assertEqual(len(starts.read_text().splitlines()), 1)
                mem.write_text('MemAvailable: 100000 kB\n')
                wait_until(lambda: len(starts.read_text().splitlines()) == 2,
                           'service did not recover after memory returned')
                self.assertIsNone(p.poll())
            finally:
                p.terminate()
                try:
                    p.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    p.kill()
                    p.wait()


if __name__ == '__main__':
    unittest.main()
