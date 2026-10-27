// RUN: rm -rf %t && mkdir -p %t/include/c++/v1 %t/include/wasm32-linux-musl/c++/v1
// RUN: %clangxx -### --target=wasm32-unknown-linux-musl --sysroot=%t %s 2>&1 \
// RUN:   | FileCheck -DSYSROOT=%t --check-prefix=LIBCXX %s
// RUN: %clangxx -### --target=wasm32-unknown-linux-musl --sysroot=%t \
// RUN:   -stdlib=libstdc++ %s 2>&1 | FileCheck --check-prefix=LIBSTDCXX %s

// wasm Linux uses libc++, as clang does on wasi and the unknown OS.

// LIBCXX: "-cc1"
// LIBCXX-SAME: "-internal-isystem" "[[SYSROOT]]/include/wasm32-linux-musl/c++/v1"
// LIBCXX-SAME: "-internal-isystem" "[[SYSROOT]]/include/c++/v1"
// LIBCXX: wasm-ld{{.*}}" "-lc++" "-lc++abi"
// LIBCXX-NOT: -lstdc++

// LIBSTDCXX: wasm-ld{{.*}}" "-lstdc++"
