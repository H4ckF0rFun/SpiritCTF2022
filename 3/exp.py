from pwn import*

#sh = process("./login-rev")

#192.168.237.136:28027
sh = remote('192.168.237.136',28027)

sh.sendlineafter(b'Username:',b'aaa')

sh.sendafter(b'Password:',b'a' * 16)

sh.interactive()
