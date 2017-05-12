# sharable data

import numpy
import ctypes
import prwlock

class NumpyCommonBlock(ctypes.Structure):
    _fields_ = [
        ("numArrays", ctypes.c_uint64),
        ("names", ctypes.POINTER(ctypes.c_char_p)),
        ("types", ctypes.POINTER(ctypes.c_char_p)),
        ("data", ctypes.POINTER(ctypes.POINTER(None))),
        ("lengths", ctypes.POINTER(ctypes.c_uint64)),
        ("locks", ctypes.POINTER(ctypes.POINTER(None))),
        ("statelock", ctypes.POINTER(None)),
        ("state", ctypes.c_uint64)]

nametotype = [("one", str(numpy.dtype(numpy.double))),
              ("two", str(numpy.dtype(numpy.double))),
              ("three", str(numpy.dtype(numpy.double)))]

numArrays = ctypes.c_uint64(len(nametotype))
names = ctypes.ARRAY(ctypes.c_char_p, len(nametotype))(*[ctypes.c_char_p(n) for n, v in nametotype])
types = ctypes.ARRAY(ctypes.c_char_p, len(nametotype))(*[ctypes.c_char_p(v) for n, v in nametotype])

data = ctypes.ARRAY(ctypes.POINTER(None), 3)(
    numpy.array([1.1]).ctypes.data_as(ctypes.POINTER(None)),
    numpy.array([2.2, 2.2]).ctypes.data_as(ctypes.POINTER(None)),
    numpy.array([3.3, 3.3, 3.3]).ctypes.data_as(ctypes.POINTER(None)))

lengths = ctypes.ARRAY(ctypes.c_uint64, 3)(ctypes.c_uint64(1), ctypes.c_uint64(2), ctypes.c_uint64(3))

locks = [prwlock.RWLock(), prwlock.RWLock(), prwlock.RWLock()]
c_locks = ctypes.ARRAY(ctypes.POINTER(None), len(locks))(*[ctypes.cast(x._lock, ctypes.POINTER(None)) for x in locks])

statelock = prwlock.RWLock()
c_statelock = ctypes.cast(statelock._lock, ctypes.POINTER(None))
state = ctypes.c_uint64(0)

cb = NumpyCommonBlock(numArrays, names, types, data, lengths, c_locks, c_statelock, state)

# normal CMSSW configuration

import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))

process.source = cms.Source(
    "PoolSource", fileNames = cms.untracked.vstring("file:testagain_cmssw.root"))

process.demo = cms.EDAnalyzer("DemoAnalyzer", cb = cms.uint64(ctypes.addressof(cb)))
process.p = cms.Path(process.demo)

# run it in Python

import threading
import time

import libFWCorePythonFramework
import libFWCorePythonParameterSet

class CMSSWThread(threading.Thread):
    def __init__(self, process):
        super(CMSSWThread, self).__init__()
        self.process = process

    def run(self):
        processDesc = libFWCorePythonParameterSet.ProcessDesc()
        self.process.fillProcessDesc(processDesc.pset())

        cppProcessor = libFWCorePythonFramework.PythonEventProcessor(processDesc)
        cppProcessor.run()

cmsswThread = CMSSWThread(process)
cmsswThread.run()

# count = 0
# while cmsswThread.isAlive():
#     lock.acquire_read()
#     print "python x =", x.value
#     lock.release()

#     lock.acquire_write()
#     x.value += 1
#     count += 1
#     lock.release()

# cmsswThread.join()

# print "x =", x.value
# print "count =", count
