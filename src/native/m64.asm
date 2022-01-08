PUBLIC make_generic_call_impl
PUBLIC make_generic_call_r_mmx0

_DATA	segment
_DATA	ends

_RDATA	segment
_RDATA	ends

_TEXT	segment

make_generic_call_impl proc

    push    rbp
    mov     rbp, rsp
    push    rbx
    push    rcx
    push    rdx
    push    r8
    push    r9
    mov     rax, rcx
    mov     rbx, rdx
    mov     rsp, r8
    mov     rcx, [rbx]
    mov     rdx, [rbx + 8]
    mov     r8, [rbx + 16]
    mov     r9, [rbx + 24]
    call    rax
    pop     rcx
    pop     rdx
    pop     r8
    pop     r9
    pop     rbx
    mov     rsp, rbp
    pop     rbp
    ret

make_generic_call_impl endp


make_generic_call_r_mmx0 proc

    push    rbp
    mov     rbp, rsp
    mov     rax, [rbp + 10h]
    mov     rsp, [rbp + 18h]
    call    rax
    movsd   qword ptr [rbp+20h], xmm0
    mov     rsp, rbp
    pop     rbp
    ret     24

make_generic_call_r_mmx0 endp

_TEXT	ends

end
