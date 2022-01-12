from ctypes import WINFUNCTYPE, CFUNCTYPE, PYFUNCTYPE, c_int32, c_int64, c_uint64, c_long, c_ulong, c_longlong, c_ulonglong, c_void_p, c_uint32, cast
from inspect import signature
from typing import Union, Callable, List, Any, Tuple

import _migi
from migi import get_module_address, _pack_one_argument, make_thiscall
from migi.module import Module


def interceptable_stdcall(func_addr: int, module: Union[str, Module, None] = None):

    def wrapper(func):
        return _InterceptedFunctionMock(func) if _migi.is_mocked() else _InterceptableFunction(func, WINFUNCTYPE, _to_function_address(func_addr, module))

    return wrapper


def cdecl(func_addr: int, module: Union[str, Module, None] = None, *, interceptable=False):
    return interceptable_cdecl(func_addr, module) if interceptable else _c_function_type_decorator(CFUNCTYPE, func_addr, module)


def stdcall(func_addr: Union[int, str], module: Union[str, Module, None] = None, *, interceptable=False):
    return interceptable_stdcall(func_addr, module) if interceptable else _c_function_type_decorator(WINFUNCTYPE, func_addr, module)


def thiscall(func_addr: int, module: Union[str, Module, None] = None, *, stack_size: int = 0, interceptable: bool = False):

    def decorator(func):
        return func if _migi.is_mocked() else _make_thiscall_func(_to_function_address(func_addr, module), stack_size)

    def interceptable_decorator(func):
        return _InterceptedFunctionMock(func) if _migi.is_mocked() else _InterceptableThiscallFunction(func, _to_function_address(func_addr, module), stack_size)

    return interceptable_decorator if interceptable else decorator


def fastcall(func_addr: int, module: Union[str, Module, None] = None, stack_size: int = 0):

    def wrapper(func):
        arg_signatures = [i.annotation for i in signature(func).parameters.values()]

        def fastcall_func(*args):
            args_packed = _pack_arguments(args, arg_signatures)
            args_packed_len = len(args_packed)

            if args_packed_len < 2:
                args_packed += [0] * (2 - args_packed_len)

            rcx, rdx = args_packed[:2]
            return _migi.make_fastcall(_to_function_address(func_addr, module), rcx, rdx, args_packed[2:], stack_size)

        return func if _migi.is_mocked() else fastcall_func

    return wrapper


def interceptable_cdecl(func_addr: int, module: Union[str, Module, None] = None):

    def wrapper(func):
        return _InterceptedFunctionMock(cdecl(func_addr, module)(func)) if _migi.is_mocked() else _InterceptableFunction(func, CFUNCTYPE, _to_function_address(func_addr, module))

    return wrapper


def _c_function_type_decorator(c_function_type: Union[CFUNCTYPE, WINFUNCTYPE], func_addr: Union[int, str], module: Union[str, Module, None] = None):

    def wrapper(func):
        func_type = _to_c_function_type(c_function_type, func)
        return func if _migi.is_mocked() else func_type(_to_function_address(func_addr, module))

    return wrapper


def _to_c_function_type(c_function_type: Union[CFUNCTYPE, WINFUNCTYPE, PYFUNCTYPE], func: Callable):
    func_sig = signature(func)
    return_annotation = func_sig.return_annotation
    return_annotation = return_annotation if return_annotation in (c_int32, c_int64, c_uint64, c_long, c_ulong, c_longlong, c_ulonglong, c_void_p) else c_uint32
    return c_function_type(return_annotation, *[j.annotation for i, j in func_sig.parameters.items()])


def _to_function_address(func_addr: Union[int, str], module: Union[str, Module, None]) -> int:
    if type(module) is str:
        return get_module_address(module, func_addr)
    elif type(module) is Module:
        return module.address + func_addr
    return func_addr


def _make_thiscall_func(function_address: int, stack_size: int = 0):

    def thiscall_func(this_ptr, *args):
        return make_thiscall(function_address, _pack_one_argument(this_ptr)[0], *args, stack_size=stack_size)

    return thiscall_func


def _pack_arguments(objs: Union[List[Any], Tuple[Any]], obj_signatures: List[Any]):
    assert len(objs) == len(obj_signatures)
    return sum([_pack_one_argument(i, j) for i, j in zip(objs, obj_signatures)], [])


class _InterceptedFunctionMock:
    def __init__(self, function_detoured_to: Callable):
        self._function_detoured_to = function_detoured_to

    def __call__(self, *args, **kwargs):
        self._function_detoured_to(*args, **kwargs)

    def intercept(self):
        pass

    def restore(self):
        pass

    def call_original(self, *args, **kwargs):
        self._function_detoured_to(*args, **kwargs)


class _InterceptableFunctionBase:
    def __init__(self, c_function_type, c_function_entry_ptr: int, c_function_detoured_to):
        self._c_function_type = c_function_type
        self._c_function_entry_ptr = c_function_entry_ptr
        self._c_function_detoured_to = c_function_detoured_to
        self._c_function_entry = None
        self._interceptor = None

    def __del__(self):
        if self._interceptor:
            self._interceptor.restore()

    def intercept(self) -> int:
        if not self._interceptor:
            self._interceptor = self._make_interceptor()
        return self._interceptor.intercept()

    def restore(self) -> int:
        return self._interceptor.restore() if self._interceptor else 0

    def __call__(self, *args, **kwargs):
        if not self._c_function_entry:
            self._c_function_entry = self._make_function_entry()
        return self._c_function_entry(*args, **kwargs)

    def call_original(self, *args, **kwargs):
        function_entry = self._c_function_type(self._interceptor.function_entry)
        return function_entry(*args, **kwargs)

    def _make_function_entry(self):
        return self._c_function_type(self._c_function_entry_ptr)

    def _make_interceptor(self):
        return _migi.make_interceptor(self._c_function_entry_ptr, cast(self._c_function_detoured_to, c_void_p).value)


class _InterceptableFunction(_InterceptableFunctionBase):
    def __init__(self, function_detoured_to: Callable, c_function_type: Union[CFUNCTYPE, WINFUNCTYPE, PYFUNCTYPE], function_entry: int):
        c_function_type = _to_c_function_type(c_function_type, function_detoured_to)
        c_function_detoured_to = c_function_type(function_detoured_to)
        super().__init__(c_function_type, function_entry, c_function_detoured_to)


class _InterceptableThiscallFunction(_InterceptableFunctionBase):
    def __init__(self, function_detoured_to: Callable, function_entry: int, stack_size: int):
        c_function_type = _to_c_function_type(WINFUNCTYPE, function_detoured_to)
        c_function_detoured_to = c_function_type(function_detoured_to)
        self._c_function_converted_ptr = _migi.stdcall_to_thiscall(cast(c_function_detoured_to, c_void_p).value)
        self._c_function_converted = c_function_type(self._c_function_converted_ptr)
        self._stack_size = stack_size
        super().__init__(c_function_type, function_entry, c_function_detoured_to)

    def __del__(self):
        _migi.stdcall_to_thiscall_recycle(self._c_function_converted_ptr)
        super().__del__()

    def call_original(self, this_ptr, *args, stack_size=0):
        function_entry = self._interceptor.function_entry
        return make_thiscall(function_entry, this_ptr, *args, stack_size=stack_size or self._stack_size)

    def _make_function_entry(self):
        return _make_thiscall_func(self._c_function_entry_ptr, self._stack_size)

    def _make_interceptor(self):
        return _migi.make_interceptor(self._c_function_entry_ptr, cast(self._c_function_converted, c_void_p).value)
