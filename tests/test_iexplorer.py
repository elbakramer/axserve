from __future__ import annotations

from axserve import AxServeObject


def test_iexplorer():
    res = {
        "visible_fired": False,
        "visible": None,
        "quit_fired": False,
    }

    def OnVisible(visible):
        res["visible_fired"] = True
        res["visible"] = visible

    def OnQuit():
        res["quit_fired"] = True

    with AxServeObject("InternetExplorer.Application") as iexplorer:
        iexplorer.OnVisible.connect(OnVisible)
        iexplorer.OnQuit.connect(OnQuit)
        iexplorer.Visible = 1
        assert res["visible_fired"]
        iexplorer.Quit()
        assert res["quit_fired"]


if __name__ == "__main__":
    test_iexplorer()
