import binascii
from pwn import *
def send(r,num):
    r.sendline(str(num))
port = 1234
server = '127.0.0.1'
sleep(1)
for i in range(10000):
    r = remote(server, port)
    send(r,i)
    r.close()

