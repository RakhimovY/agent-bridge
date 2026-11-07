# Dot Connect

Connect a ChatGPT Dot to devices and services. Dot Connect is a community project for documenting, testing, and improving those connections.

Today there is one documented route: call a Dot from a **Yandex Station Lite 1**, using the speaker itself rather than a continuously running computer. A user heard a voice reply before and after reboot. Other devices and services are future integrations, not supported features yet.

## Choose an integration

| Integration | Status | Start here |
| --- | --- | --- |
| Yandex Station | Experimental; tested on one Lite 1 and firmware | [Yandex Station guide](integrations/yandex-station/README.md) |

[Integration index](integrations/README.md) | [Call sounds](docs/sounds.md) | [Add an integration](docs/adding-integrations.md) | [Contribute](CONTRIBUTING.md) | [Changelog](CHANGELOG.md)

## Get started

Clone this repository and follow the integration guide. If your agent supports skills, use [dot-connect-init](.agents/skills/dot-connect-init/SKILL.md): it follows the available route, asks for your desired start and stop phrases, and guides the setup. When there is only one available route, it does not ask you to choose an integration. It asks you to choose once multiple routes are available and your target is unclear.

For the current Yandex route, Alice starts the call. The tested Russian commands and English explanations are in its guide. An optional local English stop detector passed synthetic and automated device tests; the user confirmed real-microphone stop and reopening on the tested device. A new wake word such as "Hey [name]" is not implemented.

## Current limitations

- The current connection uses an undocumented ChatGPT web interface and a sensitive stored web session, not an official Dot SDK or API key. Interface behavior, session lifetime, and subscription limits may change.
- The current device path requires root and SSH. Rooting or downgrading may erase settings or make the speaker unusable. Our test device has OTA updates disabled and consequently receives no automatic security updates.
- Hardware mute in this mode, other models, long-term stability, and factory recovery have not been verified. The successful root transition has not been reconstructed in full.
- The public project is an experimental integration patch and guides, not a universal installer or a finished cross-platform library.
- Do not publish account sessions, tokens, private keys, real Dot IDs, device dumps, firmware, Wi-Fi credentials, or logs. See [security](docs/05-security.md).

## Community

Issues and pull requests are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) and [the integration guide](docs/adding-integrations.md) for the path from proposal to tested support. Be considerate: [Code of Conduct](CODE_OF_CONDUCT.md). Report security concerns using [SECURITY.md](SECURITY.md).

Original Dot Connect material is offered under the [MIT License](LICENSE). Recorded handset sounds retain their CC0 license. The Yandex Station patch includes context from the independently authored upstream client, whose GitHub project did not declare a license when checked. The MIT license here does not grant rights to that upstream material; see [NOTICE](NOTICE.md) and the upstream project before redistribution.

Dot Connect is not officially affiliated with Yandex or OpenAI.
