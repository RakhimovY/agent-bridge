# Contributing

Issues, documentation improvements, fixes, and new integration proposals are welcome. You do not need write access to the main repository: fork it, create a branch in your fork, push there, and open a pull request against `main`.

1. For a substantial change, open an issue first so scope, compatibility, and ownership are clear. For a new device or service, follow [Adding an integration](docs/adding-integrations.md).
2. Clone your fork, create a branch, install [Gitleaks](https://github.com/gitleaks/gitleaks), and run `./scripts/setup-hooks.sh`. The hooks must be enabled in each clone. Never commit credentials, session data, device identifiers, private logs, or firmware.
3. Make the smallest useful change. Keep device changes reversible, respect upstream licenses, and update the relevant integration guide and tests.
4. Verify the paths you changed. State actual device, firmware, tools, commands, results, limitations, and rollback in the PR. Mark untested claims explicitly. Do not include sensitive output in an issue or PR.
5. Push your branch and open a PR. Maintainers review changes and merge to `main`; successful CI alone is not an approval. Ask before making changes that affect someone else's account, device, security, or paid resources.

See [SECURITY.md](SECURITY.md) for private reporting. By contributing original material, you agree it is made available under the project's MIT license. You must have the right to submit any third-party material and disclose its license; the license does not override upstream rights described in [NOTICE.md](NOTICE.md).
