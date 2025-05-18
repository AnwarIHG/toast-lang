format ELF64 executable ;3

; SYSCALLS
SYS_read equ 0
SYS_write equ 1
SYS_exit equ 60

; exit status codes
EXIT_SUCCESS equ 0
EXIT_FAILURE equ 1

STDIN equ 0
STDOUT equ 1
STDERR equ 2

; ssize_t read(int fd, void buf[.count], size_t count);
macro read fd, buf, count
{
	mov	rax, SYS_read
	mov	rdi, fd
	mov rsi, buf
	mov	rdx, count
	syscall
}

; ssize_t write(int fd, const void buf[.count], size_t count);
macro write fd, buf, count
{
	mov	rax, SYS_write
	mov	rdi, fd
	mov rsi, buf
	mov	rdx, count
	syscall
}

macro exit status
{
  mov	rax, SYS_exit
  mov rdi, status
  syscall
}


segment readable executable
entry main

main:
  write STDOUT,massage,massage_len  
  read  STDIN,input,input_size
  exit EXIT_SUCCESS
  
  segment readable writeable

;; ---------------------------------
;; Operator 	Bits 	Bytes
;; byte 	    8 	  1
;; word 	    16 	  2
;; dword 	    32 	  4
;; fword 	    48 	  6
;; pword 	    48 	  6
;; qword 	    64 	  8
;; tbyte 	    80 	  10
;; tword 	    80 	  10
;; dqword 	  128 	16
;; xword 	    128 	16
;; qqword 	  256 	32
;; yword 	    256 	32
;; dqqword 	  512 	64
;; zword 	    512 	64

;; define bytes
;; db - file ; 1  byte
;; dw - du   ; 2  byte
;; dd        ; 4  byte
;; dp - df   ; 6  byte
;; dq        ; 8  byte
;; dt        ; 10 byte

;; reseve bytes
;; rb        ; 1  byte
;; rw        ; 2  byte
;; rd        ; 4  byte
;; rp        ; 6  byte
;; rf        ; 8  byte
;; rt        ; 10 byte

;; EX:
;; [var] rb [number] ; reseve number bytes

;; move word  [name], value - 2 byte
;; move dword [name], value - 4 byte
;; move qword [name], value - 8 byte
;; ---------------------------------

;; struc name
; {
;   .x dw 0
;   .y dw 0
;   .z dd 0
;   .a dq 0
; }

input_size db 32
input rb input_size;

massage db 'bay',0xA
massage_len = $- massage


massage2 db 'hiiii',0xA
massage2_len = $- massage

