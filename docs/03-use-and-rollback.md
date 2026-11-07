# 3. Connect your Dot and roll back

## Preparation

Set the speaker's address and the path to **your own** private SSH key locally, without writing them into the repository. These commands apply to the tested installation under `/data/opt/alisa-standalone`; paths may differ elsewhere. Make sure no call is in progress. You need free space and an existing ChatGPT login on the speaker.

```sh
export STATION_HOST='YOUR_STATION_ADDRESS'
export STATION_KEY="$HOME/.ssh/YOUR_PRIVATE_KEY"
export SSH_PORT=2222
export SSH_TARGET="root@$STATION_HOST"
export BUILT_CLIENT="$PWD/engines/native/build/client/alisa-standalone"
```

Create a private backup directory on the speaker before replacement. Do not overwrite an existing backup directory.

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; test ! -e /data/dot-experiment-backup && mkdir /data/dot-experiment-backup && cp /data/opt/alisa-standalone/bin/alisa-standalone /data/dot-experiment-backup/client && cp /data/opt/alisa-standalone/runtime.env /data/dot-experiment-backup/runtime.env'
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" '/data/opt/alisa-standalone/run.sh stop'
scp -P "$SSH_PORT" -i "$STATION_KEY" "$BUILT_CLIENT" "$SSH_TARGET:/data/dot-experiment-backup/client.new"
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'chmod 700 /data/dot-experiment-backup/client.new && cp /data/dot-experiment-backup/client.new /data/opt/alisa-standalone/bin/alisa-standalone.new && mv /data/opt/alisa-standalone/bin/alisa-standalone.new /data/opt/alisa-standalone/bin/alisa-standalone'
```

Find your Dot ID in its page URL: the part after `/dots/`, consisting of two parts separated by `~`. It identifies your specific Dot; do not publish your actual URL. Set `ALISA_DOT_ID` on the speaker in `/data/opt/alisa-standalone/runtime.env` using the format below and replacing the value with your own. Keep existing lines. This is a placeholder, not a working ID.

```sh
export ALISA_DOT_ID='YOUR_ACCOUNT_ID~YOUR_DOT_ID'
```

Start the client and check its status:

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" '/data/opt/alisa-standalone/run.sh start'
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" '/data/opt/alisa-standalone/run.sh status'
```

Wait for `agent_connected: true` and phase `idle`. Say **"Алиса, открой чат"** ("Alice, open the chat"), wait for the signal, speak to the Dot, then say **"Алиса, закрой чат"** ("Alice, close the chat"). These are the exact Russian phrases configured on our speaker; the English translations are explanatory. Check the Dot's call history. Only then test autostart with a normal speaker reboot. A computer is not needed afterwards as long as Wi-Fi and the session work.

For optional bare English "stop" on the tested route, build and install the [local stop detector](../integrations/yandex-station/stop-detector/README.md). The user confirmed stop and reopening on one device; other devices and long-term false-trigger rates remain unverified.

`run.sh status` and logs may reveal account and conversation information: do not publish them in full. Do not use diagnostics intended for the primary ChatGPT assistant as proof that Dot works: in our test, its text command closed the call.

## Restore the previous client

If experimental mode fails, end the call. Restore **your own** backup (these paths correspond to the commands above):

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" 'set -e; /data/opt/alisa-standalone/run.sh stop; cp /data/dot-experiment-backup/client /data/opt/alisa-standalone/bin/alisa-standalone.new; chmod 700 /data/opt/alisa-standalone/bin/alisa-standalone.new; mv /data/opt/alisa-standalone/bin/alisa-standalone.new /data/opt/alisa-standalone/bin/alisa-standalone; cp /data/dot-experiment-backup/runtime.env /data/opt/alisa-standalone/runtime.env; /data/opt/alisa-standalone/run.sh start'
```

This restores the client and its settings, but **does not** restore factory firmware, OTA, DNS, or root. Keep original backups and keys until you have verified recovery.
