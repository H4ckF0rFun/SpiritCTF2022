from pwn import*

#sh = process("./login")
sh = remote('192.168.237.136',28157)
sh.sendlineafter(b'Username:',b'r00t')
sh.sendlineafter(b'Password:',b'1233211234567')

sh.interactive()
