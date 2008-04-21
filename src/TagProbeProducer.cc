// -*- C++ -*-
//
// Package:    TagProbeProducer
// Class:      TagProbeProducer
// 
/**\class TagProbeProducer TagProbeProducer.cc MuonAnalysis/TagProbeProducer/src/TagProbeProducer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Nadia Adam
//         Created:  Wed Apr 16 09:46:30 CDT 2008
// $Id$
//
//


// User includes
#include "MuonAnalysis/TagAndProbe/interface/TagProbeProducer.h"
#include "AnalysisDataFormats/TagAndProbe/interface/CandidateAssociation.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Math/GenVector/VectorUtil.h"


//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
TagProbeProducer::TagProbeProducer(const edm::ParameterSet& iConfig)
{
   tagCollection_   = iConfig.getParameter<edm::InputTag>("TagCollection");
   probeCollection_ = iConfig.getParameter<edm::InputTag>("ProbeCollection");

   massMinCut_      = iConfig.getUntrackedParameter<double>("MassMinCut",50.0);
   massMaxCut_      = iConfig.getUntrackedParameter<double>("MassMaxCut",120.0);
   delRMinCut_      = iConfig.getUntrackedParameter<double>("DelRMinCut",0.0);
   delRMaxCut_      = iConfig.getUntrackedParameter<double>("DelRMaxCut",10000.0);

   requireOS_       = iConfig.getUntrackedParameter<bool>("RequireOS",true);

   produces<reco::CandCandAssociationCollection>();
}


TagProbeProducer::~TagProbeProducer()
{
 
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
TagProbeProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   using namespace reco;

   // We need the output Muon association collection to fill
   std::auto_ptr<CandCandAssociationCollection> 
      muonTPCollection( new CandCandAssociationCollection );
 
   // Read in the tag muons
   edm::Handle<CandidateCollection> tags;
   try{ iEvent.getByLabel( tagCollection_, tags ); }
   catch(...)
   {
      LogWarning("TagProbe") << "Could not extract tag muons with input tag "
				 << tagCollection_;
   }

   // Read in the probe muons
   edm::Handle<CandidateCollection> probes;
   try{ iEvent.getByLabel( probeCollection_, probes ); }
   catch(...)
   {
      LogWarning("TagProbe") << "Could not extract probe muons with input tag "
				 << probeCollection_;
   }


   // Loop over Tag and associate with Probes
   if( tags.isValid() && probes.isValid() )
   {
      int itag = 0;
      CandidateCollection::const_iterator tag = (*tags).begin();
      for( ; tag != (*tags).end(); ++tag ) 
      {  
	 CandidateRef tagRef(tags,itag);
	 ++itag;

	 int iprobe = 0;
	 CandidateCollection::const_iterator probe = (*probes).begin();
	 for( ; probe != (*probes).end(); ++probe ) 
	 {
	    CandidateRef probeRef(probes,iprobe);
	    ++iprobe;
	    
	    // Tag-Probe invariant mass cut
	    double invMass = ROOT::Math::VectorUtil::InvariantMass(tag->p4(), probe->p4());
	    if( invMass < massMinCut_ ) continue;
	    if( invMass > massMaxCut_ ) continue;

	    // Tag-Probe deltaR cut
	    double delR = deltaR<double>(tag->eta(),tag->phi(),probe->eta(),probe->phi());
	    if( delR < delRMinCut_ ) continue;
	    if( delR > delRMaxCut_ ) continue;

	    // Tag-Probe opposite sign
	    int sign = tag->charge() * probe->charge();
	    if( requireOS_ && sign > 0 ) continue;

            muonTPCollection->insert( tagRef, probeRef );

	 }	 
      }
   }

   // Finally put the tag probe collection in the event
   iEvent.put( muonTPCollection );
}

// ------------ method called once each job just before starting event loop  ------------
void 
TagProbeProducer::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
TagProbeProducer::endJob() {
}

//define this as a plug-in
//DEFINE_FWK_MODULE(TagProbeProducer);
