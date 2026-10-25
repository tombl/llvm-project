// RUN: %clang_cc1 %s -triple wasm32-unknown-unknown -fexceptions -exception-model=wasm -target-feature +exception-handling -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple wasm32-unknown-linux-musl -fexceptions -exception-model=wasm -target-feature +exception-handling -emit-llvm -o - | FileCheck %s

// C cleanups under wasm EH use funclet pads, which are all the wasm backend
// can lower; landingpads with __gcc_personality_v0 crash it.

void may_throw(void);
void release(int *);

// CHECK-LABEL: define {{.*}}void @cleanup()
// CHECK-SAME: personality ptr @__gxx_wasm_personality_v0
// CHECK: cleanuppad within none []
// CHECK-NOT: landingpad
void cleanup(void) {
  int guard __attribute__((cleanup(release))) = 0;
  may_throw();
}
