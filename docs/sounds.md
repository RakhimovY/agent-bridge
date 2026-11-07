# Call sounds

Agent Bridge defines a shared sound vocabulary for three events: **ringback** while connecting, **connected** when the call is picked up, and **ended** when it is hung up. Each integration declares which of these it implements. Currently only the Yandex Station route implements the pool; a future integration must add and test support rather than assuming it exists.

| Preset | Ringback | Connected / ended |
| --- | --- | --- |
| `classic` (default) | Telephone-style ringback | Recorded handset pickup and hangup |
| `gentle` | Quieter periodic tone | Soft ascending and descending cues |
| `silent` | No sound | No sound |

The original tested telephone ringback remains the `classic` default. Sounds are played locally; no additional online service is used. The classic handset recordings come from the CC0 source documented in [sound assets](../shared/sounds/assets/README.md). If the recorded pack is not installed, the Yandex route uses synthesized handset cues as a fallback. Short clips or spoken words can replace each event independently. `silent` suppresses all cues, including overrides.

## Yandex Station configuration

For the classic handset recordings, first convert and privately transfer the shared WAV assets to the speaker. These commands use the SSH variables defined in the [Yandex route guide](03-use-and-rollback.md) and create a new directory without overwriting an existing one:

```sh
mkdir -p /tmp/dot-connect-sound-pack
ffmpeg -i shared/sounds/assets/classic-connected.wav -f s16le /tmp/dot-connect-sound-pack/connected.pcm
ffmpeg -i shared/sounds/assets/classic-ended.wav -f s16le /tmp/dot-connect-sound-pack/ended.pcm
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; test ! -e /data/opt/dot-connect-sounds && mkdir -p /data/opt/dot-connect-sounds'
tar -C /tmp/dot-connect-sound-pack -cf - connected.pcm ended.pcm | \
  ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; tar -xf - -C /data/opt/dot-connect-sounds'
```

On the speaker, add the desired settings to `/data/opt/alisa-standalone/runtime.env`, preserving existing lines. Restart the native client only when no call is active. For example:

```sh
export DOT_SOUND_PRESET=classic
export DOT_SOUND_PACK_DIR=/data/opt/dot-connect-sounds
# Optional individual overrides; omit an event to use its preset sound:
export DOT_SOUND_RINGBACK=/data/opt/dot-connect-sounds/my-ringback.pcm
export DOT_SOUND_CONNECTED=/data/opt/dot-connect-sounds/my-connected.pcm
export DOT_SOUND_ENDED=/data/opt/dot-connect-sounds/my-ended.pcm
```

Override files must be raw signed 16-bit little-endian PCM, 48 kHz, stereo, at most 30 seconds, with complete stereo frames. Convert your own audio on a computer, for example:

```sh
ffmpeg -i YOUR_INPUT_AUDIO -ar 48000 -ac 2 -f s16le YOUR_OUTPUT.pcm
```

Transfer the output privately to the speaker and set its path; do not commit personal recordings. The ringback clip loops during connection, so include any desired silence in the file. Connected and ended clips play once. An absent, unreadable, empty, incorrectly sized, or overlong override falls back to the selected preset. For `classic`, the installed recorded pack is used when available; without it, synthesized handset cues are used. The client performs no speech synthesis itself: to use words, first create or record audio and convert it as above. The speaker has limited storage, so use short files.

## Add a shared sound

For a new reusable generated preset, update [the shared C++ sound pool](../shared/sounds/dot_sounds.hpp), its offline [tests](../shared/sounds/tests.cpp), the Yandex patch copy, this catalog, and the setup skill. For a curated asset, use `shared/sounds/assets/`, document its source, license and attribution, and add the actual selection implementation and offline tests. Never contribute private recordings or material without redistribution rights.

The Yandex-specific CI runs offline audio tests only when its route, the sound pool, or the Yandex patch changes. It does not connect to a speaker, create a Dot call, or require another integration to install Yandex dependencies.
