clang -O3 --target=wasm32 -msimd128 -nostdlib "-Wl,--no-entry,--allow-undefined-file=imports.sym" -o binary.wasm src/main.c src/tokenize.c src/standard_functions.c src/codegen.c
