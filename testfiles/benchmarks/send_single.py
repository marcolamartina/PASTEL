import binascii
from pwn import *
def send(r,num):
    r.sendline(str(num))
port = 1234
server = '127.0.0.1'
sleep(1)
r = remote(server, port)
for i in range(10):
    send(r,i)
r.close()

