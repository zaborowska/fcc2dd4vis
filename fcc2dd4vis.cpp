
#include <iostream>
#include <vector>

#include "DDG4/DDG4Dict.h"
#include "DDG4/Geant4Data.h"
#include "DDG4/Geant4Hits.h"
#include "DDEve/DDEveEventData.h" 

// podio specific includes
#include "podio/EventStore.h"
#include "podio/ROOTReader.h"

#include "datamodel/TrackClusterCollection.h"
#include "datamodel/TrackHitCollection.h"

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"

using DD4hep::Geometry::Position;
using DD4hep::Simulation::Geant4Tracker;

int main() {
  auto store = podio::EventStore();
  auto reader = podio::ROOTReader();

  std::string inFilename = "output.root";
  std::cout<<"Read FCCSW data file '"<< inFilename <<"' ..."<<std::endl;
  reader.openFile(inFilename);

  gSystem->Load("libDDG4IO");
  std::vector<DD4hep::Simulation::Geant4Tracker::Hit*> hv;
  auto p = new Position(1,2,3);
  
  // needed for debug only
  TRandom* random1 = new TRandom;
  TFile* f = new TFile("test.root", "RECREATE");
  TTree* T = new TTree("EVENT", "My test tree");
  T->SetAutoSave(0);
  T->Branch("clusters", &hv);

  store.setReader(&reader);
  unsigned nevents= reader.getEntries();
  std::cout<<"found "<<nevents<<" events ..."<<std::endl;
  for(unsigned iev=0; iev<nevents; ++iev) {
    const fcc::TrackClusterCollection* clustercoll(nullptr);
    bool clusterPresent = store.get("clusters", clustercoll);
    if (clusterPresent) {
      for(const auto& cluster : *clustercoll){
        DD4hep::Simulation::Geant4Tracker::Hit* hd = new DD4hep::Simulation::Geant4Tracker::Hit();
        fcc::BareCluster core = cluster.Core();
        p->SetXYZ(core.position.X, core.position.Y, core.position.Z);
        hd->position = *p;
        hd->energyDeposit = core.Energy;
        // time not used in dd4hep format
        // direction (momentum) of hit not provided by edm 
        hv.push_back(hd);
      }
    }
    T->Fill();
    hv.clear();
    store.clear();
    reader.endOfEvent();
  }
  T->Write();
  delete f;

  return 0;
}
