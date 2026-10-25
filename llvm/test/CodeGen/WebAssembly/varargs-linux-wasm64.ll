; RUN: llc < %s -mtriple=wasm64-unknown-linux-musl -asm-verbose=false -verify-machineinstrs | FileCheck %s --check-prefix=LINUX
; RUN: llc < %s -mtriple=wasm64-unknown-unknown -asm-verbose=false -verify-machineinstrs | FileCheck %s --check-prefix=PACKED
; RUN: llc < %s -mtriple=wasm64-unknown-wasi -asm-verbose=false -verify-machineinstrs | FileCheck %s --check-prefix=PACKED
; RUN: llc < %s -mtriple=wasm32-unknown-linux-musl -asm-verbose=false -verify-machineinstrs | FileCheck %s --check-prefix=PACKED

; wasm64 Linux gives each variadic argument an 8-byte slot, like x86-64 and
; AArch64. wasm32 and other wasm64 triples keep natural-size slots.

declare void @callee(i32, ...)

; LINUX-LABEL: call_ints:
; LINUX: i64.const 16
; LINUX: i64.const 7
; LINUX-NEXT: i64.store 8
; LINUX: i64.const 3
; LINUX-NEXT: i64.store 0
; Two ints share one 8-byte word when packed.
; PACKED-LABEL: call_ints:
; PACKED: i64.const 30064771075
; PACKED-NEXT: i64.store 0
define void @call_ints() {
  call void (i32, ...) @callee(i32 0, i32 3, i32 7)
  ret void
}

; Without an extension attribute, an i32 is zero-extended, as on x86-64.
; LINUX-LABEL: call_negative:
; LINUX: i64.const 4294967295
; LINUX-NEXT: i64.store 0
define void @call_negative() {
  call void (i32, ...) @callee(i32 0, i32 -1)
  ret void
}

; Sign extension is used when the frontend asks for it.
; LINUX-LABEL: call_signext:
; LINUX: i64.const -1
; LINUX-NEXT: i64.store 0
define void @call_signext() {
  call void (i32, ...) @callee(i32 0, i32 signext -1)
  ret void
}

; 16-byte types keep 16-byte alignment after an 8-byte slot.
; LINUX-LABEL: call_mixed:
; LINUX: i64.const 32
; LINUX-DAG: i64.const 24
; LINUX-DAG: i64.const 16
; LINUX: i64.const 1
; LINUX-NEXT: i64.store 0
define void @call_mixed(fp128 %x) {
  call void (i32, ...) @callee(i32 0, i32 1, fp128 %x)
  ret void
}

; An argument after an i128 starts in the next 8-byte slot.
; LINUX-LABEL: call_i128:
; LINUX: i64.const 48
; LINUX: i64.const 32
; LINUX-NEXT: i64.add
; LINUX-NEXT: i64.const 2
; LINUX-NEXT: i64.store 0
; LINUX: i64.const 24
; LINUX-NEXT: i64.add
; LINUX-NEXT: local.get 1
; LINUX-NEXT: i64.store 0
; LINUX: i64.const 16
; LINUX-NEXT: i64.add
; LINUX-NEXT: local.get 0
; LINUX-NEXT: i64.store 0
; LINUX: i64.const 1
; LINUX-NEXT: i64.store 0
define void @call_i128(i128 %x) {
  call void (i32, ...) @callee(i32 0, i32 1, i128 %x, i32 2)
  ret void
}

; va_arg of i32 reads the low half of an 8-byte slot and advances by 8.
; LINUX-LABEL: arg_i32:
; LINUX: i64.load 0
; LINUX: i64.const 8
; LINUX-NEXT: i64.add
; LINUX: i32.load 0
define i32 @arg_i32(ptr %ap) {
  %v = va_arg ptr %ap, i32
  ret i32 %v
}

; va_arg of fp128 aligns to 16, reads both halves and advances by 16.
; LINUX-LABEL: arg_fp128:
; LINUX: i64.const 15
; LINUX-NEXT: i64.add
; LINUX-NEXT: i64.const -16
; LINUX-NEXT: i64.and
; LINUX: i64.const 16
; LINUX-NEXT: i64.add
; LINUX-NEXT: i64.store 0
; LINUX: i64.load 8
define fp128 @arg_fp128(ptr %ap) {
  %v = va_arg ptr %ap, fp128
  ret fp128 %v
}

; Pointers and doubles take one slot each.
; LINUX-LABEL: call_ptr_double:
; LINUX: i64.const 16
; LINUX-NEXT: i64.add
; LINUX-NEXT: i64.const 3
; LINUX-NEXT: i64.store 0
; LINUX: f64.store 8
; LINUX: i64.store 0
define void @call_ptr_double(ptr %p, double %d) {
  call void (i32, ...) @callee(i32 0, ptr %p, double %d, i32 3)
  ret void
}
