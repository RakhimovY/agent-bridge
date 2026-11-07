# ChatGPT Dot on Yandex Station Lite 1

An experiment in calling a specific ChatGPT Dot directly from a Yandex Station Lite 1, without a computer running continuously. The speaker uses its own Wi-Fi, microphones, and speaker; a separate WebRTC client runs on the device.

```text
"Алиса, открой чат" -> local command -> client on the speaker -> Dot call
```

This is an add-on: the stock Alice assistant remains in place to start and stop calls. Tested on November 7, 2026 on one Station Lite 1 (`YNDX-00025`, `yandexmicro`): the Dot replied by voice, then replied again after a reboot. A completed call appeared in the Dot's history. Other models and firmware versions have not been tested here.

## Before you start

- You need your own rooted speaker with SSH, a ChatGPT account with access to a Dot, and a home network with internet access.
- Rooting and downgrading firmware can erase settings or render the device unusable. Firmware updates are disabled on our speaker, so security updates are not installed either.
- This is a research integration with an undocumented OpenAI web interface, not an official SDK. The interface, authentication, and subscription limits can change. This implementation does not use an API key; a sensitive web session is stored on the speaker.
- The physical microphone mute has not been verified in this experimental mode on our device. Do not assume the hardware button works.
- Never publish real Dot IDs, IP addresses, cookies, tokens, SSH keys, dumps, firmware, or logs. Use a trusted home network segment. Do not expose SSH or ADB to the internet.

## Guide

1. [Prepare the speaker, obtain root, and restore the network](docs/01-station.md).
2. [Install the voice client and build the adaptation](docs/02-build-and-install.md).
3. [Connect your Dot, test it, and roll back](docs/03-use-and-rollback.md).
4. [Architecture and test results](docs/04-architecture-and-results.md).
5. [Protect secrets and publications](docs/05-security.md).

The code changes are in [the patch](patches/alisa-voice-0.3.7-dot.patch). It targets a specific upstream client commit, not an arbitrary latest version. This is an engineering experiment, not a universal installer: after applying the patch and building, you must install the existing client and manually replace its binary. Read the full guide before changing your speaker.

## Voice commands

The commands below are the exact Russian phrases configured on our speaker. Say them in Russian; the English text is an explanation, not a tested command.

- Start: **"Алиса, открой чат"** ("Alice, open the chat"), then speak to the Dot.
- End: **"Алиса, закрой чат"** ("Alice, close the chat").

If the browser session expires, restore the login according to the upstream client's documentation. Do not send cookie or token values to chats, issues, or logs.

## Credits and upstream projects

- [Alisa-ChatGPT-voice-assistant](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant): the client this adaptation builds on, at commit `2caf3b40635d0bacad1f42b2bb54b326e9c007d8`.
- [lite-kit](https://github.com/krevetkqqq/lite-kit): a setup toolkit for compatible devices. Our results concern the Lite 1 and the firmware specified here; they do not verify every claim made by the toolkit.
- The patch includes context from upstream files. GitHub did not show an upstream project license when checked; respect the authors' rights and the dependency licenses listed upstream.

This project is not officially affiliated with Yandex or OpenAI.
