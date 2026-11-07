import os
from pathlib import Path
import socket
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


class SupervisorTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.mic = self.root / 'mic.sock'
        self.state = self.root / 'phase'
        self.state.write_text('idle')
        (self.root / 'meminfo').write_text('MemAvailable: 100000 kB\n')
        (self.root / 'wake-actions.txt').write_text('K0 default\nK1 chat\n')
        client = self.root / 'client'
        client.write_text('''#!/usr/bin/env python3
import json,os,pathlib,sys
p=pathlib.Path(os.environ['TEST_ROOT']);phase=(p/'phase').read_text()
if sys.argv[1]=='status': print(json.dumps({'phase':phase,'agent_connected':True}))
elif sys.argv[1] in ('open','open-dot','open-chat'):
 if phase!='idle' or (p/'mic.sock').exists(): sys.exit(1)
 with (p/'opens').open('a') as f:f.write(sys.argv[1]+'\\n')
 (p/'phase').write_text('active')
''')
        listener = self.root / 'listener'
        listener.write_text('''#!/usr/bin/env python3
import os,pathlib,signal,socket,sys,time
p=pathlib.Path(os.environ['TEST_ROOT']);path=sys.argv[1]
if os.environ.get('DOT_WAKE_PREWARM')=='1':
 ready=[False]
 signal.signal(signal.SIGUSR1,lambda *_:ready.__setitem__(0,True))
 (p/'warm-started').write_text('1')
 print('WARM',flush=True)
 while not ready[0]:time.sleep(.01)
s=socket.socket(socket.AF_UNIX,socket.SOCK_DGRAM);s.bind(path)
try:
 data=s.recv(32)
 if data==b'crash':sys.exit(42)
 if data.startswith(b'match'):
  os.unlink(path);(p/'detected').write_text('1')
  while (p/'hold').exists():time.sleep(.01)
  print('MATCH '+(data.decode().split()[1] if len(data.split())>1 else 'K0'),flush=True)
  sys.exit(0)
finally:
 s.close()
 if os.path.exists(path):os.unlink(path)
''')
        client.chmod(0o700)
        listener.chmod(0o700)
        env = dict(os.environ, TEST_ROOT=str(self.root), DOT_WAKE_CLIENT=str(client),
                   DOT_WAKE_STATE=str(self.root), DOT_WAKE_LISTENER=str(listener),
                   DOT_WAKE_SOCKET=str(self.mic), DOT_WAKE_ACTIONS=str(self.root / 'wake-actions.txt'),
                   DOT_WAKE_MEMINFO=str(self.root / 'meminfo'))
        self.proc = subprocess.Popen(['sh', str(HERE / 'supervisor.sh'), 'fake-model'], env=env,
                                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.addCleanup(self.stop_process)

    def stop_process(self):
        if self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=4)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()

    def match(self, key='K0'):
        wait_until(lambda: self.mic.exists(), 'listener did not bind')
        with socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM) as sender:
            sender.sendto(('match ' + key).encode(), str(self.mic))

    def opens(self):
        p = self.root / 'opens'
        return p.read_text().count('open') if p.exists() else 0

    def test_open_stop_and_open_again_with_fake_client(self):
        self.match()
        wait_until(lambda: self.opens() == 1, 'first open was not sent')
        self.assertEqual(self.state.read_text(), 'active')
        time.sleep(1.2)
        self.assertFalse(self.mic.exists(), 'listener competed with active call')
        self.state.write_text('idle')
        self.match()
        wait_until(lambda: self.opens() == 2, 'second open was not sent')

    def test_prewarm_does_not_take_mic_until_call_ends(self):
        self.match()
        wait_until(lambda: self.opens() == 1, 'first open was not sent')
        wait_until(lambda: (self.root / 'warm-started').exists(), 'listener did not prewarm')
        self.assertEqual(self.state.read_text(), 'active')
        self.assertFalse(self.mic.exists(), 'prewarm took the call microphone')
        self.state.write_text('idle')
        wait_until(lambda: self.mic.exists(), 'prewarmed listener did not resume', timeout=3)
        self.match()
        wait_until(lambda: self.opens() == 2, 'second open was not sent')

    def test_prewarmed_listener_rearms_promptly_when_call_finishes(self):
        self.match()
        wait_until(lambda: (self.root / 'warm-started').exists(), 'listener did not prewarm')
        time.sleep(0.1)
        self.state.write_text('idle')
        wait_until(lambda: self.mic.exists(), 'prewarmed listener resumed too slowly', timeout=0.7)

    def test_low_memory_stops_prewarm_and_blocks_restart_until_safe(self):
        self.match()
        wait_until(lambda: self.opens() == 1, 'first open was not sent')
        wait_until(lambda: (self.root / 'warm-started').exists(), 'listener did not prewarm')
        (self.root / 'meminfo').write_text('MemAvailable: 20000 kB\n')
        time.sleep(2)
        self.state.write_text('idle')
        time.sleep(2)
        self.assertFalse(self.mic.exists(), 'listener started while memory was low')
        (self.root / 'meminfo').write_text('MemAvailable: 100000 kB\n')
        wait_until(lambda: self.mic.exists(), 'listener did not recover when memory returned')

    def test_phrases_route_to_dot_and_chat_separately(self):
        (self.root / 'wake-actions.txt').write_text('K0 dot\nK1 chat\n')
        self.match('K0')
        wait_until(lambda: self.opens() == 1, 'dot action not sent')
        self.assertEqual((self.root / 'opens').read_text().splitlines(), ['open-dot'])
        time.sleep(3)
        self.state.write_text('idle')
        self.match('K1')
        wait_until(lambda: self.opens() == 2, 'chat action not sent')
        self.assertEqual((self.root / 'opens').read_text().splitlines(), ['open-dot', 'open-chat'])

    def test_unknown_target_does_not_open_anything(self):
        (self.root / 'wake-actions.txt').write_text('K0 unsupported\n')
        self.match('K0')
        self.assertEqual(self.proc.wait(timeout=5), 1)
        self.assertEqual(self.opens(), 0)

    def test_listener_failure_does_not_restart_or_open(self):
        wait_until(lambda: self.mic.exists(), 'listener did not bind')
        with socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM) as sender:
            sender.sendto(b'crash', str(self.mic))
        try:
            code = self.proc.wait(timeout=4)
        except subprocess.TimeoutExpired:
            self.fail('supervisor restarted a failed listener')
        self.assertEqual(code, 42)
        self.assertEqual(self.opens(), 0)

    def test_no_open_when_another_call_starts_during_detection(self):
        (self.root / 'hold').write_text('1')
        self.match()
        wait_until(lambda: (self.root / 'detected').exists(), 'match was not received')
        self.state.write_text('active')
        (self.root / 'hold').unlink()
        time.sleep(2)
        self.assertEqual(self.opens(), 0)
        self.assertFalse(self.mic.exists())


if __name__ == '__main__':
    unittest.main()
