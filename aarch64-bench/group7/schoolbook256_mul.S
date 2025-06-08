// 128-bit (4×32) × 128-bit (4×32) → 256-bit (8×32)
// x0 = a[4], x1 = b[4], x2 = res[8]
.text
.align  4
.global schoolbook_256_mul_asm
.type   schoolbook_256_mul_asm, %function

schoolbook_256_mul_asm:
    // Save callee-saved registers
    stp     x19, x20, [sp, #-16]!
    stp     x21, x22, [sp, #-16]!

    mov     w19, #0              // i = 0
outer_loop:
    cmp     w19, #4
    bge     done

    ldr     w3, [x0, w19, UXTW #2]   // a[i]
    mov     x20, #0                 // carry = 0

    // j = 0
    ldr     w4, [x1]                // b[0]
    add     w5, w19, #0
    add     x5, x2, w5, UXTW #2
    ldr     w6, [x5]
    umull   x7, w3, w4
    add     x7, x7, x6, UXTW
    adds    x7, x7, x20
    str     w7, [x5]
    lsr     x20, x7, #32

    // j = 1
    ldr     w4, [x1, #4]            // b[1]
    add     w5, w19, #1
    add     x5, x2, w5, UXTW #2
    ldr     w6, [x5]
    umull   x7, w3, w4
    add     x7, x7, x6, UXTW
    adds    x7, x7, x20
    str     w7, [x5]
    lsr     x20, x7, #32

    // j = 2
    ldr     w4, [x1, #8]            // b[2]
    add     w5, w19, #2
    add     x5, x2, w5, UXTW #2
    ldr     w6, [x5]
    umull   x7, w3, w4
    add     x7, x7, x6, UXTW
    adds    x7, x7, x20
    str     w7, [x5]
    lsr     x20, x7, #32

    // j = 3
    ldr     w4, [x1, #12]           // b[3]
    add     w5, w19, #3
    add     x5, x2, w5, UXTW #2
    ldr     w6, [x5]
    umull   x7, w3, w4
    add     x7, x7, x6, UXTW
    adds    x7, x7, x20
    str     w7, [x5]
    lsr     x20, x7, #32

    // propagate carry
    add     w5, w19, #4
carry_loop:
    cmp     x20, #0
    beq     next_i

    add     x6, x2, w5, UXTW #2
    ldr     w7, [x6]
    add     x7, x20, x7, UXTW
    str     w7, [x6]
    lsr     x20, x7, #32
    add     w5, w5, #1
    b       carry_loop

next_i:
    add     w19, w19, #1
    b       outer_loop

done:
    ldp     x21, x22, [sp], #16
    ldp     x19, x20, [sp], #16
    ret

.size schoolbook_256_mul_asm, . - schoolbook_256_mul_asm
