#!/bin/bash
# Build the WebAssembly version. Needs emscripten (brew install emscripten).
#
# The desktop build (CMakeLists.txt, FLTK) is untouched by this. The only
# difference is the graphics backend: graphics_web.cpp replaces graphics.cpp,
# both implementing the same graphics.h.
set -e
cd "$(dirname "$0")"

command -v em++ >/dev/null || { echo "emcc not found -- brew install emscripten"; exit 1; }

OUT=docs
mkdir -p "$OUT"

em++ -std=c++17 -O2 \
  -I src -I src/graphics -I src/TuringMachine \
  main_web.cpp \
  src/viz.cpp src/machine.cpp src/tape.cpp src/helper.cpp \
  src/graphics/graphics_web.cpp \
  -s WASM=1 \
  -fexceptions \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s SINGLE_FILE=1 \
  -s MODULARIZE=1 -s EXPORT_NAME=createTuringViz \
  -s ENVIRONMENT=web,node \
  -s FORCE_FILESYSTEM=1 \
  -s EXPORTED_FUNCTIONS='["_main","_tv_load","_tv_load_file","_tv_set_paused","_tv_push_key","_tv_push_mouse"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8","FS"]' \
  -o "$OUT/turingviz.js"

echo "[build] wrote $OUT/turingviz.js (wasm embedded, SINGLE_FILE=1)"
