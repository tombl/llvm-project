// RUN: %clang_cc1 -triple wasm64-unknown-linux-musl -emit-llvm -o - %s | FileCheck %s --check-prefixes=CHECK,LINUX
// RUN: %clang_cc1 -triple wasm32-unknown-linux-musl -emit-llvm -o - %s | FileCheck %s --check-prefixes=CHECK,W32
// RUN: %clang_cc1 -triple wasm64-unknown-unknown -emit-llvm -o - %s | FileCheck %s --check-prefixes=CHECK,PACKED
// RUN: %clang_cc1 -triple wasm64-unknown-wasi -emit-llvm -o - %s | FileCheck %s --check-prefixes=CHECK,PACKED

// wasm64 Linux reads each variadic argument from an 8-byte slot, matching the
// backend's caller layout. wasm32 and other wasm64 triples use 4-byte slots.

#include <stdarg.h>

struct S1 { int x; };
struct S2 { int a, b; };
void callee(int, ...);

// Variadic ints carry no extension attribute, so the backend zero-extends.
// CHECK-LABEL: define{{.*}} void @call_negative
// CHECK: call void (i32, ...) @callee(i32 noundef 0, i32 noundef -1)
void call_negative(void) { callee(0, -1); }

// CHECK-LABEL: define{{.*}} i32 @next_int
// LINUX: getelementptr inbounds i8, ptr %{{.*}}, i64 8
// W32: getelementptr inbounds i8, ptr %{{.*}}, i32 4
// PACKED: getelementptr inbounds i8, ptr %{{.*}}, i64 4
int next_int(va_list ap) { return va_arg(ap, int); }

// CHECK-LABEL: define{{.*}} i64 @next_long
// LINUX: getelementptr inbounds i8, ptr %{{.*}}, i64 8
long long next_long(va_list ap) { return va_arg(ap, long long); }

// 16-byte types align to 16 and advance by 16.
// CHECK-LABEL: define{{.*}} i128 @next_i128
// LINUX: [[P:%.*]] = call ptr @llvm.ptrmask.p0.i64(ptr %{{.*}}, i64 -16)
// LINUX: getelementptr inbounds i8, ptr [[P]], i64 16
__int128 next_i128(va_list ap) { return va_arg(ap, __int128); }

// CHECK-LABEL: define{{.*}} fp128 @next_ld
// LINUX: [[P:%.*]] = call ptr @llvm.ptrmask.p0.i64(ptr %{{.*}}, i64 -16)
// LINUX: getelementptr inbounds i8, ptr [[P]], i64 16
long double next_ld(va_list ap) { return va_arg(ap, long double); }

// A single-element struct is read directly from its slot.
// CHECK-LABEL: define{{.*}} i32 @next_s1
// LINUX: getelementptr inbounds i8, ptr %{{.*}}, i64 8
// LINUX: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %{{.*}}, ptr align 8 %{{.*}}, i64 4, i1 false)
struct S1 next_s1(va_list ap) { return va_arg(ap, struct S1); }

// Other aggregates are passed indirectly: one pointer slot, then the next
// argument in the following slot.
// CHECK-LABEL: define{{.*}} i32 @next_s2_then_int
// LINUX: getelementptr inbounds i8, ptr %{{.*}}, i64 8
// LINUX: load ptr, ptr %{{.*}}, align 8
// LINUX: getelementptr inbounds i8, ptr %{{.*}}, i64 8
// LINUX: load i32, ptr %{{.*}}, align 8
int next_s2_then_int(va_list ap) {
  struct S2 s = va_arg(ap, struct S2);
  return s.a + va_arg(ap, int);
}

// Callers pass a single-element struct as its coerced element and other
// aggregates as a pointer to a byval copy.
// CHECK-LABEL: define{{.*}} void @call_aggregates
// CHECK: call void (i32, ...) @callee(i32 noundef 0, i32 %{{.*}}, ptr noundef byval(%struct.S2) align 4 %{{.*}})
void call_aggregates(struct S1 s1, struct S2 s2) {
  callee(0, s1, s2);
}

// The argument after a direct struct is in the next slot.
// CHECK-LABEL: define{{.*}} i32 @next_s1_then_int
// LINUX: getelementptr inbounds i8, ptr %{{.*}}, i64 8
// LINUX: [[NEXT:%.*]] = load ptr, ptr %{{.*}}, align 8
// LINUX: getelementptr inbounds i8, ptr [[NEXT]], i64 8
// LINUX: load i32, ptr [[NEXT]], align 8
int next_s1_then_int(va_list ap) {
  struct S1 s = va_arg(ap, struct S1);
  return s.x + va_arg(ap, int);
}

// The argument after a long double is 16 bytes on.
// CHECK-LABEL: define{{.*}} i32 @next_ld_then_int
// LINUX: [[P:%.*]] = call ptr @llvm.ptrmask.p0.i64(ptr %{{.*}}, i64 -16)
// LINUX: getelementptr inbounds i8, ptr [[P]], i64 16
// LINUX: [[NEXT:%.*]] = load ptr, ptr %{{.*}}, align 8
// LINUX: getelementptr inbounds i8, ptr [[NEXT]], i64 8
// LINUX: load i32, ptr [[NEXT]], align 8
int next_ld_then_int(va_list ap) {
  long double d = va_arg(ap, long double);
  (void)d;
  return va_arg(ap, int);
}
