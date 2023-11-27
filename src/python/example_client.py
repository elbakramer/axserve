from __future__ import annotations

import threading

import grpc

from axserve import AxServeObject


def main():
    with grpc.insecure_channel("127.0.0.1:8080") as channel:
        with AxServeObject(channel) as ax:
            print(ax.GetAPIModulePath())
            print(ax.GetConnectState())

            connected = threading.Condition()

            def OnEventConnect(res):
                print(res)
                ax.OnEventConnect.disconnect(OnEventConnect)
                with connected:
                    connected.notify()

            ax.OnEventConnect.connect(OnEventConnect)

            print(ax.CommConnect())

            with connected:
                connected.wait()


if __name__ == "__main__":
    main()
