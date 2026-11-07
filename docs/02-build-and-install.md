# 2. Клиент и сборка

Установка подразумевает уже работающие root и SSH. Сначала установите и проверьте штатный нативный режим [Alisa Voice](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant) согласно документации его версии: авторизация, звук, запуск и остановка, автозапуск. Его установщик описывает Linux и Windows; в нашем эксперименте работа с macOS потребовала ручных шагов. Эта инструкция не обещает автоматическую установку на Mac.

У нас базовый клиент `0.3.7` был установлен в `/data/opt/alisa-standalone`. Сессия хранится в `state/web-session.json` с правами `600`; каталог состояния закрыт для остальных пользователей. Это действующий доступ к аккаунту: используйте только на своём доверенном устройстве. Никакие значения сессии не нужны в репозитории.

## Подготовить исходники

На компьютере установите Git, Python 3, CMake, Ninja, компилятор и Zig 0.14.1 для своей платформы. Следующий пример показывает сборку на macOS с Apple Silicon и обычным Homebrew. Исходники и версии зависят от upstream.

```sh
git clone https://github.com/rison1337/Alisa-ChatGPT-voice-assistant.git alisa-voice
cd alisa-voice
git checkout 2caf3b40635d0bacad1f42b2bb54b326e9c007d8
# Путь к своей локальной копии данного репозитория:
git apply --check /PATH/TO/yandex-station-dot/patches/alisa-voice-0.3.7-dot.patch
git apply /PATH/TO/yandex-station-dot/patches/alisa-voice-0.3.7-dot.patch
```

Скачайте `alisa-voice-sources.tar.gz` и файл `.sha256` из [релиза upstream v0.2.0](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/releases/tag/v0.2.0), проверьте контрольную сумму и распакуйте только зависимости `engines/native/research/upstream`. Точные команды и история зависимостей: [upstream development.md](https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/blob/2caf3b40635d0bacad1f42b2bb54b326e9c007d8/docs/development.md#исходники-зависимостей). Не распаковывайте архив поверх чужого проекта без проверки его содержимого.

```sh
curl -fLO https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/releases/download/v0.2.0/alisa-voice-sources.tar.gz
curl -fLO https://github.com/rison1337/Alisa-ChatGPT-voice-assistant/releases/download/v0.2.0/alisa-voice-sources.tar.gz.sha256
shasum -a 256 -c alisa-voice-sources.tar.gz.sha256
tar -xzf alisa-voice-sources.tar.gz --strip-components=1 alisa-voice-sources/engines/native/research/upstream
```

На macOS мы использовали официальный архив Zig `0.14.1` для `aarch64-macos`; проверенный SHA-256 архива: `39f3dc5e79c22088ce878edc821dedb4ca5a1cd9f5ef915e9b3cc3053e8faefa`. Перед использованием проверьте текущие источники и контрольные суммы Zig. Патч добавляет переменную `DOT_ZIG` и обёртки `ar`/`ranlib` для сборки с Mac.

```sh
export DOT_ZIG=/PATH/TO/zig
python3 engines/native/scripts/build_native.py --jobs 4
cmake -S engines/native -B engines/native/build/client -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/engines/native/build/arm/toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DALISA_USE_NETEQ=ON
cmake --build engines/native/build/client --target alisa-standalone --parallel 4
file engines/native/build/client/alisa-standalone
```

Для нашего устройства результат: 32-битный ARM ELF, hard-float (`arm-linux-gnueabihf`), не macOS-приложение. Исходный архив, бинарники, логи сборки и локальные сессии в Git не добавлять.

## Установка экспериментального бинарника

В этой части нет универсального безопасного скрипта: пути, способ SSH, свободное место и бэкапы зависят от вашей установки. На нашей станции при неактивном разговоре мы остановили `run.sh`, сохранили прежний бинарник и `runtime.env` в закрытой отдельной папке, заменили бинарник на собранный, добавили ID Dot в `runtime.env` и снова запустили `run.sh`. Перед заменой убедитесь, что восстановление именно вашей конфигурации возможно.

На нашей станции пути:

```text
/data/opt/alisa-standalone/bin/alisa-standalone
/data/opt/alisa-standalone/runtime.env
/data/opt/alisa-standalone/state/web-session.json
/data/opt/alisa-standalone/run.sh
```

Фактические команды резервирования, копирования и запуска приведены в [следующем разделе](03-use-and-rollback.md) с переменными вместо адресов и идентификаторов. Не выполняйте команды до проверки путей и возможности вернуть прежний клиент.
