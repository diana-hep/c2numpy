// -*- C++ -*-
//
// Package:    Demo/DemoAnalyzer
// Class:      DemoAnalyzer
// 
/**\class DemoAnalyzer DemoAnalyzer.cc Demo/DemoAnalyzer/plugins/DemoAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  James Pivarski
//         Created:  Wed, 10 May 2017 16:16:22 GMT
//
//

// system include files
#include <memory>
#include <iostream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHitFwd.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"

#include "Demo/DemoAnalyzer/interface/NumpyCommonBlock.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

class DemoAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
public:
  explicit DemoAnalyzer(const edm::ParameterSet&);
  ~DemoAnalyzer();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;

  // ----------member data ---------------------------

  edm::InputTag trackTag;
  edm::EDGetTokenT<reco::TrackCollection> trackToken;

  edm::InputTag muonTag;
  edm::EDGetTokenT<reco::TrackCollection> muonToken;

  NumpyCommonBlock *tracksBlock;
  NumpyCommonBlock *hitsBlock;

  NumpyCommonBlockAccessor<double> *trackermu_qoverp;
  NumpyCommonBlockAccessor<double> *trackermu_qoverp_err;
  NumpyCommonBlockAccessor<double> *trackermu_phi;
  NumpyCommonBlockAccessor<double> *trackermu_eta;
  NumpyCommonBlockAccessor<double> *trackermu_dxy;
  NumpyCommonBlockAccessor<double> *trackermu_dz;
  NumpyCommonBlockAccessor<double> *globalmu_qoverp;
  NumpyCommonBlockAccessor<double> *globalmu_qoverp_err;

  NumpyCommonBlockAccessor<uint64_t> *detid;
  NumpyCommonBlockAccessor<double> *localx;
  NumpyCommonBlockAccessor<double> *localy;
  NumpyCommonBlockAccessor<double> *localx_err;
  NumpyCommonBlockAccessor<double> *localy_err;

  uint64_t tracksIndex = 0;
  uint64_t hitsIndex = 0;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
DemoAnalyzer::DemoAnalyzer(const edm::ParameterSet& iConfig)
  : trackTag(edm::InputTag("ALCARECOMuAlZMuMu", "TrackerOnly")), trackToken(consumes<reco::TrackCollection>(trackTag))
  , muonTag(edm::InputTag("ALCARECOMuAlZMuMu", "GlobalMuon")), muonToken(consumes<reco::TrackCollection>(muonTag))
{
   //now do what ever initialization is needed
   // usesResource("TFileService");

   tracksBlock = (NumpyCommonBlock*)iConfig.getParameter<unsigned long long>("tracks");
   hitsBlock   = (NumpyCommonBlock*)iConfig.getParameter<unsigned long long>("hits");

   trackermu_qoverp     = tracksBlock->newAccessor<double>("trackermu_qoverp");
   trackermu_qoverp_err = tracksBlock->newAccessor<double>("trackermu_qoverp_err");
   trackermu_phi        = tracksBlock->newAccessor<double>("trackermu_phi");
   trackermu_eta        = tracksBlock->newAccessor<double>("trackermu_eta");
   trackermu_dxy        = tracksBlock->newAccessor<double>("trackermu_dxy");
   trackermu_dz         = tracksBlock->newAccessor<double>("trackermu_dz");
   globalmu_qoverp      = tracksBlock->newAccessor<double>("globalmu_qoverp");
   globalmu_qoverp_err  = tracksBlock->newAccessor<double>("globalmu_qoverp_err");

   detid      = hitsBlock->newAccessor<uint64_t>("detid");
   localx     = hitsBlock->newAccessor<double>("localx");
   localy     = hitsBlock->newAccessor<double>("localy");
   localx_err = hitsBlock->newAccessor<double>("localx_err");
   localy_err = hitsBlock->newAccessor<double>("localy_err");
}


DemoAnalyzer::~DemoAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
DemoAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   edm::Handle<reco::TrackCollection> tracks;
   iEvent.getByLabel("ALCARECOMuAlZMuMu", "TrackerOnly", tracks);

   if (tracks->size() != 2)
     return;

   edm::Handle<reco::TrackCollection> muons;
   iEvent.getByLabel("ALCARECOMuAlZMuMu", "GlobalMuon", muons);

   if (muons->size() != 2)
     return;

   // auto t0 = (*tracks)[0].momentum();
   // auto t1 = (*tracks)[1].momentum();
   // auto m0 = (*muons)[0].momentum();
   // auto m1 = (*muons)[1].momentum();
   // double normal = t0.Dot(m0) / sqrt(t0.Mag2()) / sqrt(m0.Mag2()) + t1.Dot(m1) / sqrt(t1.Mag2()) / sqrt(m1.Mag2());
   // double inverted = t0.Dot(m1) / sqrt(t0.Mag2()) / sqrt(m1.Mag2()) + t1.Dot(m0) / sqrt(t1.Mag2()) / sqrt(m0.Mag2());
   // if (inverted > normal)
   //   return;

   reco::TrackCollection::const_iterator track = tracks->begin();
   reco::TrackCollection::const_iterator muon = muons->begin();
   
   for (int i = 0;  i < 2;  i++) {
     if (tracksIndex < trackermu_qoverp->size()) {
       trackermu_qoverp->set(tracksIndex, track->qoverp());
       trackermu_qoverp_err->set(tracksIndex, track->qoverpError());
       trackermu_phi->set(tracksIndex, track->phi());
       trackermu_eta->set(tracksIndex, track->eta());
       trackermu_dxy->set(tracksIndex, track->dxy());
       trackermu_dz->set(tracksIndex, track->dz());
       globalmu_qoverp->set(tracksIndex, muon->qoverp());
       globalmu_qoverp_err->set(tracksIndex, muon->qoverpError());
       tracksIndex++;
       if (tracksIndex == trackermu_qoverp->size())
         tracksBlock->notify(1);
     }

     for (trackingRecHit_iterator hit = muon->recHitsBegin();  hit != muon->recHitsEnd();  ++hit) {
       if ((*hit)->isValid()  &&  (*hit)->rawId() > 600000000) {   // only muon hits have localpositions
         if (hitsIndex < detid->size()) {
           detid->set(hitsIndex, (*hit)->rawId());
           localx->set(hitsIndex, (*hit)->localPosition().x());
           localy->set(hitsIndex, (*hit)->localPosition().y());
           localx_err->set(hitsIndex, (*hit)->localPositionError().xx());
           localy_err->set(hitsIndex, (*hit)->localPositionError().yy());
           hitsIndex++;
           if (hitsIndex == detid->size())
             hitsBlock->notify(1);
         }
       }
     }

     ++track;
     ++muon;
   }
}


// ------------ Method called once each job just before starting event loop  ------------
void 
DemoAnalyzer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
DemoAnalyzer::endJob() 
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
DemoAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DemoAnalyzer);
