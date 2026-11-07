---
name: agent-bridge-init
description: Use when setting up a supported Agent Bridge connection between an AI agent and a device or service.
---

# Agent Bridge setup

Guide the owner through a reversible setup on their own device or service. Handle safe steps yourself; let them perform physical interactions and enter credentials privately. Never request or print passwords, cookies, tokens, private keys, private URLs, device dumps, or network captures.

1. Locate the Agent Bridge repository containing this skill. If unavailable, clone `https://github.com/RakhimovY/agent-bridge` into the owner's chosen workspace. Read `README.md`, `integrations/README.md`, and repository instructions; check Git status.
2. Match **Available** catalog entries against the requested agent and device or service. If none matches, explain that the combination is not yet supported and point to `docs/adding-integrations.md`; do not substitute a different route. If one matches, select it without an unnecessary choice. If several match and the user's target is unclear, ask which one they want. Ask for missing agent or target information when needed. Never present planned routes as available.
3. Read the selected route's guide. Ask only for details relevant to it, such as versions, access, account, target, interaction, commands, or sounds. Offer only options implemented by that route. Keep account identifiers and credentials private; explain any setup risks and costs the guide identifies.
4. Preserve the existing configuration, working access, account state, and a concrete rollback. Follow the route guide and the owner's instructions. Do not invent missing root, firmware, authentication, network, or recovery steps.
5. Verify the route's documented setup and relevant behavior, including startup, input and output, stopping, recovery, and reboot where applicable. Report the exact tested scope, remaining uncertainties, and recovery steps. Do not assume other agents, devices, or versions work.

For a new connection, follow `docs/adding-integrations.md`. For repository changes, enable `./scripts/setup-hooks.sh`, scan with Gitleaks, and keep private information out of the repository.
