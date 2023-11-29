from __future__ import annotations

import ipaddress
import socket

from contextlib import closing

from wrapt import synchronized


@synchronized
def FindFreePort():
    with closing(
        socket.socket(
            socket.AF_INET,
            socket.SOCK_STREAM,
        )
    ) as sock:
        sock.bind(("", 0))
        sock.setsockopt(
            socket.SOL_SOCKET,
            socket.SO_REUSEADDR,
            1,
        )
        _, port = sock.getsockname()
        return port


def IsPrivateAddress(host):
    host = socket.gethostbyname(host)
    ip_address = ipaddress.ip_address(host)
    private_networks = [
        "10.0.0.0/8",
        "127.0.0.0/8",
        "172.16.0.0/12",
        "192.168.0.0/16",
    ]
    private_networks = [ipaddress.ip_network(network) for network in private_networks]
    for private_network in private_networks:
        if ip_address in private_network:
            return True
    return False
