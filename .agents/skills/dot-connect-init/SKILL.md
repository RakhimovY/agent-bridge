---
name: dot-connect-init
description: Use when setting up a ChatGPT Dot connection to a device or service using a supported Dot Connect integration.
---

# Dot Connect setup

Guide the owner through a reversible setup on their own device or service. Do all safe steps available to you; have them handle physical interactions and credentials. Never ask for or print passwords, cookies, tokens, private keys, network dumps, or a private Dot URL. Do not put them into the repository or issue tracker.

1. Locate the Dot Connect repository containing this skill. If unavailable locally, clone `https://github.com/RakhimovY/dot-connect` into the owner's chosen workspace. Read `README.md`, `integrations/README.md`, and repository instructions; check Git status.
2. Count integration routes marked as available in `integrations/README.md`. If exactly one is available, use it without asking which integration. If multiple routes are available and the owner's target is not already clear, ask which device or service they want to connect. Never present planned or untested routes as supported. After selecting a route, read its guide and relevant documentation.
3. Ask for the model, firmware, existing root/SSH and client status where relevant, the target Dot, preferred start phrase, and sound preset. Offer `classic`, `gentle`, `silent`, and custom audio as supported by the route. For the Yandex route ask whether to use the stock Alice command or try the experimental local wake phrase; follow `integrations/yandex-station/wake-word/README.md` for the latter. Explain that recognition is not guaranteed, the available local stop word is English `stop`, and separate Dot and ChatGPT wake targets are not yet supported by the released client. Do not imply other devices or phrases are verified. Keep real IDs and credentials private.
4. Before device, firmware, authentication, or DNS changes, identify existing state and backups, explain the actual risks, and follow the integration documentation and the owner's instructions. Restrict experimental DNS to the device where possible and restore it afterwards. Close unauthenticated ADB and avoid public SSH. Do not invent missing root steps, overwrite unknown device data, expose an account session, or disable security updates without explaining the consequence.
5. Follow the selected route and preserve the working client, settings, SSH, and a verified rollback path. Use preferred phrases only where supported; preserve a known working route while testing. Custom command matching alone does not replace a wake-word engine. Never send secrets in prompts, shared commands, logs, or published files.
6. Verify start, voice reply from the intended Dot, stop, reboot/autostart, and operation without a continuously running computer where applicable. Report what was actually tested, limitations, exact working commands, and recovery. Hardware mute and compatibility on other models are not assumed.

When adding another integration, follow `docs/adding-integrations.md`; implement only supported, verified capabilities. For repository changes, activate `./scripts/setup-hooks.sh`, scan with Gitleaks, and avoid publishing private details. Scanning is not a guarantee.
