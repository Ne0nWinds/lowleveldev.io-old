clang \
-O3 \
--target=wasm32 \
-nostdlib \
-Wl,--no-entry,--allow-undefined-file=imports.sym \
-o binary.wasm \
src/main.c
