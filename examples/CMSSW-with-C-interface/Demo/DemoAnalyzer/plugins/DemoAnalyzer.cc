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
// Code examples used to write this code
// https://github.com/cms-sw/cmssw/blob/CMSSW_8_1_X/DataFormats/TrackReco/interface/TrackBase.h
// http://cmslxr.fnal.gov/source/Fireworks/Tracks/plugins/FWTracksRecHitsProxyBuilder.cc
// http://cmslxr.fnal.gov/source/Fireworks/Tracks/src/TrackUtils.cc#0422
// http://cmslxr.fnal.gov/source/Fireworks/Geometry/src/FWRecoGeometryESProducer.cc#0075
// http://cmslxr.fnal.gov/source/Fireworks/Geometry/interface/FWRecoGeometryESProducer.h#0059


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

// c2numpy convertion include
#include "Demo/DemoAnalyzer/interface/c2numpy.h"


// fireworks and geometry includes
#include "TEveGeoShape.h"
#include "TEvePointSet.h"

#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Tracks/interface/TrackUtils.h"
#include "Fireworks/Core/interface/fwLog.h"

#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"

#include <iostream>
using namespace std;

// local function to get Pixel Detector Hits, see
// http://cmslxr.fnal.gov/source/Fireworks/Tracks/src/TrackUtils.cc#0611
void
pixelHits( std::vector<TVector3> &pixelPoints, FWGeometry *geom, const reco::Track &t );
void
pixelHits( std::vector<TVector3> &pixelPoints, FWGeometry *geom, const reco::Track &t )
{
   for( trackingRecHit_iterator it = t.recHitsBegin(), itEnd = t.recHitsEnd(); it != itEnd; ++it )
   {
      const TrackingRecHit* rh = &(**it);
      // -- get position of center of wafer, assuming (0,0,0) is the center
      DetId id = (*it)->geographicalId();
      if( ! geom->contains( id ))
      {
         fwLog( fwlog::kError )
            << "failed to get geometry of Tracker Det with raw id: "
            << id.rawId() << std::endl;

        continue;
      }

      // -- in which detector are we?
      unsigned int subdet = (unsigned int)id.subdetId();

      if(( subdet == PixelSubdetector::PixelBarrel ) || ( subdet == PixelSubdetector::PixelEndcap ))
      {
         if( const SiPixelRecHit* pixel = dynamic_cast<const SiPixelRecHit*>( rh ))
         {
            const SiPixelCluster& c = *( pixel->cluster());
            fireworks::pushPixelCluster( pixelPoints, *geom, id, c, geom->getParameters( id ));
         }
      }
   }
}

// local function to get SiStripClusters, see
// http://cmslxr.fnal.gov/source/Fireworks/Tracks/src/TrackUtils.cc#0422
void
SiStripClusters( std::vector<TVector3> &points, FWGeometry *geom, const reco::Track &t );
void
SiStripClusters( std::vector<TVector3> &points, FWGeometry *geom, const reco::Track &t )
{
//   const edmNew::DetSetVector<SiStripCluster> * allClusters = 0;
   for( trackingRecHit_iterator it = t.recHitsBegin(), itEnd = t.recHitsEnd(); it != itEnd; ++it )
   {
       unsigned int rawid = (*it)->geographicalId();
       if( ! geom->contains( rawid ))
       {
          fwLog( fwlog::kError )
            << "failed to get geometry of SiStripCluster with detid: "
            << rawid << std::endl;

          continue;
       }

       const float* pars = geom->getParameters( rawid );

       // -- get phi from SiStripHit
       auto rechitRef = *it;
       const TrackingRecHit *rechit = &( *rechitRef );
       const SiStripCluster *cluster = fireworks::extractClusterFromTrackingRecHit( rechit );
       if (cluster)
       {
           short firststrip = cluster->firstStrip();
           float localTop[3] = { 0.0, 0.0, 0.0 };
           float localBottom[3] = { 0.0, 0.0, 0.0 };

           fireworks::localSiStrip( firststrip, localTop, localBottom, pars, rawid );

           float globalTop[3];
           float globalBottom[3];
           geom->localToGlobal( rawid, localTop, globalTop, localBottom, globalBottom );
           TVector3 pt( globalTop[0], globalTop[1], globalTop[2] );
           points.push_back( pt );
           TVector3 pb( globalBottom[0], globalBottom[1], globalBottom[2] );
           points.push_back( pb );
       }
   }
}

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

      // FWGeometry
      FWGeometry *geom;

      // hits constrains
      int max_pxhits = 5;
      int max_sihits = 50;
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
   //
   // load geometry
   geom = new FWGeometry();
   const char* filename="cmsGeom10.root";
   geom->loadMap(filename);

   // c2numpy
   c2numpy_init(&writer, "output/params", 1000);

   c2numpy_addcolumn(&writer, "run", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "evt", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "lumi", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "TrackId", C2NUMPY_INTC);
   c2numpy_addcolumn(&writer, "charge", C2NUMPY_INTC);

   c2numpy_addcolumn(&writer, "chi2", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "ndof", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "normalizedChi2", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "qoverp", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "theta", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "lambda", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "dxy", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "d0", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "dsz", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "dz", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "p", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "pt", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "px", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "py", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "pz", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "eta", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "phi", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "vx", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "vy", C2NUMPY_FLOAT64);
   c2numpy_addcolumn(&writer, "vz", C2NUMPY_FLOAT64);

   for (auto i = 0;  i < max_pxhits;  ++i) { // number of pixel hits
       std::ostringstream name;
       name << "pix_" << i;
       for (auto j = 0;  j < 3;  ++j) { // 3 coordinates
           std::ostringstream cname;
           if(j==0) cname << name.str() << "_x";
           if(j==1) cname << name.str() << "_y";
           if(j==2) cname << name.str() << "_z";
           c2numpy_addcolumn(&writer, cname.str().c_str(), C2NUMPY_FLOAT64);
       }
   }

   for (auto i = 0;  i < max_sihits;  ++i) { // number of sistrip clusters
       std::ostringstream name;
       name << "sis_" << i;
       for (auto j = 0;  j < 3;  ++j) { // 3 coordinates
           std::ostringstream cname;
           if(j==0) cname << name.str() << "_x";
           if(j==1) cname << name.str() << "_y";
           if(j==2) cname << name.str() << "_z";
           c2numpy_addcolumn(&writer, cname.str().c_str(), C2NUMPY_FLOAT64);
       }
   }

   // a module register what data it will request from the Event, Chris' suggestion
   consumes<reco::TrackCollection>(edm::InputTag("generalTracks"));

}


DemoAnalyzer::~DemoAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
   geom->clear();
   delete geom;

}


//
// member functions
//

// ------------ method called for each event  ------------
void
DemoAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;


   Handle<reco::TrackCollection> tracks;
   iEvent.getByLabel("generalTracks", tracks);
   LogInfo("Demo") << "number of tracks "<<tracks->size();

   // get event id
   auto eid = iEvent.id();

   // c2numpy
   int tidx = 0;
   for (auto track = tracks->cbegin();  track != tracks->end();  ++track, ++tidx) {
       // extract track parameters

       c2numpy_intc(&writer, eid.run());
       c2numpy_intc(&writer, eid.event());
       c2numpy_intc(&writer, eid.luminosityBlock());
       c2numpy_intc(&writer, tidx);
       c2numpy_intc(&writer, track->charge());

       c2numpy_float64(&writer, track->chi2());
       c2numpy_float64(&writer, track->ndof());
       c2numpy_float64(&writer, track->normalizedChi2());
       c2numpy_float64(&writer, track->qoverp());
       c2numpy_float64(&writer, track->theta());
       c2numpy_float64(&writer, track->lambda());
       c2numpy_float64(&writer, track->dxy());
       c2numpy_float64(&writer, track->d0());
       c2numpy_float64(&writer, track->dsz());
       c2numpy_float64(&writer, track->dz());
       c2numpy_float64(&writer, track->p());
       c2numpy_float64(&writer, track->pt());
       c2numpy_float64(&writer, track->px());
       c2numpy_float64(&writer, track->py());
       c2numpy_float64(&writer, track->pz());
       c2numpy_float64(&writer, track->eta());
       c2numpy_float64(&writer, track->phi());
       c2numpy_float64(&writer, track->vx());
       c2numpy_float64(&writer, track->vy());
       c2numpy_float64(&writer, track->vz());

       // extract Pixel hits
       int npxhits = 0;
       std::vector<TVector3> pxpoints;
       pixelHits( pxpoints, geom, *track );
       for( auto it = pxpoints.begin(), itEnd = pxpoints.end(); it != itEnd; ++it, ++npxhits) {
           if (npxhits<max_pxhits) {
               c2numpy_float64(&writer, it->x());
               c2numpy_float64(&writer, it->y());
               c2numpy_float64(&writer, it->z());
           }
       }
       // fill the rest
       for(auto i=npxhits; i < max_pxhits; ++i){
           c2numpy_float64(&writer, 0.); // init x
           c2numpy_float64(&writer, 0.); // init y
           c2numpy_float64(&writer, 0.); // init z
       }

       // extract SiStripClusters
       int nsihits = 0;
       std::vector<TVector3> sipoints;
       SiStripClusters(sipoints, geom, *track);
       for( auto it = sipoints.begin(), itEnd = sipoints.end(); it != itEnd; ++it, ++nsihits) {
           if (nsihits<max_sihits) {
               c2numpy_float64(&writer, it->x());
               c2numpy_float64(&writer, it->y());
               c2numpy_float64(&writer, it->z());
           }
       }
       // fill the rest
       for(auto i=nsihits; i < max_sihits; ++i){
           c2numpy_float64(&writer, 0.); // init x
           c2numpy_float64(&writer, 0.); // init y
           c2numpy_float64(&writer, 0.); // init z
       }
       LogInfo("Demo") << "track "<< tidx << "# pixel hits" << npxhits << "# sihits" << nsihits;

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
