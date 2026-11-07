# Contributing

Issues, documentation improvements, fixes, and new agent-to-device or agent-to-service integrations are welcome. You do not need write access to this repository: fork it, create a branch in your fork, push there, and open a pull request against `main`.

1. For a substantial change, open an issue first so scope and compatibility are clear. For a new connection, follow [Adding an integration](docs/adding-integrations.md) and use the [guide template](docs/integration-template.md).
2. Clone your fork, create a branch, install [Gitleaks](https://github.com/gitleaks/gitleaks), and run `./scripts/setup-hooks.sh`. The hooks must be enabled in each clone. Never commit credentials, session data, device identifiers, private logs, or firmware.
3. Make the smallest useful change. Keep route-specific dependencies and checks scoped to that route, preserve recovery paths, respect upstream licenses, and update the route guide and catalog.
4. Verify the paths you changed. State the agent, target, versions, steps, results, limitations, and rollback in the PR. Label untested behavior. Do not include sensitive output in an issue or PR.
5. Open a PR from your fork. The repository owner reviews and approves contributions and controls merges to `main`; successful CI alone is not approval.

See [SECURITY.md](SECURITY.md) for private reporting. By contributing original material, you agree it is made available under the project's MIT license. You must have the right to submit any third-party material and disclose its license; the license does not override upstream rights described in [NOTICE.md](NOTICE.md).
