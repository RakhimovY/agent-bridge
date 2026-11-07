# Integration guide template

Copy this template to `integrations/<route-slug>/README.md` for a proposed connection. Replace each prompt with concrete information; mark unknown or untested behavior explicitly. Do not put private identifiers, credentials, or user data in the guide.

## Agent and target

- **Agent or agent service:**
- **Device or service:**
- **Status:** planned, experimental, or available
- **Tested versions and scope:**
- **Interaction:** voice, text, events, or other
- **Connection path:** local, Bluetooth, network, or other

## Prerequisites

List accounts, hardware, operating systems, firmware, permissions, dependencies, costs, and which components must stay online. State whether the connection uses an official interface and which subscriptions or API charges apply.

## Setup

Provide ordered steps to prepare the target, authenticate safely, select the agent, configure the connection, and start it. Use placeholders for private values. Describe any device or network changes and preserve existing settings.

## Use and verification

Explain how to initiate and stop a session, exchange input and output, and verify the intended agent and target. Include applicable sensor controls and restart behavior. Give reproducible tests, observed results, versions, and limitations.

## Troubleshooting and recovery

Describe likely failures, how to identify them without exposing private information, and exact steps to stop, undo changes, and restore the previous configuration.

## Privacy, security, and licenses

Explain account/session storage, sensor and network access, update implications, cost, and relevant third-party licenses. Link to authoritative sources and clearly identify undocumented interfaces.
