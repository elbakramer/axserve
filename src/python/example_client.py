from __future__ import annotations

import threading

import grpc

from axserve import AxServeObject


def main():
    with grpc.insecure_channel("127.0.0.1:8080") as channel:
        # channel = "{A1574A0D-6BFA-4BD7-9020-DED88711818D}"
        with AxServeObject(channel) as ax:
            print(ax.GetAPIModulePath())
            print(ax.GetConnectState())

            connected = threading.Event()

            def OnEventConnect(res):
                print(res)
                ax.OnEventConnect.disconnect(OnEventConnect)
                connected.set()

            ax.OnEventConnect.connect(OnEventConnect)

            print(ax.CommConnect())

            connected.wait()


if __name__ == "__main__":
    main()
