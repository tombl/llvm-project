// RUN: %clang_cc1 %s -triple wasm32-unknown-linux-musl -fcxx-exceptions -fexceptions -exception-model=wasm -mllvm -wasm-enable-eh -target-feature +exception-handling -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple wasm64-unknown-linux-musl -fcxx-exceptions -fexceptions -exception-model=wasm -mllvm -wasm-enable-eh -target-feature +exception-handling -emit-llvm -o - | FileCheck %s

// Linux wasm uses the WebAssembly C++ ABI: catch bodies leave through
// catchret, and constructors return `this`.

void may_throw();

struct S {
  S();
};

// CHECK: define {{.*}}ptr @_ZN1SC2Ev(ptr {{.*}}returned
S::S() {}

// CHECK-LABEL: define {{.*}}void @_Z7catchesv()
// CHECK: catchswitch within none [label %catch.start] unwind to caller
// CHECK: catchret from %{{.*}} to label
void catches() {
  try {
    may_throw();
  } catch (int) {
  }
}
