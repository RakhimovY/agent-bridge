# Integration catalog

An integration is a documented connection between an agent and a device or service. A route can support voice, text, or another interaction. List the exact combination and evidence rather than claiming that an entire provider or device family works.

| Agent | Device or service | Status | Verified scope | Guide |
| --- | --- | --- | --- | --- |
| ChatGPT Dot | Yandex Station Lite 1 | Available, experimental | One `YNDX-00025` (`yandexmicro`) on firmware `0.369.1.46.4778262442.20261013.297` | [Guide](yandex-station/README.md) |

**Available** means that a user can follow the documented route and its claimed behavior has been tested in the stated scope. **Experimental** means that the route has significant limitations or may change. **Planned** and **proposed** ideas are not selectable setup options.

Wearables, other speakers, services, and other agents are opportunities for contributions, not supported routes yet. See [Adding an integration](../docs/adding-integrations.md). The setup skill chooses the only matching available route automatically; when multiple routes match and the target is unclear, it asks the user.
