/*
 * ChargedHadronPFTrackIsolationProducer
 *
 * Author: Andreas Hinzmann
 *
 * Associates PF isolation flag to charged hadron candidates
 *
 */

#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlockElement.h"

class ChargedHadronPFTrackIsolationProducer : public edm::global::EDProducer<> 
{
public:

  explicit ChargedHadronPFTrackIsolationProducer(const edm::ParameterSet& cfg);
  ~ChargedHadronPFTrackIsolationProducer() {}
  void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;
  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);

private:
  // input collection
  edm::InputTag srccandidates_;
  edm::EDGetTokenT<edm::View<reco::PFCandidate> > candidates_token;
  double minTrackPt_;
  double minRawCaloEnergy_;

};

ChargedHadronPFTrackIsolationProducer::ChargedHadronPFTrackIsolationProducer(const edm::ParameterSet& cfg)
{
  srccandidates_ = cfg.getParameter<edm::InputTag>("src");
  candidates_token = consumes<edm::View<reco::PFCandidate> >(srccandidates_);
  minTrackPt_ = cfg.getParameter<double>("minTrackPt");
  minRawCaloEnergy_ = cfg.getParameter<double>("minRawCaloEnergy");
  
  produces<edm::ValueMap<bool> >();
}

void ChargedHadronPFTrackIsolationProducer::produce(edm::StreamID, edm::Event& evt, const edm::EventSetup& es) const
{
  // get a view of our candidates via the base candidates
  typedef edm::View<reco::PFCandidate> PFCandidateView;
  edm::Handle<PFCandidateView> candidates;
  evt.getByToken(candidates_token, candidates);
  
  std::vector<bool> values;

  for( auto const& c : *candidates ) {
    // Check that there is only one track in the block.
    unsigned int nTracks = 0;
    if ((c.particleId()==1) && (c.pt()>minTrackPt_) && ((c.rawEcalEnergy()+c.rawHcalEnergy())>minRawCaloEnergy_)) {
      const reco::PFCandidate::ElementsInBlocks& theElements = c.elementsInBlocks();
      if( theElements.empty() ) continue;
      const reco::PFBlockRef blockRef = theElements[0].first;
      const edm::OwnVector<reco::PFBlockElement>& elements = blockRef->elements();
      // Find the tracks in the block
      for( auto const & ele : elements ) {
         reco::PFBlockElement::Type type = ele.type();
         if( type== reco::PFBlockElement::TRACK)
           nTracks++;
      }
    }
    values.push_back((nTracks==1));
  }

  std::unique_ptr<edm::ValueMap<bool> > out(new edm::ValueMap<bool>());
  edm::ValueMap<bool>::Filler filler(*out);
  filler.insert(candidates,values.begin(),values.end());
  filler.fill();
  evt.put(std::move(out));
}

void ChargedHadronPFTrackIsolationProducer::fillDescriptions(edm::ConfigurationDescriptions & descriptions) {
   edm::ParameterSetDescription desc;
   desc.add<edm::InputTag>("src", edm::InputTag("particleFlow"));
   desc.add<double>("minTrackPt", 1);
   desc.add<double>("minRawCaloEnergy", 0.5);
   descriptions.add("chargedHadronPFTrackIsolation", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(ChargedHadronPFTrackIsolationProducer);
