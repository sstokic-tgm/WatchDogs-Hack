EXTERNDEF pJmpToNoRecoilAddr:QWORD

.code
noRecoil proc
	mov r11, pJmpToNoRecoilAddr
	jmp r11
noRecoil endp
end