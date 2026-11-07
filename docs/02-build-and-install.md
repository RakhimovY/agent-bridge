# 2. Client and build

Installation assumes working root and SSH. First install and verify the standard native mode of [Alisa Voice](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant) according to that version's documentation: authentication, audio, start and stop, and autostart. Its installer documents Linux and Windows; our macOS experiment required manual steps. This guide does not promise automated installation on Mac.

Our base client `0.3.7` was installed under `/data/opt/alisa-standalone`. The session is stored in `state/web-session.json` with mode `600`; the state directory is inaccessible to other users. This grants account access: use it only on a trusted device you own. No session values belong in the repository.

## Prepare the source

Install Git, Python 3, CMake, Ninja, a compiler, and Zig 0.14.1 for your platform on the computer. This example covers macOS on Apple Silicon with Homebrew. Sources and versions are controlled by upstream.

```sh
git clone https://github.com/rison1337/Alisa-ChatGPT-voice-assistant.git alisa-voice
cd alisa-voice
git checkout 2caf3b40635d0bacad1f42b2bb54b326e9c007d8
# Use the path to your local copy of this repository:
git apply --check /PATH/TO/dot-connect/patches/alisa-voice-0.3.7-dot.patch
git apply /PATH/TO/dot-connect/patches/alisa-voice-0.3.7-dot.patch
```

Download `alisa-voice-sources.tar.gz` and its `.sha256` file from the [upstream v0.2.0 release](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/releases/tag/v0.2.0), verify the checksum, and extract only the `engines/native/research/upstream` dependencies. For dependency history and upstream instructions, see [development.md](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/blob/2caf3b40635d0bacad1f42b2bb54b326e9c007d8/docs/development.md). Do not extract an archive over someone else's project without inspecting it first.

```sh
curl -fLO https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/releases/download/v0.2.0/alisa-voice-sources.tar.gz
curl -fLO https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/releases/download/v0.2.0/alisa-voice-sources.tar.gz.sha256
shasum -a 256 -c alisa-voice-sources.tar.gz.sha256
tar -xzf alisa-voice-sources.tar.gz --strip-components=1 alisa-voice-sources/engines/native/research/upstream
```

On macOS we used the official Zig `0.14.1` archive for `aarch64-macos`; the verified archive SHA-256 was `39f3dc5e79c22088ce878edc821dedb4ca5a1cd9f5ef915e9b3cc3053e8faefa`. Verify current Zig sources and checksums before use. The patch adds `DOT_ZIG` and `ar`/`ranlib` wrappers for building on Mac.

```sh
export DOT_ZIG=/PATH/TO/zig
python3 engines/native/scripts/build_native.py --jobs 4
cmake -S engines/native -B engines/native/build/client -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/engines/native/build/arm/toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DALISA_USE_NETEQ=ON
cmake --build engines/native/build/client --target alisa-standalone --parallel 4
file engines/native/build/client/alisa-standalone
```

For our device the output is a 32-bit ARM ELF, hard-float (`arm-linux-gnueabihf`), not a macOS app. Do not add the source archive, binaries, build logs, or local sessions to Git.

## Install the experimental binary

There is no universal safe script for this step: paths, SSH configuration, available space, and backups depend on your installation. With no call in progress, we stopped `run.sh`, saved the previous binary and `runtime.env` in a private separate directory, replaced the binary with our build, added the Dot ID to `runtime.env`, and restarted `run.sh`. Before replacing anything, make sure you can restore your specific configuration.

Our paths:

```text
/data/opt/alisa-standalone/bin/alisa-standalone
/data/opt/alisa-standalone/runtime.env
/data/opt/alisa-standalone/state/web-session.json
/data/opt/alisa-standalone/run.sh
```

The [next section](03-use-and-rollback.md) contains backup, copy, and startup commands with variables instead of actual addresses or IDs. Do not run them until you have checked paths and can restore the previous client.
