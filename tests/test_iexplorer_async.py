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


async def test_dynamic_iexplorer_async():
    from axserve.aio.client.stub import AxServeObject

    on_visible_fired = asyncio.Event()

    async def OnVisible(visible):
        on_visible_fired.set()

    async with AxServeObject("InternetExplorer.Application") as iexplorer:
        await iexplorer.OnVisible.connect(OnVisible)
        await iexplorer.__setattr__("Visible", 1)  # normal assignment syntax won't return awaitable
        fired = await on_visible_fired.wait()
        assert fired
        await iexplorer.Quit()
        await asyncio.sleep(1)


async def test_declarative_iexplorer_async():
    from axserve.aio.client.descriptor import AxServeEvent
    from axserve.aio.client.descriptor import AxServeMethod
    from axserve.aio.client.descriptor import AxServeProperty
    from axserve.aio.client.stub import AxServeObject

    class IExplorer(AxServeObject):
        __CLSID__ = "InternetExplorer.Application"

        OnVisible = AxServeEvent()
        Visible = AxServeProperty()
        Quit = AxServeMethod()

    on_visible_fired = asyncio.Event()

    async def OnVisible(visible):
        on_visible_fired.set()

    async with IExplorer() as iexplorer:
        await iexplorer.OnVisible.connect(OnVisible)
        await iexplorer.__setattr__("Visible", 1)  # normal assignment syntax won't return awaitable
        fired = await on_visible_fired.wait()
        assert fired
        await iexplorer.Quit()
        await asyncio.sleep(1)


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(test_dynamic_iexplorer_async())
    loop.run_until_complete(test_declarative_iexplorer_async())
