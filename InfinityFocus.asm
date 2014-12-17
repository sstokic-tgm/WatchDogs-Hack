EXTERNDEF retInfFocusAddr:QWORD

.code
infFocus proc
	mov [rcx+392], DWORD PTR 1068495628
	movss xmm0, DWORD PTR [rcx+392]
	mov r11, retInfFocusAddr
	jmp r11
infFocus endp
end