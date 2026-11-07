# Adding an integration

Dot Connect is currently a guide and patch for one route. The lifecycle below is a contribution contract, not an existing adapter API. Build only what a real second integration requires; do not mark an idea available before it works.

## 1. Propose the route

Open an integration proposal describing the device or service, owner-controlled access, intended Dot interaction, required additional hardware, operating system and firmware, network path, expected cost, and the steps a user must perform. State whether the route is voice, text, or both. Distinguish official APIs from undocumented interfaces. Name what is unknown and any terms or license restrictions. Do not include real credentials, private URLs, account IDs, logs, or device dumps.

## 2. Define capabilities and recovery

Describe the setup prerequisites and the connection lifecycle: prepare, authenticate, initiate, exchange input and output, stop, and clean up. Specify which component runs continuously, how wake/start/stop work, what happens when authentication or connectivity fails, and whether the user can use their existing subscription. Preserve pre-existing settings and provide a concrete rollback. Explain microphone, wake-word, privacy, update, network, and credential implications where relevant.

## 3. Implement and document

Place integration-specific code and its guide under `integrations/<slug>/`, keeping shared behavior small and based on actual common needs. A guide should include supported devices and versions, dependencies, ordered setup, how to choose the target Dot, the exact tested start/stop commands, safe configuration, verification, troubleshooting, rollback, source attribution, and known limits. Use placeholders for private values. Do not copy another project's source, binary, firmware, or model unless its license permits the use and attribution is preserved.

## 4. Verify and submit

Run meaningful checks for the changed behavior and the repo's secret scans. Test on the claimed device or service when available: setup, authentication, target identity, data exchange, stop, restart/reconnect, failure handling, and rollback. For voice, test microphone control and whether a second computer is required. If something has not been tested, label it explicitly and narrow the support claim. Open a PR explaining the change, evidence, limitations, and recovery; maintainers review it.

## 5. Make the route discoverable

Once reviewed and supported, add one **Available** row to `integrations/README.md`. The setup skill uses that index: it selects the only available route automatically, or asks the user when multiple routes exist and the target is unclear. Keep planned or partial work marked **Planned** or **Experimental / not yet available** until it meets the documented scope. Update README links and credit contributors.
