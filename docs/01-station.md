# 1. Prepare the speaker

## Our device

Device label: Yandex Station Lite 1, `YNDX-00025`, platform `yandexmicro`. Initial firmware: `0.373.1.16.4829676639.20261104.301`. Working firmware after downgrade: `0.369.1.46.4778262442.20261013.297`. Model and version matter: other devices or firmware may behave differently.

The speaker needs normal internet access over Wi-Fi. Bluetooth had previously been paired for ordinary use, but was not needed for modification or Dot calls. USB-C was not used for data transfer in this experiment.

## General root path, not a universal recipe

1. Read [lite-kit](https://github.com/krevetkqqq/lite-kit) and identify your model and firmware. Root access is not included in the voice client's installer.
2. Record the router's current settings and save backups before making changes. The temporary setup needs a computer, network access to the speaker, and administrator privileges. The toolkit changes DNS and responses used by the speaker during setup: do this only on your own network and, where possible, only for the device. Changing DNS for the entire network can affect other devices.
3. On the initial, newer firmware, an attempt to reach the local toolkit through DNS and HTTPS redirection did not downgrade the device. The speaker was later moved to the specified 0.369 firmware and root was obtained. The available records do not fully reconstruct that successful transition; do not treat the initial attempt as a tested recipe for 0.373.
4. Follow the toolkit's firmware and ADB instructions only if the version and method match your model. This repository does not distribute firmware or certificates from the local toolkit. Verify sources and checksums; do not use unverified random mirrors.
5. After obtaining access, configure SSH with a dedicated key, keep backups of system files outside the project, and verify SSH and Alice. In our case, Dropbear runs on port `2222` with password login disabled and key authentication enabled. Do not publish private or host keys.
6. Close temporary unauthenticated ADB on port `5555`, stop the toolkit, restore the router's DNS, and verify internet access on the speaker and other devices. In our case ADB stopped listening and the temporary DNS entry was removed. Do not leave DNS dependent on a laptop that may be switched off.

## Changes on our speaker

- The root filesystem is mounted `ro` after setup; user components are stored under `/data`.
- `/etc/init.d/S99zdropbear` starts Dropbear with its host key and the `-s` flag; SSH port `2222` listens on the local network. Restrict access with firewall rules or client isolation where possible.
- `/system/vendor/quasar/device_ota_update.sh` was replaced with a stub that blocks OTA application; the original was backed up locally. Restoring updates and root depends on device state; we have not verified an automated factory restoration.
- `/system/vendor/quasar/ota_script.pub` also differs from the saved original. Its exact role and the time of change were not established from the available records. It is related to update trust: do not copy it between devices, and account for it during recovery.
- Backups of original system files are kept outside the code and may contain identifiers and sensitive data. Do not publish them.

> If you have newer firmware, an unknown certificate, no backup, or cannot obtain root, stop and investigate compatibility. Do not overwrite unfamiliar partitions based on instructions for another model.
