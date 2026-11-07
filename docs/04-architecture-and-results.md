# 4. Architecture and results

## Architecture

The stock Alice service can pass a local command to the native client. An experimental separate wake listener can instead detect a configured phrase locally and hand the microphone to that same client. The client runs on the speaker, exchanges audio with OpenAI over WebRTC, and uses a saved ChatGPT web session. An account and internet access are required; a laptop is not used as a proxy.

The `0.3.7` patch selects the Dot when `ALISA_DOT_ID` is present, creates an SDP offer, sends it to the Dot call web interface, applies the SDP answer, opens the data channel, and then attaches the call. The order matters: attaching before the connection was established returned HTTP 502. The client sends a stop request when the call ends. Without the variable, the existing primary ChatGPT path remains in use.

`ALISA_DOT_ID` contains the account and Dot identifiers. The adaptation passes an account header for the relevant web request. This is an observation of browser behavior, not a promise of a stable public API. The original client's synthetic text probe is incompatible with this experiment: the audio channel remained open without it, but closed after roughly half a second when it was sent.

## Results on one device

1. Regular ChatGPT voice worked in native mode, including after reboot.
2. An isolated ARMv7 build without microphone access established a channel and sent 501 and received 469 audio packets over a 10-second test. This checks transport, not a conversation.
3. After a reversible switch, the user asked a question and heard the Dot reply. A completed call of about one minute appeared in the Dot.
4. After reboot, the client started automatically and the user heard another reply. The speaker process had no proxy configured.
5. The close command returned the client to idle; call history was checked in the browser.
6. With the ringback candidate installed, the user confirmed hearing the call tone followed by a Dot voice reply. An automated call also reached active audio and returned to idle after a close request. Reboot with this ringback version has not been separately verified.
7. The optional local stop detector recognized two synthetic English "stop" voices and did not trigger on two synthetic ordinary phrases. Two automatic call/stop/reopen cycles succeeded. These injected audio into the test device's capture path; the user confirmed real-microphone stop and reopening on the tested device.
8. The user confirmed that the CC0 recordings of a real telephone handset sounded good on the tested speaker. Alternative presets and per-event overrides were tested offline only.
9. A text-configured local wake phrase opened the Dot without Alice, and the owner confirmed an unlimited call, bare stop, reopening and another call after reboot. The physical mute input changed with the button; muted-voice behavior has automated tests but remains unverified live. Recognition can miss phrases or take several seconds after a call.

We have not tested all models or firmware versions, session expiration, live muted-voice suppression, long-term stability, privacy on untrusted networks, automatic factory firmware restoration, or session behavior after server-side API changes.

## Adaptation changes

- `dot_signaling.hpp`: constructs requests and validates call URLs without embedding real IDs.
- `web_auth.hpp`: required headers, JSON and SDP, `Location` parsing, and empty POST requests for attaching and stopping.
- `voice_call.hpp`: attaches after WebRTC opens, ends the call, provides an audio diagnostics mode, and optionally forwards capture audio to a local stop detector only for a configured Dot call.
- `dot_sounds.hpp`, `effects.hpp`, and `daemon.hpp`: implement the shared sound pool, per-event custom PCM, and ringback during connection. The original ringback and recorded handset cues were confirmed by the user on the device; alternative presets and per-event overrides have offline tests only.
- `integrations/yandex-station/stop-detector`: optional separate-process local stop detector and isolated build, with no replacement of system libraries.
- `build_native.py`: Zig and archiver selection through variables and wrappers for cross-building on Mac.
- `dot_signaling.cpp`: local request and URL checks using synthetic IDs.
- `ringback.cpp`: checks the generated tone, stereo output, and silent interval without a speaker.
