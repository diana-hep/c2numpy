# Copyright 2017 Jim Pivarski
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

import ctypes

import numpy
import prwlock

class NumpyCommonBlock(object):
    class struct(ctypes.Structure):
        _fields_ = [
            ("numArrays", ctypes.c_uint64),
            ("names", ctypes.POINTER(ctypes.c_char_p)),
            ("types", ctypes.POINTER(ctypes.c_char_p)),
            ("data", ctypes.POINTER(ctypes.POINTER(None))),
            ("lengths", ctypes.POINTER(ctypes.c_uint64)),
            ("locks", ctypes.POINTER(ctypes.POINTER(None))),
            ("statelock", ctypes.POINTER(None)),
            ("state", ctypes.c_uint64)]

    def __init__(self, **arrays):
        self._arrays = arrays

        numArrays = len(arrays)
        names = sorted(arrays)
        c_names = ctypes.ARRAY(ctypes.c_char_p, numArrays)(*[ctypes.c_char_p(x) for x in names])
        c_types = ctypes.ARRAY(ctypes.c_char_p, numArrays)(*[ctypes.c_char_p(bytes(arrays[x].dtype)) for x in names])
        c_data = ctypes.ARRAY(ctypes.POINTER(None), numArrays)(*[
            arrays[x].ctypes.data_as(ctypes.POINTER(None)) for x in names])
        c_lengths = ctypes.ARRAY(ctypes.c_uint64, numArrays)(*[ctypes.c_uint64(numpy.product(arrays[x].shape)) for x in names])
        self._locks = [prwlock.RWLock() for x in names]
        c_locks = ctypes.ARRAY(ctypes.POINTER(None), numArrays)(*[ctypes.cast(x._lock, ctypes.POINTER(None)) for x in self._locks])
        self._statelock = prwlock.RWLock()
        c_statelock = ctypes.cast(self._statelock._lock, ctypes.POINTER(None))
        self._state = ctypes.c_uint64(0)

        self._struct = self.struct(ctypes.c_uint64(numArrays), c_names, c_types, c_data, c_lengths, c_locks, c_statelock, self._state)


