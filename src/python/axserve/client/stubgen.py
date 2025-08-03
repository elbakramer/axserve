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

import ast
import datetime
import itertools
import sys

from typing import TYPE_CHECKING
from typing import ClassVar

import pythoncom
import win32api

from pythoncom import LoadRegTypeLib
from pythoncom import LoadTypeLib
from pythoncom import ProgIDFromCLSID
from pywintypes import IID
from pywintypes import TimeType
from win32api import ExpandEnvironmentStrings
from win32api import RegOpenKey
from win32api import RegQueryValue
from win32api import RegQueryValueEx
from win32com.client.build import MakePublicAttributeName as _MakePublicAttributeName
from win32com.client.genpy import Generator
from win32com.client.genpy import MakeEventMethodName as _MakeEventMethodName
from win32com.client.selecttlb import EnumKeys
from win32com.client.selecttlb import TypelibSpec
from win32con import HKEY_CLASSES_ROOT
from win32con import REG_EXPAND_SZ


if TYPE_CHECKING:
    from collections.abc import Sequence


# ruff: noqa: N802


def GetTypelibSpecsForTypelibCLSID(clsid: str) -> list[TypelibSpec] | None:
    clsid = str(IID(clsid))

    key_typelibs = RegOpenKey(HKEY_CLASSES_ROOT, "TypeLib")

    try:
        key_clsid = RegOpenKey(key_typelibs, clsid)
    except win32api.error:
        return None

    specs = []
    enum_keys: list[tuple[str, str]] = EnumKeys(key_clsid)

    for version, tlbdesc in enum_keys:
        versions = version.split(".")
        versions = iter(versions)

        major = next(versions)
        minor = next(versions, None)

        key_version = RegOpenKey(key_clsid, version)

        try:
            flags = int(RegQueryValue(key_version, "FLAGS"))
        except (win32api.error, ValueError):
            flags = 0

        for lcid_string, _ in EnumKeys(key_version):
            try:
                lcid = int(lcid_string)
            except ValueError:
                continue

            try:
                key_dll = RegOpenKey(key_version, f"{lcid}\\win32")
            except win32api.error:
                try:
                    key_dll = RegOpenKey(key_version, f"{lcid}\\win64")
                except win32api.error:
                    continue

            try:
                dll, typ = RegQueryValueEx(key_dll, None)
                if typ == REG_EXPAND_SZ:
                    dll = ExpandEnvironmentStrings(dll)
            except win32api.error:
                dll = None

            spec = TypelibSpec(clsid, lcid, major, minor, flags)
            spec.dll = dll
            spec.desc = tlbdesc
            spec.ver_desc = tlbdesc + " (" + version + ")"
            specs.append(spec)

    return specs


def GetLatestTypelibSpecFromSpecs(
    specs: Sequence[TypelibSpec] | None = None,
) -> TypelibSpec | None:
    if not specs:
        return None
    specs_grouped_by_version = itertools.groupby(
        specs,
        key=lambda spec: (
            spec.major,
            spec.minor if spec.minor is not None else sys.maxsize,
        ),
    )
    specs_grouped_by_version = [(k, list(g)) for k, g in specs_grouped_by_version]
    specs_latest = sorted(specs_grouped_by_version, key=lambda kg: kg[0])[-1][1]
    specs_latest_by_lcid = {spec.lcid: spec for spec in specs_latest}
    spec = specs_latest_by_lcid.get(0) or next(
        iter(specs_latest_by_lcid.values()), None
    )
    return spec


def GetLatestTypelibSpecForTypelibCLSID(clsid: str) -> TypelibSpec | None:
    specs = GetTypelibSpecsForTypelibCLSID(clsid)
    spec = GetLatestTypelibSpecFromSpecs(specs)
    return spec


def GetTypelibSpecForCLSID(clsid: str) -> TypelibSpec | None:
    spec = GetLatestTypelibSpecForTypelibCLSID(clsid)

    if spec is not None:
        return spec

    clsid = str(IID(clsid))

    key_wow6432node = RegOpenKey(HKEY_CLASSES_ROOT, "WOW6432Node")
    key_clsids = RegOpenKey(key_wow6432node, "CLSID")

    try:
        key_clsid = RegOpenKey(key_clsids, clsid)
    except win32api.error:
        key_clsids = RegOpenKey(HKEY_CLASSES_ROOT, "CLSID")
        try:
            key_clsid = RegOpenKey(key_clsids, clsid)
        except win32api.error:
            return None

    try:
        typelib_clsid = RegQueryValue(key_clsid, "TypeLib")
    except win32api.error:
        return None

    try:
        version = RegQueryValue(key_clsid, "Version")
    except win32api.error:
        version = None
        major = None
        minor = None
    else:
        versions = version.split(".")
        versions = iter(versions)
        major = next(versions)
        minor = next(versions, None)

    specs = GetTypelibSpecsForTypelibCLSID(typelib_clsid)

    if not specs:
        return None

    if major is not None:
        specs = [spec for spec in specs if spec.major == major]
        if minor is not None:
            specs = [spec for spec in specs if spec.minor == minor]

    return GetLatestTypelibSpecFromSpecs(specs)


def LoadTypeLibForSpec(spec: TypelibSpec):
    if spec.dll:
        tlb = LoadTypeLib(spec.dll)
    else:
        tlb = LoadRegTypeLib(
            spec.clsid,
            spec.major,
            spec.minor,
            spec.lcid,
        )
    return tlb


def BuildOleItemsForCLSID(clsid: str):
    spec = GetTypelibSpecForCLSID(clsid)
    if not spec:
        return {}, {}, {}, {}
    tlb = LoadTypeLibForSpec(spec)
    gen = Generator(tlb, spec.dll, None, bBuildHidden=1)
    return gen.BuildOleItemsFromType()


class StubGenerator:
    COMTYPE_TO_ANNOTATION: ClassVar = {
        pythoncom.VT_I2: int.__name__,
        pythoncom.VT_I4: int.__name__,
        pythoncom.VT_R4: float.__name__,
        pythoncom.VT_R8: float.__name__,
        pythoncom.VT_BSTR: str.__name__,
        pythoncom.VT_BOOL: bool.__name__,
        pythoncom.VT_VARIANT: "Any",
        pythoncom.VT_UNKNOWN: "Any",
        pythoncom.VT_VOID: "None",
    }

    def __init__(
        self,
        *,
        is_async: bool = False,
        is_base: bool = False,
    ):
        self._is_async = is_async
        self._is_base = is_base

        self._used_iterator = False
        self._used_async_iterator = False
        self._used_emtpy = False
        self._used_missing = False
        self._used_time = False

    def MakePublicAttributeName(self, name: str, *, is_global: bool = False) -> str:
        return _MakePublicAttributeName(name, is_global=is_global)

    def MakeEventMethodName(self, name: str) -> str:
        return _MakeEventMethodName(name)

    def MakeDefaultArg(self, arg_desc) -> ast.expr | None:
        try:
            arg_flag = arg_desc[1]
        except IndexError:
            arg_flag = pythoncom.PARAMFLAG_FIN

        if arg_flag & pythoncom.PARAMFLAG_FHASDEFAULT:
            arg_default = arg_desc[2]
            if isinstance(arg_default, datetime.datetime):
                arg_default = ast.Tuple(
                    [ast.Constant(v) for v in arg_default.utctimetuple()]
                )
            elif isinstance(arg_default, TimeType):
                arg_default = ast.Call(
                    ast.Name("Time", ast.Load()),
                    [
                        ast.Constant(arg_default.year),
                        ast.Constant(arg_default.month),
                        ast.Constant(arg_default.day),
                        ast.Constant(arg_default.hour),
                        ast.Constant(arg_default.minute),
                        ast.Constant(arg_default.second),
                        ast.Constant(0),
                        ast.Constant(0),
                        ast.Constant(0),
                        ast.Constant(arg_default.msec),  # type: ignore
                    ],
                    [],
                )
                self._used_time = True
            else:
                arg_default = ast.Constant(arg_default)
        else:
            arg_default = None

        return arg_default

    def MakeArguments(
        self,
        fdesc,
        names: list[str],
        *,
        is_method: bool = True,
        set_defaults: bool = False,
        def_named_opt_arg: ast.expr | None = None,
        def_named_not_opt_arg: ast.expr | None = None,
        def_unnamed_arg: ast.expr | None = None,
        def_out_arg: ast.expr | None = None,
    ) -> ast.arguments:
        arg_descs = fdesc[2]

        num_args = len(arg_descs)
        num_opt_args = fdesc[6]

        if num_opt_args == -1:
            first_opt_arg = num_args
            num_args -= 1
        else:
            first_opt_arg = num_args - num_opt_args

        args = []
        defaults = []

        if is_method:
            args.append(ast.arg("self", None))

        empty = ast.Name("Empty", ast.Load())
        missing = ast.Name("Missing", ast.Load())

        if def_named_opt_arg is None:
            def_named_opt_arg = empty
        if def_named_not_opt_arg is None:
            def_named_not_opt_arg = empty
        if def_unnamed_arg is None:
            def_unnamed_arg = empty
        if def_out_arg is None:
            def_out_arg = missing

        for i in range(num_args):
            try:
                arg_name = names[i + 1]
                if arg_name:
                    named_arg = True
                else:
                    arg_name = f"arg{i}"
                    named_arg = False
            except IndexError:
                arg_name = f"arg{i}"
                named_arg = False

            arg_name = self.MakePublicAttributeName(arg_name)
            arg_desc = fdesc[2][i]
            arg_type = arg_desc[0]
            arg_type = self.COMTYPE_TO_ANNOTATION.get(arg_type, "Any")
            arg_annotation = ast.Name(arg_type, ast.Load())
            arg = ast.arg(arg_name, arg_annotation)
            args.append(arg)

            arg_default = self.MakeDefaultArg(arg_desc)

            if arg_default is None and set_defaults:
                if (
                    arg_desc[1] & (pythoncom.PARAMFLAG_FOUT | pythoncom.PARAMFLAG_FIN)
                    == pythoncom.PARAMFLAG_FOUT
                ):
                    arg_default = def_out_arg
                elif named_arg:
                    if i >= first_opt_arg:
                        arg_default = def_named_opt_arg
                    else:
                        arg_default = def_named_not_opt_arg
                else:
                    arg_default = def_unnamed_arg

            if arg_default is not None:
                defaults.append(arg_default)
                if arg_default is empty:
                    self._used_emtpy = True
                elif arg_default is missing:
                    self._used_missing = True

        vararg = ast.arg(names[-1], None) if num_opt_args == -1 else None
        args = ast.arguments([], args, vararg, [], [], None, defaults)

        return args

    def MakeMethodFunctionDef(
        self,
        entry,
        name: str | None = None,
        *,
        is_sink: bool = False,
        is_getter: bool = False,
        is_setter: bool = False,
        is_setter_only: bool = False,
        is_async: bool = False,
        is_base: bool = False,
    ) -> ast.FunctionDef | ast.AsyncFunctionDef:
        if not name:
            name = entry.names[0]
            if is_sink:
                make_method_name = self.MakeEventMethodName
            else:
                make_method_name = self.MakePublicAttributeName
            if not name:
                msg = "method name is not available"
                raise ValueError(msg)
            name = make_method_name(name)

        if (is_setter or is_setter_only) and len(entry.desc.args) != 1:
            msg = "invalid number of args for setter"
            raise ValueError(msg)
        if is_getter and len(entry.desc.args) != 0:
            msg = "invalid number of args for getter"
            raise ValueError(msg)

        if is_getter:
            args = [ast.arg("self", None)]
            args = ast.arguments([], args, None, [], [], None, [])
        elif is_setter:
            arg_type = entry.desc.args[0][0]
            arg_type = self.COMTYPE_TO_ANNOTATION.get(arg_type, "Any")
            args = [
                ast.arg("self", None),
                ast.arg("value", ast.Name(arg_type, ast.Load())),
            ]
            args = ast.arguments([], args, None, [], [], None, [])
        else:
            args = self.MakeArguments(entry.desc, entry.names)

        func_body: list[ast.stmt] = []
        decorator_list: list[ast.expr] = []

        if not is_setter and entry.doc and entry.doc[1]:
            func_body.append(ast.Expr(ast.Constant(entry.doc[1])))

        if is_base:
            if is_getter and is_setter_only:
                func_body.append(
                    ast.Raise(
                        ast.Call(
                            ast.Name("AttributeError", ast.Load()),
                            [
                                ast.JoinedStr(
                                    [
                                        ast.Constant("'"),
                                        ast.FormattedValue(
                                            ast.Name("self", ast.Load()),
                                            conversion=114,
                                        ),
                                        ast.Constant(
                                            f"' object has no attribute '{name}'"
                                        ),
                                    ]
                                )
                            ],
                            [],
                        )
                    )
                )
            else:
                func_body.append(
                    ast.Raise(
                        ast.Call(ast.Name("NotImplementedError", ast.Load()), [], [])
                    )
                )

            if is_getter:
                decorator_list = [ast.Name("property", ast.Load())]
            elif is_setter:
                decorator_list = [
                    ast.Attribute(ast.Name(name, ast.Load()), "setter", ast.Load())
                ]
            else:
                decorator_list = []
        else:
            if not func_body:
                func_body.append(ast.Expr(ast.Constant(Ellipsis)))

            if is_sink:
                decorator_list = [
                    ast.Attribute(
                        ast.Name("decorator", ast.Load()), "event", ast.Load()
                    )
                ]
            elif is_getter or is_setter:
                decorator_list = [
                    ast.Attribute(
                        ast.Name("decorator", ast.Load()), "property", ast.Load()
                    )
                ]
            else:
                decorator_list = [
                    ast.Attribute(
                        ast.Name("decorator", ast.Load()), "method", ast.Load()
                    )
                ]

        return_type = entry.GetResultName()

        if not return_type:
            if is_setter_only:
                return_type = entry.desc.args[0][0]
            else:
                return_type = entry.desc.rettype[0]
            return_type = self.COMTYPE_TO_ANNOTATION.get(return_type, "Any")

        returns = ast.Name(return_type, ast.Load())

        if name == "__iter__":
            returns = ast.Subscript(
                ast.Name("Iterator", ast.Load()), returns, ast.Load()
            )
            self._used_iterator = True
        if name == "__aiter__":
            returns = ast.Subscript(
                ast.Name("AsyncIterator", ast.Load()), returns, ast.Load()
            )
            self._used_async_iterator = True

        if is_async:
            func_def = ast.AsyncFunctionDef(
                name=name,
                args=args,
                body=func_body,
                decorator_list=decorator_list,
                returns=returns,
                type_comment=None,
                type_params=[],
            )
        else:
            func_def = ast.FunctionDef(
                name=name,
                args=args,
                body=func_body,
                decorator_list=decorator_list,
                returns=returns,
                type_comment=None,
                type_params=[],
            )

        return func_def

    def MakeOleItemClassDefs(
        self,
        ole_item,
        class_name: str | None = None,
        *,
        is_async: bool = False,
        is_base: bool = False,
    ) -> list[ast.ClassDef]:
        class_defs = []

        if not class_name:
            class_name = ole_item.python_name
        if not class_name:
            msg = "invalid class name"
            raise ValueError(msg)

        is_sink = ole_item.bIsSink

        class_body: list[ast.stmt] = []
        class_body_assigns = []

        clsid = ole_item.clsid
        clsid_assign = ast.Assign(
            [ast.Name("__CLSID__", ast.Store())],
            ast.Constant(str(clsid)),
        )
        class_body_assigns.append(clsid_assign)

        try:
            progid = ProgIDFromCLSID(clsid)
        except pythoncom.com_error:
            progid = None

        if progid:
            progid_assign = ast.Assign(
                [ast.Name("__PROGID__", ast.Store())], ast.Constant(progid)
            )
            class_body_assigns.append(progid_assign)

        class_body.extend(class_body_assigns)

        if hasattr(ole_item, "mapFuncs"):
            for name, entry in ole_item.mapFuncs.items():
                if entry.desc.desckind != pythoncom.DESCKIND_FUNCDESC:
                    msg = "invalid desckind"
                    raise ValueError(msg)

                if (
                    entry.desc.wFuncFlags & pythoncom.FUNCFLAG_FRESTRICTED
                    and entry.desc.memid != pythoncom.DISPID_NEWENUM
                ):
                    continue
                if entry.desc.funckind != pythoncom.FUNC_DISPATCH:
                    continue
                if entry.hidden:
                    continue

                if entry.desc.memid == pythoncom.DISPID_VALUE:
                    name_lower = "value"
                elif entry.desc.memid == pythoncom.DISPID_NEWENUM:
                    name_lower = "_newenum"
                else:
                    name_lower = name.lower()

                if name_lower == "count":
                    func_def = self.MakeMethodFunctionDef(entry, "__len__")
                    class_body.append(func_def)
                elif name_lower == "item":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__getitem__", is_async=is_async
                    )
                    class_body.append(func_def)
                elif name_lower == "value":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__call__", is_async=is_async
                    )
                    class_body.append(func_def)
                elif name_lower == "_newenum":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__aiter__" if is_async else "__iter__"
                    )
                    class_body.append(func_def)
                else:
                    func_def = self.MakeMethodFunctionDef(
                        entry,
                        is_sink=is_sink,
                        is_async=is_async,
                        is_base=is_base,
                    )
                    class_body.append(func_def)

        prop_names = set()

        if hasattr(ole_item, "propMap"):
            for name, entry in ole_item.propMap.items():
                if entry.desc.memid == pythoncom.DISPID_VALUE:
                    name_lower = "value"
                elif entry.desc.memid == pythoncom.DISPID_NEWENUM:
                    name_lower = "_newenum"
                else:
                    name_lower = name.lower()

                if name_lower == "count":
                    func_def = self.MakeMethodFunctionDef(entry, "__len__")
                    class_body.append(func_def)
                elif name_lower == "item":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__getitem__", is_async=is_async
                    )
                    class_body.append(func_def)
                elif name_lower == "value":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__call__", is_async=is_async
                    )
                    class_body.append(func_def)
                elif name_lower == "_newenum":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__aiter__" if is_async else "__iter__"
                    )
                    class_body.append(func_def)
                else:
                    if is_sink:
                        func_def = self.MakeMethodFunctionDef(
                            entry,
                            is_sink=is_sink,
                            is_async=is_async,
                            is_base=is_base,
                        )
                        class_body.append(func_def)
                    else:
                        func_def = self.MakeMethodFunctionDef(
                            entry,
                            is_getter=True,
                            is_async=is_async,
                            is_base=is_base,
                        )
                        class_body.append(func_def)

                        func_def = self.MakeMethodFunctionDef(
                            entry,
                            is_setter=True,
                            is_async=is_async,
                            is_base=is_base,
                        )
                        class_body.append(func_def)

                    prop_names.add(name)

        if hasattr(ole_item, "propMapGet"):
            for name, entry in ole_item.propMapGet.items():
                if entry.desc.memid == pythoncom.DISPID_VALUE:
                    name_lower = "value"
                elif entry.desc.memid == pythoncom.DISPID_NEWENUM:
                    name_lower = "_newenum"
                else:
                    name_lower = name.lower()

                if name_lower == "count":
                    func_def = self.MakeMethodFunctionDef(entry, "__len__")
                    class_body.append(func_def)
                elif name_lower == "item":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__getitem__", is_async=is_async
                    )
                    class_body.append(func_def)
                elif name_lower == "value":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__call__", is_async=is_async
                    )
                    class_body.append(func_def)
                elif name_lower == "_newenum":
                    func_def = self.MakeMethodFunctionDef(
                        entry, "__aiter__" if is_async else "__iter__"
                    )
                    class_body.append(func_def)
                elif is_sink:
                    func_def = self.MakeMethodFunctionDef(
                        entry,
                        is_sink=is_sink,
                        is_async=is_async,
                        is_base=is_base,
                    )
                    class_body.append(func_def)
                elif name not in prop_names:
                    func_def = self.MakeMethodFunctionDef(
                        entry,
                        is_getter=True,
                        is_async=is_async,
                        is_base=is_base,
                    )
                    class_body.append(func_def)

                prop_names.add(name)

        if hasattr(ole_item, "propMapPut"):
            for name, entry in ole_item.propMapPut.items():
                if is_sink:
                    func_def = self.MakeMethodFunctionDef(
                        entry,
                        is_sink=is_sink,
                        is_async=is_async,
                        is_base=is_base,
                    )
                    class_body.append(func_def)
                else:
                    if name not in prop_names:
                        func_def = self.MakeMethodFunctionDef(
                            entry,
                            is_getter=True,
                            is_setter_only=True,
                            is_async=is_async,
                            is_base=is_base,
                        )
                        class_body.append(func_def)

                    func_def = self.MakeMethodFunctionDef(
                        entry,
                        is_setter=True,
                        is_async=is_async,
                        is_base=is_base,
                    )
                    class_body.append(func_def)

        if not class_body:
            class_body = [ast.Expr(ast.Constant(Ellipsis))]

        class_bases = []

        if hasattr(ole_item, "interfaces"):
            class_bases_interfaces = [
                ast.Name(interface.python_name, ast.Load())
                for interface, flag in ole_item.interfaces
            ]
            class_bases.extend(class_bases_interfaces)

        if hasattr(ole_item, "sources"):
            class_bases_sources = [
                ast.Name(source.python_name, ast.Load())
                for source, flag in ole_item.sources
            ]
            class_bases.extend(class_bases_sources)

        if not is_base and (
            hasattr(ole_item, "interfaces") or hasattr(ole_item, "sources")
        ):
            class_bases.append(ast.Name("AxServeObject", ast.Load()))

        class_def = ast.ClassDef(
            name=class_name,
            bases=class_bases,
            keywords=[],
            body=class_body,
            decorator_list=[],
            type_params=[],
        )
        class_defs.append(class_def)

        return class_defs

    def MakeStubModule(self, clsid):
        import_froms = []
        import_froms.append(ast.ImportFrom("typing", [ast.alias("Any")], 0))
        if self._used_async_iterator:
            import_froms.append(
                ast.ImportFrom("typing", [ast.alias("AsyncIterator")], 0)
            )
        if self._used_iterator:
            import_froms.append(ast.ImportFrom("typing", [ast.alias("Iterator")], 0))
        if self._used_emtpy:
            import_froms.append(ast.ImportFrom("pythoncom", [ast.alias("Empty")], 0))
        if self._used_missing:
            import_froms.append(ast.ImportFrom("pythoncom", [ast.alias("Missing")], 0))
        if self._used_time:
            import_froms.append(ast.ImportFrom("pywintypes", [ast.alias("Time")], 0))
        import_froms.append(
            ast.ImportFrom(
                "axserve.aio.client" if self._is_async else "axserve.client",
                [ast.alias("decorator")],
                0,
            )
        )
        import_froms.append(
            ast.ImportFrom(
                "axserve.aio.client.stub" if self._is_async else "axserve.client.stub",
                [ast.alias("AxServeObject")],
                0,
            )
        )
        class_defs = []

        clsid = str(IID(clsid))
        ole_items = BuildOleItemsForCLSID(clsid)[0]

        if clsid in ole_items:
            ole_items_selected = {}
            item = ole_items[clsid]
            if hasattr(item, "interfaces"):
                for interface, _ in item.interfaces:  # type: ignore
                    ole_items_selected[interface.clsid] = ole_items[interface.clsid]
            if hasattr(item, "sources"):
                for source, _ in item.sources:  # type: ignore
                    ole_items_selected[source.clsid] = ole_items[source.clsid]
            ole_items_selected[clsid] = item
            ole_items = ole_items_selected

        for ole_item in ole_items.values():
            item_class_defs = self.MakeOleItemClassDefs(
                ole_item,
                is_async=self._is_async,
                is_base=self._is_base,
            )
            class_defs.extend(item_class_defs)

        mod_body = import_froms + class_defs
        mod = ast.Module(mod_body, [])
        return mod
