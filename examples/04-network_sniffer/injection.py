import struct
from ctypes import c_void_p, c_int32, cast, string_at
from socket import htonl, htons
from socket import inet_ntop
from typing import Union

from migi.decorators import stdcall
from migi.structs.inet import sockaddr_p, AF_INET, AF_INET6, sockaddr_in_p, sockaddr_in6_p
from migi.utils import dump_bytes


@stdcall('connect', 'Ws2_32.dll', interceptable=True)
def _native_connect(socket: c_void_p, name: sockaddr_p, namelen: c_int32) -> c_int32:
    sa_family = name.contents.sa_family
    print('[connect] socket: %d' % socket)
    print('[connect] sockaddr.sa_family: %d' % sa_family)
    if sa_family == AF_INET:
        sockaddr = cast(name, sockaddr_in_p).contents
        print('[connect] sockaddr_in.sin_addr: %s' % inet_ntop(sa_family, struct.pack("I", htonl(sockaddr.sin_addr.s_addr))))
        print('[connect] sockaddr_in.sin_port: %d' % htons(sockaddr.sin_port))
    elif sa_family == AF_INET6:
        sockaddr = cast(name, sockaddr_in6_p).contents
        print('[connect] sockaddr_in6.sin6_addr: %s' % inet_ntop(sa_family, struct.pack("8H", *[htons(i) for i in sockaddr.sin6_addr.s6_addr16])))
        print('[connect] sockaddr_in6.sin6_port: %d' % htons(sockaddr.sin6_port))
    return _native_connect.call_original(socket, name, namelen)


@stdcall('send', 'Ws2_32.dll', interceptable=True)
def _native_send(socket: c_void_p, buf: c_void_p, length: c_int32, flags: c_int32) -> c_int32:
    print('[send] socket: %d, length: %d, flags: %d' % (socket, length, flags))
    peeking_package_head('send', string_at(buf, length))
    return _native_send.call_original(socket, buf, length, flags)


@stdcall('recv', 'Ws2_32.dll', interceptable=True)
def _native_recv(socket: c_void_p, buf: c_void_p, length: c_int32, flags: c_int32) -> c_int32:
    received = _native_recv.call_original(socket, buf, length, flags)
    print('[recv] socket: %d, length: %d, flags: %d' % (socket, length, flags))
    if received > 0:
        peeking_package_head('recv', string_at(buf, length))
    return received


def peeking_package_head(tag: str, content: Union[bytes, str]):
    peeking_len = min(len(content), 64)
    print(f'[{tag}] peeking package head({peeking_len} bytes)\n%s' % dump_bytes(content[:peeking_len]))


_native_connect.intercept()
_native_send.intercept()
_native_recv.intercept()
