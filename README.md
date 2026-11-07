# Agent Bridge

Agent Bridge is an open-source collection of guides and integrations for connecting AI agents to devices and services. Contributors can add routes for different agents, speakers, wearables, computers, or other targets, with clear setup, verification, and recovery instructions.

Today, the only documented and tested route is **ChatGPT Dot on Yandex Station Lite 1**. It is experimental and works on the speaker without a continuously running computer. Other agents and devices are welcome as contributions, but are not supported until an integration has been documented and verified.

## Find a connection

| Agent | Device or service | Status | Guide |
| --- | --- | --- | --- |
| ChatGPT Dot | Yandex Station Lite 1 | Available, experimental; tested on one device and firmware | [Yandex Station guide](integrations/yandex-station/README.md) |

See the [integration catalog](integrations/README.md) for exact support and limitations. The [setup skill](.agents/skills/agent-bridge-init/SKILL.md) guides users through an available route; it selects the only matching route automatically and asks when multiple available routes match. It does not invent unsupported setup steps. The former `dot-connect-init` name remains as a compatibility alias.

For Codex or OMP environments that discover user skills in `~/.agents/skills`, make the skill available outside the repository on macOS or Linux by running the following from the cloned repository. If the destination already exists, inspect it before changing it.

```sh
mkdir -p ~/.agents/skills
ln -s "$PWD/.agents/skills/agent-bridge-init" ~/.agents/skills/agent-bridge-init
```

Invoke `agent-bridge-init` in a compatible agent. Skill discovery can require a new session; use your tool's own installation instructions on other platforms.

## Add a connection

Anyone can propose an agent, device, or service combination. Start with [Adding an integration](docs/adding-integrations.md), use the [integration guide template](docs/integration-template.md), and submit a pull request from a fork. Each route owns its documentation, implementation, dependencies, and tests. The existing Yandex route stays intact; shared code should emerge from demonstrated common needs.

[Contributing](CONTRIBUTING.md) | [Security](SECURITY.md) | [Call sounds](docs/sounds.md) | [Changelog](CHANGELOG.md)

## Current limits

- The available route uses an undocumented ChatGPT web interface and a sensitive stored session, not an official Dot API. Interface behavior, session lifetime, and subscription limits may change.
- That route requires root and SSH. Rooting or downgrading may erase settings or make the speaker unusable. Our test device has OTA updates disabled and receives no automatic security updates.
- The physical mute input was checked on one device; live voice suppression while muted, other models, long-term stability, and factory recovery remain unverified.
- Agent Bridge is a collection of experimental integrations, not a universal installer or a promise that every agent and device combination works.
- Never publish sessions, tokens, private keys, real account or device IDs, device dumps, firmware, Wi-Fi credentials, or logs. See [secret safety](docs/05-security.md).

## Community and licensing

Issues and pull requests are welcome. The repository owner reviews contributions and controls merges to `main`. Be considerate: [Code of Conduct](CODE_OF_CONDUCT.md). Report sensitive security concerns privately via [SECURITY.md](SECURITY.md).

Original Agent Bridge material is offered under the [MIT License](LICENSE). Recorded handset sounds retain their CC0 license. The Yandex Station patch includes context from the independently authored upstream client, whose GitHub project did not declare a license when checked. The MIT license does not grant rights to that upstream material; see [NOTICE](NOTICE.md).

Agent Bridge is not officially affiliated with any agent or device provider.
