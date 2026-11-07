# Yandex Station integration

The current route runs an adapted native Alisa Voice client on the speaker. Wi-Fi connects the client to a specific Dot; the stock Alice service recognizes the tested activation command. A computer is required for setup, but not continuously for calls.

**Evidence:** one Yandex Station Lite 1 (`YNDX-00025`, `yandexmicro`) on firmware `0.369.1.46.4778262442.20261013.297`; voice response and autostart were confirmed before and after reboot. Do not extrapolate this to other devices or firmware.

1. [Prepare the speaker and root access](../../docs/01-station.md). The successful root transition is not fully reconstructed.
2. [Install the native client and build the patch](../../docs/02-build-and-install.md).
3. [Connect the Dot, test, and roll back](../../docs/03-use-and-rollback.md).
4. [Architecture and evidence](../../docs/04-architecture-and-results.md).
5. [Secret safety](../../docs/05-security.md).

Our tested commands are **"Алиса, открой чат"** ("Alice, open the chat") and **"Алиса, закрой чат"** ("Alice, close the chat"). The Russian text is what was spoken; English is an explanation. Users may choose preferred commands during setup, but a new wake word and bare stop are not verified. Physical mute in this mode is also unverified.

The current patch includes an experimental telephone-style ringback during connection. The user confirmed hearing it and a Dot reply on the tested device. Reboot and long-term reliability with this change have not been separately verified.

Based on [Alisa-ChatGPT-voice-assistant](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant) 0.3.7 at commit `2caf3b40635d0bacad1f42b2bb54b326e9c007d8` and the [lite-kit](https://github.com/krevetkqqq/lite-kit) toolkit. See [NOTICE](../../NOTICE.md) for license boundaries.
