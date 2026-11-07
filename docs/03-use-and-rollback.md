# 3. Подключение Dot и откат

## Подготовка

Укажите адрес станции и путь к **своему** закрытому SSH-ключу локально, не записывая их в репозиторий. Следующие команды относятся к проверенной схеме установки в `/data/opt/alisa-standalone`; для других установок пути могут отличаться. Проверьте, что разговор завершён. Требуются свободное место и уже настроенный вход ChatGPT на станции.

```sh
export STATION_HOST='YOUR_STATION_ADDRESS'
export STATION_KEY="$HOME/.ssh/YOUR_PRIVATE_KEY"
export SSH_PORT=2222
export SSH_TARGET="root@$STATION_HOST"
export BUILT_CLIENT="$PWD/engines/native/build/client/alisa-standalone"
```

Для безопасной замены создайте закрытую резервную папку на станции. Не перезаписывайте существующую папку, если в ней уже есть бэкап.

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'umask 077; test ! -e /data/dot-experiment-backup && mkdir /data/dot-experiment-backup && cp /data/opt/alisa-standalone/bin/alisa-standalone /data/dot-experiment-backup/client && cp /data/opt/alisa-standalone/runtime.env /data/dot-experiment-backup/runtime.env'
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" '/data/opt/alisa-standalone/run.sh stop'
scp -P "$SSH_PORT" -i "$STATION_KEY" "$BUILT_CLIENT" "$SSH_TARGET:/data/dot-experiment-backup/client.new"
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" \
  'chmod 700 /data/dot-experiment-backup/client.new && cp /data/dot-experiment-backup/client.new /data/opt/alisa-standalone/bin/alisa-standalone.new && mv /data/opt/alisa-standalone/bin/alisa-standalone.new /data/opt/alisa-standalone/bin/alisa-standalone'
```

Найдите ID своего Dot в адресе его страницы: часть после `/dots/`, содержащую две части через `~`. Это идентификатор конкретного Dot: не публикуйте свой реальный адрес. Установите `ALISA_DOT_ID` на самой станции в `/data/opt/alisa-standalone/runtime.env` в формате ниже, заменив значение своим. Не удаляйте имеющиеся строки. Пример является шаблоном, не действительным ID.

```sh
export ALISA_DOT_ID='YOUR_ACCOUNT_ID~YOUR_DOT_ID'
```

Запустите и проверьте состояние:

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" '/data/opt/alisa-standalone/run.sh start'
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" '/data/opt/alisa-standalone/run.sh status'
```

Дождитесь `agent_connected: true` и фазы `idle`. Скажите «Алиса, открой чат», дождитесь сигнала, поговорите с Dot и скажите «Алиса, закрой чат». Проверьте историю звонков в самом Dot. Только после этого проверьте автозапуск через обычную перезагрузку станции. Компьютер в дальнейшем не нужен при условии рабочего Wi-Fi и действующей сессии.

`run.sh status` и журналы могут раскрывать сведения об аккаунте и разговорах: не публикуйте их целиком. Не используйте диагностику, предназначенную для основного ChatGPT, как доказательство работы Dot: в ходе нашего теста её текстовая команда закрывала звонок.

## Откат к прежнему клиенту

Если экспериментальный режим не работает, завершите разговор. Восстановите именно свою резервную копию (пути ниже соответствуют командам выше):

```sh
ssh -i "$STATION_KEY" -p "$SSH_PORT" "$SSH_TARGET" 'set -e; /data/opt/alisa-standalone/run.sh stop; cp /data/dot-experiment-backup/client /data/opt/alisa-standalone/bin/alisa-standalone.new; chmod 700 /data/opt/alisa-standalone/bin/alisa-standalone.new; mv /data/opt/alisa-standalone/bin/alisa-standalone.new /data/opt/alisa-standalone/bin/alisa-standalone; cp /data/dot-experiment-backup/runtime.env /data/opt/alisa-standalone/runtime.env; /data/opt/alisa-standalone/run.sh start'
```

Это возвращает клиент и его настройки, но **не** восстанавливает заводскую прошивку, OTA, DNS или root. Не удаляйте исходные резервные копии и ключи, пока не проверили восстановление.
