#!/usr/bin/env python3
"""Exercise enrollment and listening using generated audio and a fake action."""
import math
import os
from pathlib import Path
import socket
import stat
import struct
import subprocess
import sys
import tempfile
import time


def send_audio(path, frequency):
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    # The initial and final silence bracket a short tone; no speaker is used.
    for frame in range(110):
        values = [
            int(10000 * math.sin(2 * math.pi * frequency * (frame * 160 + i) / 16000))
            if 20 <= frame < 70 else 0
            for i in range(160)
        ]
        try:
            sock.sendto(struct.pack('<160h', *values), str(path))
        except FileNotFoundError:
            break  # Enrollment closes the socket after the utterance.
        time.sleep(.01)
    sock.close()


def wait_socket(path):
    for _ in range(100):
        if path.exists():
            return
        time.sleep(.02)
    raise AssertionError('detector did not open the test socket')


def wait_count(path, count):
    for _ in range(100):
        if path.exists() and len(path.read_text().splitlines()) >= count:
            return
        time.sleep(.02)
    raise AssertionError('expected fake action was not invoked')


def main(binary):
    with tempfile.TemporaryDirectory(prefix='dot-wake-test-') as directory:
        root = Path(directory)
        sock = root / 'input.sock'
        ref = root / 'reference.wake'
        mute = root / 'mute'
        marker = root / 'events'
        action = root / 'fake-action'
        mute.write_text(' gpio-0 ( |mute-switch ) in hi\n')
        action.write_text('#!/bin/sh\nprintf "event\\n" >> "$DOT_TEST_MARKER"\n')
        action.chmod(0o700)
        env = dict(os.environ, DOT_WAKE_MUTE_FILE=str(mute), DOT_TEST_MARKER=str(marker))

        enroll = subprocess.Popen([binary, 'enroll', str(sock), str(ref)], env=env,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        try:
            assert enroll.stdout.readline().strip() == 'ready'
            send_audio(sock, 440)
            stdout, stderr = enroll.communicate(timeout=5)
            assert enroll.returncode == 0, stderr
            assert 'saved' in stdout and stat.S_IMODE(ref.stat().st_mode) == 0o600
        finally:
            if enroll.poll() is None:
                enroll.terminate()
                enroll.wait(timeout=5)

        for invalid_threshold in ('nan', 'inf', '1', '0'):
            result = subprocess.run([binary, 'listen', str(sock), str(ref), invalid_threshold, str(action)],
                                    env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            assert result.returncode != 0 and not sock.exists(), 'invalid threshold was accepted'

        listener = subprocess.Popen([binary, 'listen', str(sock), str(ref), '0.19', str(action)],
                                    env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        try:
            wait_socket(sock)
            send_audio(sock, 3000)
            time.sleep(.5)
            assert not marker.exists(), 'unrelated signal triggered an action'
            send_audio(sock, 440)
            wait_count(marker, 1)
            mute.write_text(' gpio-0 ( |mute-switch ) in lo\n')
            send_audio(sock, 440)
            time.sleep(.5)
            assert len(marker.read_text().splitlines()) == 1, 'muted signal triggered an action'
        finally:
            listener.terminate()
            listener.wait(timeout=5)
        assert not sock.exists(), 'detector left a stale socket'

        # A call may replace the audio socket. The old listener must exit
        # without deleting the new owner's socket.
        listener = subprocess.Popen([binary, 'listen', str(sock), str(ref), '0.19', str(action)],
                                    env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        replacement = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
        try:
            wait_socket(sock)
            sock.unlink()
            replacement.bind(str(sock))
            assert listener.wait(timeout=3) == 0
            assert sock.exists(), 'listener removed the replacement socket'
        finally:
            if listener.poll() is None:
                listener.terminate()
                listener.wait(timeout=5)
            replacement.close()
            if sock.exists():
                sock.unlink()

        # The supervisor must resume listening after a fake call takes and
        # releases the microphone. This uses no station, network, or audio output.
        mute.write_text(' gpio-0 ( |mute-switch ) in hi\n')
        state = root / 'active'
        marker.unlink()
        control = root / 'fake-control'
        control.write_text('''#!/usr/bin/env python3
import os, pathlib, socket, sys, time
state = pathlib.Path(os.environ['DOT_TEST_STATE'])
path = pathlib.Path(os.environ['DOT_WAKE_SOCKET'])
marker = pathlib.Path(os.environ['DOT_TEST_MARKER'])
if sys.argv[1] == 'status':
    print('{"phase": "' + ('active' if state.exists() else 'idle') + '"}')
elif sys.argv[1] == 'open':
    state.write_text('active')
    if path.exists(): path.unlink()
    mic = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    mic.bind(str(path))
    with marker.open('a') as out: out.write('event\\n')
    time.sleep(.5)
    mic.close()
    path.unlink()
    state.unlink()
''')
        control.chmod(0o700)
        supervisor = Path(__file__).with_name('run.sh')
        supervisor_env = dict(env, DOT_WAKE_BINARY=binary, DOT_WAKE_SOCKET=str(sock),
                              DOT_WAKE_CONTROL=str(control), DOT_WAKE_THRESHOLD='0.19',
                              DOT_TEST_STATE=str(state))
        monitor = subprocess.Popen([str(supervisor), str(ref)], env=supervisor_env,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        try:
            wait_socket(sock)
            send_audio(sock, 440)
            wait_count(marker, 1)
            time.sleep(2.5)
            wait_socket(sock)
            send_audio(sock, 440)
            wait_count(marker, 2)
            for _ in range(100):
                if not state.exists():
                    break
                time.sleep(.02)
            assert not state.exists(), 'fake call did not release the socket'
        finally:
            monitor.terminate()
            monitor.wait(timeout=5)
    print('Offline enrollment, matching and software mute tests passed')


if __name__ == '__main__':
    main(sys.argv[1])
