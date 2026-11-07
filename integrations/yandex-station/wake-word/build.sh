#!/bin/sh
# Build a private Russian wake bundle from the separately obtained dependencies.
set -eu
: "${DOT_ZIG:?Set the path to Zig 0.14.1}"
: "${DOT_WAKE_PYTHON:?Set the Python with sentencepiece, onnx and onnxruntime}"
: "${DOT_WAKE_RU_INT8:?Set the extracted Russian int8 model directory}"
: "${DOT_WAKE_RU_FP32:?Set the extracted Russian float model directory}"
: "${DOT_WAKE_LIBS:?Set the sherpa ARM library directory}"
: "${DOT_WAKE_HEADERS:?Set the parent of sherpa-onnx/c-api/c-api.h}"
: "${DOT_WAKE_RUNTIME:?Set the isolated ARM runtime directory}"
: "${DOT_WAKE_BUILD_DIR:?Set a new private output directory}"
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
umask 077
test ! -e "$DOT_WAKE_BUILD_DIR" || { echo "Build directory already exists" >&2; exit 1; }
mkdir -p "$DOT_WAKE_BUILD_DIR/ru"
for file in bpe.model decoder.onnx joiner.int8.onnx tokens.txt; do
  cp "$DOT_WAKE_RU_INT8/$file" "$DOT_WAKE_BUILD_DIR/ru/$file"
done
"$DOT_WAKE_PYTHON" - "$DOT_WAKE_RU_FP32/encoder.onnx" "$DOT_WAKE_BUILD_DIR/ru/encoder.int4.onnx" <<'PY'
import logging
import sys
from onnxruntime.quantization import matmul_nbits_quantizer
logging.disable(logging.CRITICAL)
model = matmul_nbits_quantizer.MatMulNBitsQuantizer(sys.argv[1], block_size=128, is_symmetric=True)
model.process()
model.model.save_model_to_file(sys.argv[2], False)
PY
"$DOT_ZIG" c++ -target arm-linux-gnueabihf.2.28 -std=c++17 -O2 -s \
  -I"$here/include" -I"$DOT_WAKE_HEADERS" "$here/listener.cpp" \
  -L"$DOT_WAKE_LIBS" -lsherpa-onnx-c-api -o "$DOT_WAKE_BUILD_DIR/listener"
"$DOT_WAKE_PYTHON" "$here/bundle.py" --ru "$DOT_WAKE_BUILD_DIR/ru" \
  --runtime "$DOT_WAKE_RUNTIME" --libs "$DOT_WAKE_LIBS" \
  --listener "$DOT_WAKE_BUILD_DIR/listener" --out "$DOT_WAKE_BUILD_DIR/bundle"
echo "Private bundle created at $DOT_WAKE_BUILD_DIR/bundle"
