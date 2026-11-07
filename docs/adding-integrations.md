# Adding an integration

An integration connects a specific AI agent or agent service to a specific device or service. Agent Bridge currently provides one experimental route. This guide is a contribution workflow, not a promise of a generic adapter API or existing support for every combination.

## 1. Propose the connection

Open an integration proposal naming both ends, the intended interaction, and who controls the device and account. Include the operating system or firmware, connection and network path, extra hardware, ongoing runtime requirements, expected costs, and whether it uses an official API or another interface. Distinguish facts from assumptions and identify open questions, terms, and licenses. Never include credentials, private URLs, account IDs, raw logs, or device dumps.

## 2. Design a reversible route

Explain setup, authentication, initiation, input and output, stopping, reconnecting, and recovery. Identify what runs on the target and whether another device or service must remain online. Describe access to microphones or other sensors when applicable, account storage, connectivity, updates, costs, and a rollback that preserves existing settings. Describe only capabilities the proposed agent and target actually provide.

## 3. Add the route

Put a new route under `integrations/<route-slug>/` with a `README.md` based on the [guide template](integration-template.md). Choose a slug that makes the combination clear, such as `watch-agent-example`. The existing Yandex route keeps its current location; don't move working integrations just to fit an example.

Place route-specific scripts, configuration, assets and tests within the route when practical. Keep credentials and generated local artifacts outside Git. Reuse shared material only when its existing contract fits; do not add a speculative framework or require another route's dependencies. Follow the license and attribution requirements of every dependency.

## 4. Verify and submit

Run checks appropriate to your change, including the secret scan. Record the exact agent, target, versions, steps and observed results. Where applicable, verify setup, authentication, data exchange, stopping, restart, failure handling and rollback. For voice, verify microphone controls and whether another computer is required. Mark missing tests and narrow claims to the actual evidence. Never include sensitive output in the PR.

Fork the repository, open a branch and submit a pull request. The repository owner reviews and approves changes before merging. See [Contributing](../CONTRIBUTING.md).

## 5. Add the catalog entry

Add a row to the [integration catalog](../integrations/README.md) with the agent, device or service, status, tested scope, and guide. Mark an unfinished idea **Planned** or **Experimental / not yet available**. Mark a route **Available** only when its documented setup and stated capabilities have been verified. The setup skill selects only available matching routes.
