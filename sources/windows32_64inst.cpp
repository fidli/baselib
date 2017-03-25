#ifndef WINDOWS_64INST
#define WINDOWS_64INST

//functions extracted 1:1 from SDL

extern "C" {
    
    __declspec(naked) void _ftol2()
    {
        __asm
        {
            fistp qword ptr [esp-8]
                mov   edx,[esp-4]
                mov   eax,[esp-8]
                ret
        }
    }
    
    
    void
        __declspec(naked)
        _ftol()
    {
        /* *INDENT-OFF* */
        __asm {
            push        ebp
                mov         ebp,esp
                sub         esp,20h
                and         esp,0FFFFFFF0h
                fld         st(0)
                fst         dword ptr [esp+18h]
                fistp       qword ptr [esp+10h]
                fild        qword ptr [esp+10h]
                mov         edx,dword ptr [esp+18h]
                mov         eax,dword ptr [esp+10h]
                test        eax,eax
                je          integer_QnaN_or_zero
                arg_is_not_integer_QnaN:
            fsubp       st(1),st
                test        edx,edx
                jns         positive
                fstp        dword ptr [esp]
                mov         ecx,dword ptr [esp]
                xor         ecx,80000000h
                add         ecx,7FFFFFFFh
                adc         eax,0
                mov         edx,dword ptr [esp+14h]
                adc         edx,0
                jmp         localexit
                positive:
            fstp        dword ptr [esp]
                mov         ecx,dword ptr [esp]
                add         ecx,7FFFFFFFh
                sbb         eax,0
                mov         edx,dword ptr [esp+14h]
                sbb         edx,0
                jmp         localexit
                integer_QnaN_or_zero:
            mov         edx,dword ptr [esp+14h]
                test        edx,7FFFFFFFh
                jne         arg_is_not_integer_QnaN
                fstp        dword ptr [esp+18h]
                fstp        dword ptr [esp+18h]
                localexit:
            leave
                ret
        }
        /* *INDENT-ON* */
    }
    
    void
        _ftol2_sse()
    {
        _ftol();
    }
    
    void
        __declspec(naked)
        _allmul()
    {
        /* *INDENT-OFF* */
        __asm {
            mov         eax, dword ptr[esp+8]
                mov         ecx, dword ptr[esp+10h]
                or          ecx, eax
                mov         ecx, dword ptr[esp+0Ch]
                jne         hard
                mov         eax, dword ptr[esp+4]
                mul         ecx
                ret         10h
                hard:
            push        ebx
                mul         ecx
                mov         ebx, eax
                mov         eax, dword ptr[esp+8]
                mul         dword ptr[esp+14h]
                add         ebx, eax
                mov         eax, dword ptr[esp+8]
                mul         ecx
                add         edx, ebx
                pop         ebx
                ret         10h
        }
        /* *INDENT-ON* */
    }
    
    void
        __declspec(naked)
        _alldiv()
    {
        /* *INDENT-OFF* */
        __asm {
            push        edi
                push        esi
                push        ebx
                xor         edi,edi
                mov         eax,dword ptr [esp+14h]
                or          eax,eax
                jge         L1
                inc         edi
                mov         edx,dword ptr [esp+10h]
                neg         eax
                neg         edx
                sbb         eax,0
                mov         dword ptr [esp+14h],eax
                mov         dword ptr [esp+10h],edx
                L1:
            mov         eax,dword ptr [esp+1Ch]
                or          eax,eax
                jge         L2
                inc         edi
                mov         edx,dword ptr [esp+18h]
                neg         eax
                neg         edx
                sbb         eax,0
                mov         dword ptr [esp+1Ch],eax
                mov         dword ptr [esp+18h],edx
                L2:
            or          eax,eax
                jne         L3
                mov         ecx,dword ptr [esp+18h]
                mov         eax,dword ptr [esp+14h]
                xor         edx,edx
                div         ecx
                mov         ebx,eax
                mov         eax,dword ptr [esp+10h]
                div         ecx
                mov         edx,ebx
                jmp         L4
                L3:
            mov         ebx,eax
                mov         ecx,dword ptr [esp+18h]
                mov         edx,dword ptr [esp+14h]
                mov         eax,dword ptr [esp+10h]
                L5:
            shr         ebx,1
                rcr         ecx,1
                shr         edx,1
                rcr         eax,1
                or          ebx,ebx
                jne         L5
                div         ecx
                mov         esi,eax
                mul         dword ptr [esp+1Ch]
                mov         ecx,eax
                mov         eax,dword ptr [esp+18h]
                mul         esi
                add         edx,ecx
                jb          L6
                cmp         edx,dword ptr [esp+14h]
                ja          L6
                jb          L7
                cmp         eax,dword ptr [esp+10h]
                jbe         L7
                L6:
            dec         esi
                L7:
            xor         edx,edx
                mov         eax,esi
                L4:
            dec         edi
                jne         L8
                neg         edx
                neg         eax
                sbb         edx,0
                L8:
            pop         ebx
                pop         esi
                pop         edi
                ret         10h
        }
        /* *INDENT-ON* */
    }
    
    void
        __declspec(naked)
        _allrem()
    {
        /* *INDENT-OFF* */
        __asm {
            push        ebx
                push        edi
                xor         edi,edi
                mov         eax,dword ptr [esp+10h]
                or          eax,eax
                jge         L1
                inc         edi
                mov         edx,dword ptr [esp+0Ch]
                neg         eax
                neg         edx
                sbb         eax,0
                mov         dword ptr [esp+10h],eax
                mov         dword ptr [esp+0Ch],edx
                L1:
            mov         eax,dword ptr [esp+18h]
                or          eax,eax
                jge         L2
                mov         edx,dword ptr [esp+14h]
                neg         eax
                neg         edx
                sbb         eax,0
                mov         dword ptr [esp+18h],eax
                mov         dword ptr [esp+14h],edx
                L2:
            or          eax,eax
                jne         L3
                mov         ecx,dword ptr [esp+14h]
                mov         eax,dword ptr [esp+10h]
                xor         edx,edx
                div         ecx
                mov         eax,dword ptr [esp+0Ch]
                div         ecx
                mov         eax,edx
                xor         edx,edx
                dec         edi
                jns         L4
                jmp         L8
                L3:
            mov         ebx,eax
                mov         ecx,dword ptr [esp+14h]
                mov         edx,dword ptr [esp+10h]
                mov         eax,dword ptr [esp+0Ch]
                L5:
            shr         ebx,1
                rcr         ecx,1
                shr         edx,1
                rcr         eax,1
                or          ebx,ebx
                jne         L5
                div         ecx
                mov         ecx,eax
                mul         dword ptr [esp+18h]
                xchg        eax,ecx
                mul         dword ptr [esp+14h]
                add         edx,ecx
                jb          L6
                cmp         edx,dword ptr [esp+10h]
                ja          L6
                jb          L7
                cmp         eax,dword ptr [esp+0Ch]
                jbe         L7
                L6:
            sub         eax,dword ptr [esp+14h]
                sbb         edx,dword ptr [esp+18h]
                L7:
            sub         eax,dword ptr [esp+0Ch]
                sbb         edx,dword ptr [esp+10h]
                dec         edi
                jns         L8
                L4:
            neg         edx
                neg         eax
                sbb         edx,0
                L8:
            pop         edi
                pop         ebx
                ret         10h
        }
        /* *INDENT-ON* */
    }
    
    
}

#endif