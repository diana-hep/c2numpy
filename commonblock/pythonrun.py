# sharable data

import numpy
import commonblock

tracks = commonblock.NumpyCommonBlock(
    trackermu_qoverp     = numpy.zeros(1000, dtype=numpy.double),
    trackermu_qoverp_err = numpy.zeros(1000, dtype=numpy.double),
    trackermu_phi        = numpy.zeros(1000, dtype=numpy.double),
    trackermu_eta        = numpy.zeros(1000, dtype=numpy.double),
    trackermu_dxy        = numpy.zeros(1000, dtype=numpy.double),
    trackermu_dz         = numpy.zeros(1000, dtype=numpy.double),
    globalmu_qoverp      = numpy.zeros(1000, dtype=numpy.double),
    globalmu_qoverp_err  = numpy.zeros(1000, dtype=numpy.double))

hits = commonblock.NumpyCommonBlock(
    detid      = numpy.zeros(5000, dtype=numpy.uint64),
    localx     = numpy.zeros(5000, dtype=numpy.double),
    localy     = numpy.zeros(5000, dtype=numpy.double),
    localx_err = numpy.zeros(5000, dtype=numpy.double),
    localy_err = numpy.zeros(5000, dtype=numpy.double))

# CMSSW configuration

import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))

process.source = cms.Source(
    "PoolSource", fileNames = cms.untracked.vstring("file:MuAlZMuMu-2016H-002590494DA0.root"))

process.demo = cms.EDAnalyzer(
    "DemoAnalyzer",
    tracks = cms.uint64(tracks.pointer()),    # pass the arrays to C++ as a pointer
    hits   = cms.uint64(hits.pointer()))

process.p = cms.Path(process.demo)

# run it in Python

import threading
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
        print "CMSSW finished"

cmsswThread = CMSSWThread(process)
cmsswThread.start()

print "running"

tracks.wait(1)   # CMSSW notifies that it has filled the tracks array
print tracks.pandas()
