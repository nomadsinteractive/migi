PUBLIC _make_generic_call_impl
PUBLIC _make_generic_call_r_mmx0

.686p
.xmm

_DATA	segment
_DATA	ends

_RDATA	segment
_RDATA	ends

_TEXT	segment

_make_generic_call_impl proc

    push    ebp
    mov     ebp, esp
    push    ecx
    push    edx
    mov     eax, [ebp + 12]
    mov     ecx, [eax]
    mov     edx, [eax + 4]
    mov     eax, [ebp + 8]
    mov     esp, [ebp + 16]
    call    eax
    pop     edx
    pop     ecx
    mov     esp, ebp
    pop     ebp
    ret

_make_generic_call_impl endp


_make_generic_call_r_mmx0 proc

    push    ebp
    mov     ebp, esp
    mov     eax, [ebp + 8]
    mov     esp, [ebp + 0Ch]
    call    eax
    movsd   qword ptr [ebp+10h], xmm0
    mov     esp, ebp
    pop     ebp
    ret     0Ch

_make_generic_call_r_mmx0 endp

_TEXT	ends

end
