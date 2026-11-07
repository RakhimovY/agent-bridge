#!/bin/sh
# Build an isolated ARMv7 stop detector. Does not access a speaker or account.
set -eu
: "${DOT_ZIG:?Set DOT_ZIG to Zig 0.14.1 executable}"
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build=${DOT_STOP_BUILD_DIR:-"$here/.build"}
mkdir -p "$build"
build=$(CDPATH= cd -- "$build" && pwd)
verify() {
  python3 - "$1" "$2" <<'PY'
import hashlib,sys
with open(sys.argv[1],'rb') as f: actual=hashlib.sha256(f.read()).hexdigest()
if actual!=sys.argv[2]: raise SystemExit('Checksum mismatch: '+sys.argv[1])
PY
}
fetch() { curl -fsSL "$1" -o "$2"; verify "$2" "$3"; }
fetch https://files.pythonhosted.org/packages/39/46/328092b4df890385594cf3d3e6015da72d77f63c58d3057276ac2353e891/pymicro_features-2.0.2.tar.gz "$build/features.tar.gz" 0d0bed7843ec78b6ced82d1a2dcddeb4fe5df61b3af80a281d0868c8e279c727
fetch https://raw.githubusercontent.com/OHF-Voice/pymicro-wakeword/333da550c54b95192af472cfe37d98dab2f2086e/lib/linux_armv7l/libtensorflowlite_c.so "$build/libtensorflowlite_c.so" fcb6ef6f3f9c68889868fb55aba544d43d0fe9703c1efeb2756e82b553d032ab
fetch https://raw.githubusercontent.com/OHF-Voice/linux-voice-assistant/f58360d207b8af5a4b49d78b118598add3bae81d/wakewords/stop.tflite "$build/stop.tflite" 020ef80d522cb09169a866f3aeeb58f2ad4045461937e78e8c806df29ff61eea
fetch 'https://deb.debian.org/debian/pool/main/g/glibc/libc6_2.36-9+deb12u14_armhf.deb' "$build/libc6.deb" 758c68b92654747025b48476a45c315bf16df1321224abdd9be672bfd120be45
fetch 'https://deb.debian.org/debian/pool/main/g/gcc-12/libstdc++6_12.2.0-14+deb12u1_armhf.deb' "$build/libstdcpp.deb" 66d53ddeb10a37f22e455611c2c515f9d444e523b55d873ff48bde47267963db
mkdir -p "$build/features" "$build/libc-package" "$build/cxx-package" "$build/stop/runtime"
tar -xzf "$build/features.tar.gz" -C "$build/features" --strip-components=1
(cd "$build/libc-package" && ar x "$build/libc6.deb")
(cd "$build/cxx-package" && ar x "$build/libstdcpp.deb")
tar -xJf "$build/libc-package/data.tar.xz" -C "$build/stop/runtime" --strip-components=3 \
  ./lib/arm-linux-gnueabihf/ld-linux-armhf.so.3 ./lib/arm-linux-gnueabihf/libc.so.6 \
  ./lib/arm-linux-gnueabihf/libdl.so.2 ./lib/arm-linux-gnueabihf/libm.so.6 \
  ./lib/arm-linux-gnueabihf/libpthread.so.0 ./lib/arm-linux-gnueabihf/librt.so.1
tar -xJf "$build/cxx-package/data.tar.xz" -C "$build/stop/runtime" --strip-components=4 \
  ./usr/lib/arm-linux-gnueabihf/libstdc++.so.6.0.30 ./usr/lib/arm-linux-gnueabihf/libstdc++.so.6
cp "$build/libtensorflowlite_c.so" "$build/stop/"
cp "$build/stop.tflite" "$build/stop/"
front="$build/features/tensorflow/lite/experimental/microfrontend/lib"
"$DOT_ZIG" c++ -target arm-linux-gnueabihf.2.28 -mcpu=cortex_a53 -std=c++17 -Os -DFIXED_POINT=16 \
  -I"$build/features" -I"$build/features/kissfft" "$here/stop-detector.cpp" \
  "$front/kiss_fft_int16.cc" "$front/fft.cc" "$front/fft_util.cc" \
  "$front/filterbank.cc" "$front/filterbank_util.cc" "$front/frontend.cc" "$front/frontend_util.cc" \
  "$front/log_lut.cc" "$front/log_scale.cc" "$front/log_scale_util.cc" \
  "$front/noise_reduction.cc" "$front/noise_reduction_util.cc" \
  "$front/pcan_gain_control.cc" "$front/pcan_gain_control_util.cc" \
  "$front/window.cc" "$front/window_util.cc" "$build/features/kissfft/kiss_fft.cc" \
  "$build/features/kissfft/tools/kiss_fftr.cc" -ldl -o "$build/stop/dot-stop-detector"
printf 'Built private runtime in %s/stop\n' "$build"
