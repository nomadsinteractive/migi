from ctypes import c_uint16, c_char, POINTER, Structure, Union, c_uint32, c_uint8


class SockAddr(Structure):
    _fields_ = [
            ('sa_family', c_uint16),
            ('sa_data', c_char * 14),
    ]


class InAddr(Structure):
    _fields_ = [
        ('s_addr', c_uint32)
    ]


class In6Addr(Union):
    _fields_ = [
        ('s6_addr', c_uint8 * 16),
        ('s6_addr16', c_uint16 * 8),
        ('s6_addr32', c_uint32 * 4),
    ]


class SockAddrIn(Structure):

    _fields_ = [
        ('sin_family', c_uint16),
        ('sin_port', c_uint16),
        ('sin_addr', InAddr),
        ('sin_zero', c_char * 8)
    ]


class SockAddrIn6(Structure):
    _fields_ = [
        ('sin6_family', c_uint16),
        ('sin6_port', c_uint16),
        ('sin6_flowinfo', c_uint32),
        ('sin6_addr', In6Addr),
        ('sin6_scope_id', c_uint32)
    ]


sockaddr = SockAddr
sockaddr_p = POINTER(sockaddr)

in_addr = InAddr
in_addr_p = POINTER(in_addr)

sockaddr_in = SockAddrIn
sockaddr_in_p = POINTER(sockaddr_in)

in6_addr = In6Addr
in6_addr_p = POINTER(in6_addr)

sockaddr_in6 = SockAddrIn6
sockaddr_in6_p = POINTER(sockaddr_in6)

AF_INET = 2
AF_INET6 = 23
