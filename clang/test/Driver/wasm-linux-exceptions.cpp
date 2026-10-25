// RUN: %clang -### --target=wasm32-unknown-linux-musl --sysroot=/foo -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=LINUX %s
// RUN: %clang -### --target=wasm64-unknown-linux-musl --sysroot=/foo -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=LINUX %s
// RUN: %clang -### --target=wasm32-unknown-linux-musl --sysroot=/foo \
// RUN:   -fwasm-exceptions -c %s 2>&1 | FileCheck --check-prefix=LINUX %s
// RUN: %clang -### --target=wasm32-unknown-linux-musl --sysroot=/foo \
// RUN:   -mno-exception-handling -c %s 2>&1 | FileCheck --check-prefix=NO-EH %s
// RUN: %clang -### --target=wasm32-wasi --sysroot=/foo -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO-EH %s

// LINUX: "-cc1"
// LINUX-SAME: "-target-feature" "+exception-handling"
// LINUX-SAME: "-exception-model=wasm"
// LINUX-SAME: "-mllvm" "-wasm-enable-eh"
// LINUX-NOT: "-wasm-enable-eh"

// NO-EH: "-cc1"
// NO-EH-NOT: "-wasm-enable-eh"
