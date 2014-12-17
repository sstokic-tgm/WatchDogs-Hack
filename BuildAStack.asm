EXTERNDEF retBuildAStackAddr:QWORD

.code
buildAstack proc
	add DWORD PTR [rdx+12], 20
	add [rdx+12], r13d
	mov eax, [rdx+12]
	mov r11, retBuildAStackAddr
	jmp r11
buildAstack endp
end