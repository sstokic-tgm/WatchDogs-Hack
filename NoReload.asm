EXTERNDEF retNoReloadAddr:QWORD

.code
noReload proc
	inc DWORD PTR [rdi+152]
	dec DWORD PTR [rdi+152]
	mov r11, retNoReloadAddr
	jmp r11
noReload endp
end