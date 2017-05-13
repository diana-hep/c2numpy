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
import time

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

    def __init__(self, *order, **arrays):
        self._arrays = arrays

        numArrays = len(arrays)

        self._order = []
        remaining = set(arrays)
        for name in order:
            if name in remaining:
                self._order.append(name)
                remaining.discard(name)
        self._order.extend(sorted(remaining))
            
        c_names = ctypes.ARRAY(ctypes.c_char_p, numArrays)(*[ctypes.c_char_p(x) for x in self._order])
        c_types = ctypes.ARRAY(ctypes.c_char_p, numArrays)(*[ctypes.c_char_p(bytes(arrays[x].dtype)) for x in self._order])
        c_data = ctypes.ARRAY(ctypes.POINTER(None), numArrays)(*[
            arrays[x].ctypes.data_as(ctypes.POINTER(None)) for x in self._order])
        c_lengths = ctypes.ARRAY(ctypes.c_uint64, numArrays)(*[ctypes.c_uint64(numpy.product(arrays[x].shape)) for x in self._order])
        self._locks = [prwlock.RWLock() for x in self._order]
        c_locks = ctypes.ARRAY(ctypes.POINTER(None), numArrays)(*[ctypes.cast(x._lock, ctypes.POINTER(None)) for x in self._locks])
        self._statelock = prwlock.RWLock()
        c_statelock = ctypes.cast(self._statelock._lock, ctypes.POINTER(None))

        self._struct = self.struct(ctypes.c_uint64(numArrays), c_names, c_types, c_data, c_lengths, c_locks, c_statelock, ctypes.c_uint64(0))

    def pointer(self):
        return ctypes.addressof(self._struct)

    def pandas(self):
        import pandas
        for lock in self._locks:
            lock.acquire_read()
        try:
            return pandas.DataFrame(self._arrays, columns=self._order)
        finally:
            for lock in self._locks:
                lock.release()

    class Accessor(object):
        def __init__(self, lock, array):
            self.lock = lock
            self.array = array

        def __getitem__(self, slice):
            self.lock.acquire_read()
            try:
                return self.array[slice]
            finally:
                self.lock.release()

        def __setitem__(self, slice, value):
            self.lock.acquire_write()
            try:
                self.array[slice] = value
            finally:
                self.lock.release()

        def size(self):
            return len(self.array)

    def accessor(self, name):
        return self.Accessor(self._locks[self._order.index(name)], self._arrays[name])

    def _wait(self, condition):
        self._statelock.acquire_read()
        try:
            current = self._struct.state
        finally:
            self._statelock.release()

        while condition(current):
            time.sleep(0.001)
            self._statelock.acquire_read()
            try:
                current = self._struct.state
            finally:
                self._statelock.release()

    def wait(self, forstate):
        self._wait(lambda current: current != forstate)

    def waitmask(self, formask):
        self._wait(lambda current: not (current & formask))

    def notify(self, newstate):
        self._statelock.acquire_write()
        try:
            self._struct.state = newstate
        finally:
            self._statelock.release()
