import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

# initialize MessageLogger and output report
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = 'INFO'
process.MessageLogger.categories.append('Demo')
process.MessageLogger.cerr.INFO = cms.untracked.PSet(
    limit = cms.untracked.int32(-1)
)
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )

# process all events
#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
# process 10 events
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(10) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        'file:/afs/cern.ch/cms/Tutorials/TWIKI_DATA/TTJets_8TeV_53X.root'
    )
)

process.dump=cms.EDAnalyzer('EventContentAnalyzer')

process.demo = cms.EDAnalyzer('DemoAnalyzer'
)


# process.p = cms.Path(process.demo)
process.p = cms.Path(process.demo*process.dump)
