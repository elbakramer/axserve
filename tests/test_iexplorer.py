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

import threading
import time

from axserve.client.stub import AxServeObject


def test_iexplorer():
    on_visible_fired = threading.Event()

    def OnVisible(visible):
        on_visible_fired.set()

    with AxServeObject("InternetExplorer.Application") as iexplorer:
        iexplorer.OnVisible.connect(OnVisible)
        iexplorer.Visible = 1
        fired = on_visible_fired.wait(10)
        assert fired
        iexplorer.Quit()
        time.sleep(1)


if __name__ == "__main__":
    test_iexplorer()
