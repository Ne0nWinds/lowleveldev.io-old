clang \
--target=wasm32 \
-nostdlib \
-Wl,--no-entry,--export-all,--allow-undefined \
-o binary.wasm \
src/main.c
