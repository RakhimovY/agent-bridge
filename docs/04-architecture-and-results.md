# 4. Architecture and results

## Architecture

The stock Alice service receives the activation phrase and passes a local command to the native client. The client runs on the speaker, exchanges audio with OpenAI over WebRTC, and uses a saved ChatGPT web session. An account and internet access are required; a laptop is not used as a proxy.

The `0.3.7` patch selects the Dot when `ALISA_DOT_ID` is present, creates an SDP offer, sends it to the Dot call web interface, applies the SDP answer, opens the data channel, and then attaches the call. The order matters: attaching before the connection was established returned HTTP 502. The client sends a stop request when the call ends. Without the variable, the existing primary ChatGPT path remains in use.

`ALISA_DOT_ID` contains the account and Dot identifiers. The adaptation passes an account header for the relevant web request. This is an observation of browser behavior, not a promise of a stable public API. The original client's synthetic text probe is incompatible with this experiment: the audio channel remained open without it, but closed after roughly half a second when it was sent.

## Results on one device

1. Regular ChatGPT voice worked in native mode, including after reboot.
2. An isolated ARMv7 build without microphone access established a channel and sent 501 and received 469 audio packets over a 10-second test. This checks transport, not a conversation.
3. After a reversible switch, the user asked a question and heard the Dot reply. A completed call of about one minute appeared in the Dot.
4. After reboot, the client started automatically and the user heard another reply. The speaker process had no proxy configured.
5. The close command returned the client to idle; call history was checked in the browser.

We have not tested all models or firmware versions, session expiration, the physical microphone button, long-term stability, privacy on untrusted networks, automatic factory firmware restoration, or session behavior after server-side API changes.

## Adaptation changes

- `dot_signaling.hpp`: constructs requests and validates call URLs without embedding real IDs.
- `web_auth.hpp`: required headers, JSON and SDP, `Location` parsing, and empty POST requests for attaching and stopping.
- `voice_call.hpp`: attaches after WebRTC opens, ends the call, and provides an audio diagnostics mode.
- `effects.hpp` and `daemon.hpp`: generate a telephone-style ringback while a Dot call connects and stop it before voice playback. The waveform and ARM build were checked; audible behavior and call reliability with this change still require confirmation on the device.
- `build_native.py`: Zig and archiver selection through variables and wrappers for cross-building on Mac.
- `dot_signaling.cpp`: local request and URL checks using synthetic IDs.
- `ringback.cpp`: checks the generated tone, stereo output, and silent interval without a speaker.
