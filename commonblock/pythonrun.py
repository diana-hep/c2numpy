# sharable data

import numpy

# normal CMSSW configuration

import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))

process.source = cms.Source(
    "PoolSource", fileNames = cms.untracked.vstring("file:MuAlZMuMu-2016H-002590494DA0.root"))

process.demo = cms.EDAnalyzer("DemoAnalyzer")
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

print "FIRST TIME DONE"

cmsswThread.run()

print "SECOND TIME DONE"
