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

import asyncio

from axserve.aio.client.stub import AxServeObject


async def test_iexplorer_async():
    on_visible_fired = asyncio.Event()

    async def OnVisible(visible):
        on_visible_fired.set()

    async with AxServeObject("InternetExplorer.Application") as iexplorer:
        await iexplorer.OnVisible.connect(OnVisible)
        await iexplorer.__setattr__("Visible", 1)
        fired = await on_visible_fired.wait()
        assert fired
        await iexplorer.Quit()
        await asyncio.sleep(1)


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(test_iexplorer_async())
