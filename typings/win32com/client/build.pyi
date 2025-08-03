from _typeshed import Incomplete

class NotSupportedException(Exception): ...

DropIndirection: str
NoTranslateTypes: Incomplete
NoTranslateMap: Incomplete

class MapEntry:
    dispid: Incomplete
    desc: Incomplete
    names: Incomplete
    doc: Incomplete
    resultCLSID: Incomplete
    resultDocumentation: Incomplete
    wasProperty: int
    hidden: Incomplete
    def __init__(
        self,
        desc_or_id,
        names=None,
        doc=None,
        resultCLSID=...,
        resultDoc=None,
        hidden: int = 0,
    ) -> None: ...
    def GetResultCLSID(self): ...
    def GetResultCLSIDStr(self): ...
    def GetResultName(self): ...

class OleItem:
    typename: str
    doc: Incomplete
    python_name: Incomplete
    bWritten: int
    bIsDispatch: int
    bIsSink: int
    clsid: Incomplete
    co_class: Incomplete
    def __init__(self, doc=None) -> None: ...

class DispatchItem(OleItem):
    typename: str
    propMap: Incomplete
    propMapGet: Incomplete
    propMapPut: Incomplete
    mapFuncs: Incomplete
    defaultDispatchName: Incomplete
    hidden: int
    def __init__(
        self, typeinfo=None, attr=None, doc=None, bForUser: int = 1
    ) -> None: ...
    clsid: Incomplete
    bIsDispatch: Incomplete
    def Build(self, typeinfo, attr, bForUser: int = 1) -> None: ...
    def CountInOutOptArgs(self, argTuple): ...
    def MakeFuncMethod(self, entry, name, bMakeClass: int = 1): ...
    def MakeDispatchFuncMethod(self, entry, name, bMakeClass: int = 1): ...
    def MakeVarArgsFuncMethod(self, entry, name, bMakeClass: int = 1): ...

class VTableItem(DispatchItem):
    vtableFuncs: Incomplete
    def Build(self, typeinfo, attr, bForUser: int = 1): ...

class LazyDispatchItem(DispatchItem):
    typename: str
    clsid: Incomplete
    def __init__(self, attr, doc) -> None: ...

typeSubstMap: Incomplete
valid_identifier_chars: Incomplete

def demunge_leading_underscores(className): ...
def MakePublicAttributeName(className, is_global: bool = False): ...
def MakeDefaultArgRepr(defArgVal): ...
def BuildCallList(
    fdesc,
    names,
    defNamedOptArg,
    defNamedNotOptArg,
    defUnnamedArg,
    defOutArg,
    is_comment: bool = False,
): ...
