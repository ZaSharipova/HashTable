global my_strlen

section .text

my_strlen:
        mov rax, rdi
        vpxor ymm0, ymm0, ymm0

.loop:
        vpcmpeqb ymm1, ymm0, [rax]  ; vp cmp equal byte, ymm1 = 00FF00FF...
        vpmovmskb edx, ymm1         ; each most signif into the 32 word (edx <- ymm1)
        test edx, edx
        jnz .found
        add rax, 32                 ; wrote this in order to use for words which are > 32 in length
        jmp .loop

.found:
        bsf edx, edx                ; bit scan forward -> finds the least signif set bit 
        add rax, rdx
        sub rax, rdi
        vzeroupper
        ret
