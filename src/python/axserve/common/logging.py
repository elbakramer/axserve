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

import logging
import logging.config
import threading

from threading import _RLock
from typing import ClassVar


class LoggingMeta(type):
    __default_config: ClassVar[dict] = {
        "version": 1,
        "formatters": {
            "default": {
                "format": "%(asctime)s [CLIENT] [%(levelname)s] %(message)s",
            },
        },
        "handlers": {
            "console": {
                "class": "logging.StreamHandler",
                "level": "NOTSET",
                "formatter": "default",
            },
        },
        "loggers": {
            "axserve": {
                "level": "DEBUG",
                "propagate": False,
                "handlers": ["console"],
            },
        },
        "incremental": False,
        "disable_existing_loggers": False,
    }

    __initialized: ClassVar[bool] = False
    __init_lock: ClassVar[_RLock] = threading.RLock()

    def __initialize_if_necessary():
        if not LoggingMeta.__initialized:
            with LoggingMeta.__init_lock:
                if not LoggingMeta.__initialized:
                    LoggingMeta.__initialize()

    def __initialize():
        logging_config_dict = LoggingMeta.__default_config
        logging.basicConfig()
        logging.config.dictConfig(logging_config_dict)
        LoggingMeta.__initialized = True


LoggingMeta.__initialize_if_necessary()
