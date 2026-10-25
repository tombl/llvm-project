; RUN: llc < %s -mtriple=wasm64-unknown-linux-musl -mattr=+simd128 -asm-verbose=false -verify-machineinstrs | FileCheck %s

; A legal v128 vararg takes one 16-byte slot aligned to 16, and the next
; argument follows it.

declare void @callee(i32, ...)

; CHECK-LABEL: call_v4i32:
; CHECK: i64.const 32
; CHECK-NEXT: i64.add
; CHECK-NEXT: i64.const 2
; CHECK-NEXT: i64.store 0
; CHECK: i64.const 16
; CHECK-NEXT: i64.add
; CHECK-NEXT: local.get 0
; CHECK-NEXT: v128.store 0
; CHECK: i64.const 1
; CHECK-NEXT: i64.store 0
define void @call_v4i32(<4 x i32> %v) {
  call void (i32, ...) @callee(i32 0, i32 1, <4 x i32> %v, i32 2)
  ret void
}
