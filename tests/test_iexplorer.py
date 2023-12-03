from __future__ import annotations

import time

from axserve import AxServeObject


def test_iexplorer():
    res = {
        "visible_fired": False,
        "visible": None,
    }

    def OnVisible(visible):
        res["visible_fired"] = True
        res["visible"] = visible

    with AxServeObject("InternetExplorer.Application") as iexplorer:
        iexplorer.OnVisible.connect(OnVisible)
        iexplorer.Visible = 1
        assert res["visible_fired"]
        iexplorer.Quit()
        time.sleep(1)


if __name__ == "__main__":
    test_iexplorer()
