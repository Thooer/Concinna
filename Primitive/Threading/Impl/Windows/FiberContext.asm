option casemap:none

EXTERN Nova_FiberExitNotify:PROC

.code

; ctx layout: [0]=rsp, [8]=rip, [16]=rbx, [24]=rbp, [32]=r12, [40]=r13, [48]=r14, [56]=r15, [64]=rdi, [72]=rsi

PUBLIC Nova_SaveContext
Nova_SaveContext PROC
    ; RCX = ctx*
    mov     [rcx+0], rsp
    mov     rax, [rsp]
    mov     [rcx+8], rax
    mov     [rcx+16], rbx
    mov     [rcx+24], rbp
    mov     [rcx+32], r12
    mov     [rcx+40], r13
    mov     [rcx+48], r14
    mov     [rcx+56], r15
    mov     [rcx+64], rdi
    mov     [rcx+72], rsi
    ret
Nova_SaveContext ENDP

PUBLIC Nova_JumpContext
Nova_JumpContext PROC
    ; RCX = ctx*
    mov     rbx, [rcx+16]
    mov     rbp, [rcx+24]
    mov     r12, [rcx+32]
    mov     r13, [rcx+40]
    mov     r14, [rcx+48]
    mov     r15, [rcx+56]
    mov     rdi, [rcx+64]
    mov     rsi, [rcx+72]
    mov     rsp, [rcx+0]
    mov     rax, [rcx+8]
    jmp     rax
Nova_JumpContext ENDP

PUBLIC Nova_SwapContexts
Nova_SwapContexts PROC
    ; RCX = from*, RDX = to*
    mov     [rcx+0], rsp
    mov     rax, [rsp]
    mov     [rcx+8], rax
    mov     [rcx+16], rbx
    mov     [rcx+24], rbp
    mov     [rcx+32], r12
    mov     [rcx+40], r13
    mov     [rcx+48], r14
    mov     [rcx+56], r15
    mov     [rcx+64], rdi
    mov     [rcx+72], rsi

    mov     rbx, [rdx+16]
    mov     rbp, [rdx+24]
    mov     r12, [rdx+32]
    mov     r13, [rdx+40]
    mov     r14, [rdx+48]
    mov     r15, [rdx+56]
    mov     rdi, [rdx+64]
    mov     rsi, [rdx+72]
    mov     rsp, [rdx+0]
    mov     rax, [rdx+8]
    jmp     rax
Nova_SwapContexts ENDP

; Stack layout for Nova_FiberEnter:
; [rsp+0]  = ret to Nova_FiberExit
; [rsp+8]  = entry function pointer (void(*)(void*) noexcept)
; [rsp+16] = arg pointer (void*)
; [rsp+24] = Fiber* (for exit notify)

PUBLIC Nova_FiberEnter
Nova_FiberEnter PROC
    sub     rsp, 20h
    mov     rax, [rsp+28h]
    mov     rcx, [rsp+30h]
    call    rax
    add     rsp, 20h
    ret
Nova_FiberEnter ENDP

PUBLIC Nova_FiberExit
Nova_FiberExit PROC
    sub     rsp, 20h
    mov     rcx, [rsp+30h]
    call    Nova_FiberExitNotify
    add     rsp, 20h
    ret
Nova_FiberExit ENDP

END
