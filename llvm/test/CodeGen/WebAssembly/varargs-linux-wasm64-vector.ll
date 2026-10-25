; RUN: not llc < %s -mtriple=wasm64-unknown-linux-musl -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR
; RUN: not llc < %s -mtriple=wasm64-unknown-linux-musl -mattr=+simd128 -o /dev/null 2>&1 | FileCheck %s --check-prefix=ERR
; RUN: llc < %s -mtriple=wasm32-unknown-linux-musl -o /dev/null
; RUN: llc < %s -mtriple=wasm64-unknown-unknown -o /dev/null
; RUN: llc < %s -mtriple=wasm64-unknown-wasi -o /dev/null

; wasm64 Linux's 8-byte vararg slots are not implemented for vectors the
; backend splits or widens, so they are rejected rather than laid out in a
; way va_arg would misread. Other targets keep the packed layout. Legal
; vectors are covered by varargs-linux-wasm64-simd.ll.

; ERR: error: {{.*}}WebAssembly hasn't implemented legalized vector varargs on wasm64 Linux

declare void @callee(i32, ...)

define void @call_v2i32(<2 x i32> %v) {
  call void (i32, ...) @callee(i32 0, <2 x i32> %v)
  ret void
}
