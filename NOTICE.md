# Third-party notices

Dot Connect's original documentation, integration guidance, and original contributions are offered under the MIT license in this repository.

The recorded classic handset cues are adapted from [Rotary Phone Pick up and Hang up by cookies+policy](https://freesound.org/people/cookies%2Bpolicy/sounds/556491/), published under CC0 1.0. See [sound asset details](shared/sounds/assets/README.md). The assets retain their source license and are not relicensed by Dot Connect.

The Yandex Station integration includes `patches/alisa-voice-0.3.7-dot.patch`, a patch against [Alisa-ChatGPT-voice-assistant](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant), including context from that upstream project. GitHub did not report a project license for the upstream repository when checked. Dot Connect's MIT license does not grant rights over upstream-owned material. Consult the upstream owner and applicable terms before redistributing or reusing upstream material. Dependency licenses are identified by upstream.

The separate [lite-kit](https://github.com/krevetkqqq/lite-kit) project is linked for context; its own license and third-party notices apply to its contents. Firmware, device backups, account sessions, and private credentials are not included in Dot Connect.

The optional Yandex Station stop detector uses the TensorFlow Lite Micro frontend via [pymicro-features](https://github.com/rhasspy/pymicro-features), a TensorFlow Lite C library from [pymicro-wakeword](https://github.com/OHF-Voice/pymicro-wakeword), and the English stop model from [linux-voice-assistant](https://github.com/OHF-Voice/linux-voice-assistant); consult those projects for authorship and applicable Apache-2.0 and model terms. The isolated build also obtains Debian glibc and libstdc++ components, whose respective license terms apply. These upstream binaries and sources are downloaded by the optional build script rather than included in this repository.

The experimental wake detector also builds against the Apache-2.0 TensorFlow Lite Micro frontend provided by pymicro-features; its downloaded source and bundled third-party components retain their own licenses.
