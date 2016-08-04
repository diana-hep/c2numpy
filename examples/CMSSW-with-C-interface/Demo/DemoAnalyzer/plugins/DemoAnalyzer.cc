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
// Original Author:  Valentin Y Kuznetsov
//         Created:  Wed, 03 Aug 2016 19:31:32 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

// track includes
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

// c2numpy
#include "Demo/DemoAnalyzer/interface/c2numpy.h"

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

      // c2numpy
      c2numpy_writer writer;
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

{
   //now do what ever initialization is needed
   // usesResource("TFileService");

   // c2numpy
   c2numpy_init(&writer, "output/trackparams", 1000);
   c2numpy_addcolumn(&writer, "pt", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "eta", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "phi", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "dxy", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "dz", C2NUMPY_FLOAT64);

   // a module register what data it will request from the Event, Chris' suggestion
   consumes<reco::TrackCollection>(edm::InputTag("generalTracks"));

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
  edm::Handle<reco::TrackCollection> tracks;
  iEvent.getByLabel("generalTracks", tracks); 
  edm::LogInfo("Demo") << "number of tracks "<< tracks->size();

  // c2numpy
  for (auto track = tracks->cbegin();  track != tracks->end();  ++track) {
    c2numpy_float64(&writer, track->pt());
    c2numpy_float64(&writer, track->eta());
    c2numpy_float64(&writer, track->phi());
    c2numpy_float64(&writer, track->dxy());
    c2numpy_float64(&writer, track->dz());
  }

}


// ------------ method called once each job just before starting event loop  ------------
void 
DemoAnalyzer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
DemoAnalyzer::endJob() 
{
  // c2numpy
  c2numpy_close(&writer);
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
