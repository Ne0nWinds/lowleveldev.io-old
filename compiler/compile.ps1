param([bool]$debug=$true)

if (!(Test-Path -Path build)) { mkdir build }
pushd
cd build

if ($debug) {
clang -O0 -D_DEBUG --target=wasm32 -msimd128 -nostdlib `
"-Wl,--no-entry,--allow-undefined-file=../imports.sym" `
-Wno-incompatible-library-redeclaration -Wno-switch `
-o binary.wasm `
../src/main.c ../src/tokenize.c ../src/standard_functions.c ../src/codegen.c
} else {
clang -Ofast --target=wasm32 -msimd128 -nostdlib `
"-Wl,--no-entry,--allow-undefined-file=../imports.sym" `
-Wno-incompatible-library-redeclaration -Wno-switch `
-o binary.wasm `
../src/main.c ../src/tokenize.c ../src/standard_functions.c ../src/codegen.c
}

popd
