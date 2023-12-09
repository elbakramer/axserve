# Copyright 2023 Yunseong Hwang
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

import ipaddress
import socket

from contextlib import closing


def FindFreePort() -> int:
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


def IsPrivateAddress(host) -> bool:
    host = socket.gethostbyname(host)
    ip_address = ipaddress.ip_address(host)
    private_networks_str = [
        "10.0.0.0/8",
        "127.0.0.0/8",
        "172.16.0.0/12",
        "192.168.0.0/16",
    ]
    private_networks = [ipaddress.ip_network(network) for network in private_networks_str]
    for private_network in private_networks:
        if ip_address in private_network:
            return True
    return False
