# Changelog

## Unreleased

- Organized Dot Connect as an integration catalog, with the Yandex Station route documented as experimental and additional routes open to contributions.
- Added the `dot-connect-init` setup skill, integration proposal workflow, contribution and security guidance, issue forms, and PR template.
- Added a telephone-style ringback while connecting. Its signal and ARM build were checked, and the user confirmed hearing it followed by a Dot reply. Reboot with this version remains unverified.
- Added a shared classic/gentle/silent sound pool, CC0 recordings of a real telephone handset, and custom per-event recordings. The user confirmed the recorded handset cues; alternative presets and overrides have offline tests only.
- Added optional local English stop detection for the Yandex route. Synthetic recognition and two automatic stop/reopen cycles passed; the user confirmed real-microphone stop and reopening on the tested device. Other integrations are not affected.
- The setup retains the original Dot voice experiment and reversible client backup. New wake words are not implemented.

## Initial experiment

- Connected a specific ChatGPT Dot directly from one Yandex Station Lite 1 using a patched Alisa Voice client and the owner's web session.
- The user confirmed voice responses and operation after reboot. The route and root limitations are documented in the Yandex Station integration guide.
