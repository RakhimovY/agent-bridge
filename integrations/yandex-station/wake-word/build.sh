#!/bin/sh
# Build the experimental detector without contacting a speaker or account.
set -eu
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build=${DOT_WAKE_BUILD_DIR:-"$here/.build"}
mkdir -p "$build/features"
build=$(CDPATH= cd -- "$build" && pwd)
archive="$build/pymicro-features.tar.gz"
curl -fsSL https://files.pythonhosted.org/packages/39/46/328092b4df890385594cf3d3e6015da72d77f63c58d3057276ac2353e891/pymicro_features-2.0.2.tar.gz -o "$archive"
python3 - "$archive" <<'PY'
import hashlib, sys
with open(sys.argv[1], 'rb') as f:
    digest = hashlib.sha256(f.read()).hexdigest()
if digest != '0d0bed7843ec78b6ced82d1a2dcddeb4fe5df61b3af80a281d0868c8e279c727':
    raise SystemExit('Frontend source checksum mismatch')
PY
tar -xzf "$archive" -C "$build/features" --strip-components=1
front="$build/features/tensorflow/lite/experimental/microfrontend/lib"
if [ "${DOT_WAKE_HOST:-0}" = 1 ]; then
  set -- c++
else
  : "${DOT_ZIG:?Set DOT_ZIG to the Zig 0.14.1 executable}"
  set -- "$DOT_ZIG" c++ -target arm-linux-gnueabihf.2.28 -mcpu=cortex_a53
fi
"$@" -std=c++17 -Os -DFIXED_POINT=16 \
  -I"$build/features" -I"$build/features/kissfft" "$here/wake-detector.cpp" \
  "$front/kiss_fft_int16.cc" "$front/fft.cc" "$front/fft_util.cc" \
  "$front/filterbank.cc" "$front/filterbank_util.cc" "$front/frontend.cc" "$front/frontend_util.cc" \
  "$front/log_lut.cc" "$front/log_scale.cc" "$front/log_scale_util.cc" \
  "$front/noise_reduction.cc" "$front/noise_reduction_util.cc" \
  "$front/pcan_gain_control.cc" "$front/pcan_gain_control_util.cc" \
  "$front/window.cc" "$front/window_util.cc" "$build/features/kissfft/kiss_fft.cc" \
  "$build/features/kissfft/tools/kiss_fftr.cc" -o "$build/wake-detector"
printf 'Built experimental detector: %s/wake-detector\n' "$build"
