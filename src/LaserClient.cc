#include "../interface/LaserClient.h"

#include "DataFormats/EcalDetId/interface/EcalPnDiodeDetId.h"

#include "CondFormats/EcalObjects/interface/EcalDQMStatusHelper.h"

#include "DQM/EcalCommon/interface/EcalDQMCommonUtils.h"
#include "DQM/EcalCommon/interface/MESetMulti.h"

#include <cmath>

namespace ecaldqm {

  LaserClient::LaserClient(edm::ParameterSet const& _workerParams, edm::ParameterSet const& _commonParams) :
    DQWorkerClient(_workerParams, _commonParams, "LaserClient"),
    wlToME_(),
    minChannelEntries_(_workerParams.getUntrackedParameter<int>("minChannelEntries")),
    expectedAmplitude_(0),
    toleranceAmplitude_(0),
    toleranceAmpRMSRatio_(0),
    expectedTiming_(0),
    toleranceTiming_(0),
    toleranceTimRMS_(0),
    expectedPNAmplitude_(0),
    tolerancePNAmp_(0),
    tolerancePNRMSRatio_(0),
    forwardFactor_(_workerParams.getUntrackedParameter<double>("forwardFactor"))
  {
    using namespace std;

    vector<int> laserWavelengths(_commonParams.getUntrackedParameter<vector<int> >("laserWavelengths"));

    unsigned iMEWL(0);
    for(vector<int>::iterator wlItr(laserWavelengths.begin()); wlItr != laserWavelengths.end(); ++wlItr){
      if(*wlItr <= 0 || *wlItr >= 5) throw cms::Exception("InvalidConfiguration") << "Laser Wavelength" << endl;
      wlToME_[*wlItr] = iMEWL++;
    }

    stringstream ss;

    expectedAmplitude_.resize(iMEWL);
    toleranceAmplitude_.resize(iMEWL);
    toleranceAmpRMSRatio_.resize(iMEWL);
    expectedTiming_.resize(iMEWL);
    toleranceTiming_.resize(iMEWL);
    toleranceTimRMS_.resize(iMEWL);
    expectedPNAmplitude_.resize(iMEWL);
    tolerancePNAmp_.resize(iMEWL);
    tolerancePNRMSRatio_.resize(iMEWL);

    for(map<int, unsigned>::iterator wlItr(wlToME_.begin()); wlItr != wlToME_.end(); ++wlItr){
      ss.str("");
      ss << "L" << wlItr->first;

      expectedAmplitude_[wlItr->second] = _workerParams.getUntrackedParameter<double>("expectedAmplitude" + ss.str());
      toleranceAmplitude_[wlItr->second] = _workerParams.getUntrackedParameter<double>("toleranceAmplitude" + ss.str());
      toleranceAmpRMSRatio_[wlItr->second] = _workerParams.getUntrackedParameter<double>("toleranceAmpRMSRatio" + ss.str());
      expectedTiming_[wlItr->second] = _workerParams.getUntrackedParameter<double>("expectedTiming" + ss.str());
      toleranceTiming_[wlItr->second] = _workerParams.getUntrackedParameter<double>("toleranceTiming" + ss.str());
      toleranceTimRMS_[wlItr->second] = _workerParams.getUntrackedParameter<double>("toleranceTimRMS" + ss.str());
      expectedPNAmplitude_[wlItr->second] = _workerParams.getUntrackedParameter<double>("expectedPNAmplitude" + ss.str());
      tolerancePNAmp_[wlItr->second] = _workerParams.getUntrackedParameter<double>("tolerancePNAmp" + ss.str());
      tolerancePNRMSRatio_[wlItr->second] = _workerParams.getUntrackedParameter<double>("tolerancePNRMSRatio" + ss.str());
    }

    map<string, string> replacements;

    unsigned wlPlots[] = {kQuality, kAmplitudeMean, kAmplitudeRMS, kTimingMean, kTimingRMS, kQualitySummary, kPNQualitySummary};
    for(unsigned iS(0); iS < sizeof(wlPlots) / sizeof(unsigned); ++iS){
      unsigned plot(wlPlots[iS]);
      MESetMulti* multi(static_cast<MESetMulti*>(MEs_[plot]));

      for(map<int, unsigned>::iterator wlItr(wlToME_.begin()); wlItr != wlToME_.end(); ++wlItr){
        multi->use(wlItr->second);

        ss.str("");
        ss << wlItr->first;
        replacements["wl"] = ss.str();

        multi->formPath(replacements);
      }
    }

    unsigned wlSources[] = {kAmplitude, kTiming, kPNAmplitude};
    for(unsigned iS(0); iS < sizeof(wlSources) / sizeof(unsigned); ++iS){
      unsigned plot(wlSources[iS]);
      MESetMulti const* multi(static_cast<MESetMulti const*>(sources_[plot]));

      for(map<int, unsigned>::iterator wlItr(wlToME_.begin()); wlItr != wlToME_.end(); ++wlItr){
        multi->use(wlItr->second);

        ss.str("");
        ss << wlItr->first;
        replacements["wl"] = ss.str();

        multi->formPath(replacements);
      }
    }
  }

  void
  LaserClient::beginRun(const edm::Run &, const edm::EventSetup &)
  {
    for(unsigned iME(0); iME < wlToME_.size(); ++iME){
      static_cast<MESetMulti*>(MEs_[kQuality])->use(iME);
      static_cast<MESetMulti*>(MEs_[kQualitySummary])->use(iME);
      static_cast<MESetMulti*>(MEs_[kPNQualitySummary])->use(iME);

      MEs_[kQuality]->resetAll(-1.);
      MEs_[kQualitySummary]->resetAll(-1.);
      MEs_[kPNQualitySummary]->resetAll(-1.);

      MEs_[kQuality]->reset(kUnknown);
      MEs_[kQualitySummary]->reset(kUnknown);
      MEs_[kPNQualitySummary]->reset(kUnknown);
    }
  }

  void
  LaserClient::producePlots()
  {
    uint32_t mask(1 << EcalDQMStatusHelper::LASER_MEAN_ERROR |
                  1 << EcalDQMStatusHelper::LASER_RMS_ERROR |
                  1 << EcalDQMStatusHelper::LASER_TIMING_MEAN_ERROR |
                  1 << EcalDQMStatusHelper::LASER_TIMING_RMS_ERROR);

    for(std::map<int, unsigned>::iterator wlItr(wlToME_.begin()); wlItr != wlToME_.end(); ++wlItr){
      static_cast<MESetMulti*>(MEs_[kQuality])->use(wlItr->second);
      static_cast<MESetMulti*>(MEs_[kQualitySummary])->use(wlItr->second);
      static_cast<MESetMulti*>(MEs_[kAmplitudeMean])->use(wlItr->second);
      static_cast<MESetMulti*>(MEs_[kAmplitudeRMS])->use(wlItr->second);
      static_cast<MESetMulti*>(MEs_[kTimingMean])->use(wlItr->second);
      static_cast<MESetMulti*>(MEs_[kTimingRMS])->use(wlItr->second);
      static_cast<MESetMulti*>(MEs_[kPNQualitySummary])->use(wlItr->second);

      static_cast<MESetMulti const*>(sources_[kAmplitude])->use(wlItr->second);
      static_cast<MESetMulti const*>(sources_[kTiming])->use(wlItr->second);
      static_cast<MESetMulti const*>(sources_[kPNAmplitude])->use(wlItr->second);

      MEs_[kAmplitudeMean]->reset();
      MEs_[kAmplitudeRMS]->reset();
      MEs_[kTimingMean]->reset();
      MEs_[kTimingRMS]->reset();

      MESet::iterator qEnd(MEs_[kQuality]->end());

      MESet::const_iterator tItr(sources_[kTiming]);
      MESet::const_iterator aItr(sources_[kAmplitude]);
      for(MESet::iterator qItr(MEs_[kQuality]->beginChannel()); qItr != qEnd; qItr.toNextChannel()){

        DetId id(qItr->getId());

        bool doMask(applyMask_(kQuality, id, mask));

        aItr = qItr;

        float aEntries(aItr->getBinEntries());

        if(aEntries < minChannelEntries_){
          qItr->setBinContent(doMask ? kMUnknown : kUnknown);
          continue;
        }

        float aMean(aItr->getBinContent());
        float aRms(aItr->getBinError() * sqrt(aEntries));

        MEs_[kAmplitudeMean]->fill(id, aMean);
        MEs_[kAmplitudeRMS]->fill(id, aRms);

        tItr = qItr;

        float tEntries(tItr->getBinEntries());

        if(tEntries < minChannelEntries_) continue;

        float tMean(tItr->getBinContent());
        float tRms(tItr->getBinError() * sqrt(tEntries));

        MEs_[kTimingMean]->fill(id, tMean);
        MEs_[kTimingRMS]->fill(id, tRms);

        float intensity(aMean / expectedAmplitude_[wlItr->second]);
        if(isForward(id)) intensity /= forwardFactor_;

        if(intensity < toleranceAmplitude_[wlItr->second] || aRms > aMean * toleranceAmpRMSRatio_[wlItr->second] ||
           abs(tMean - expectedTiming_[wlItr->second]) > toleranceTiming_[wlItr->second] || tRms > toleranceTimRMS_[wlItr->second])
          qItr->setBinContent(doMask ? kMBad : kBad);
        else
          qItr->setBinContent(doMask ? kMGood : kGood);
      }

      towerAverage_(kQualitySummary, kQuality, 0.2);

      for(unsigned iDCC(0); iDCC < BinService::nDCC; ++iDCC){

        if(memDCCIndex(iDCC + 1) == unsigned(-1)) continue;
        int subdet(0);
        if(iDCC >= kEBmLow && iDCC <= kEBpHigh) subdet = EcalBarrel;
        else subdet = EcalEndcap;

        for(unsigned iPN(0); iPN < 10; ++iPN){
          EcalPnDiodeDetId id(subdet, iDCC + 1, iPN + 1);

          bool doMask(applyMask_(kPNQualitySummary, id, mask));

          float pEntries(sources_[kPNAmplitude]->getBinEntries(id));

          if(pEntries < minChannelEntries_){
            MEs_[kPNQualitySummary]->setBinContent(id, doMask ? kMUnknown : kUnknown);
            continue;
          }

          float pMean(sources_[kPNAmplitude]->getBinContent(id));
          float pRms(sources_[kPNAmplitude]->getBinError(id) * sqrt(pEntries));
          float intensity(pMean / expectedPNAmplitude_[wlItr->second]);

          if(intensity < tolerancePNAmp_[wlItr->second] || pRms > pMean * tolerancePNRMSRatio_[wlItr->second])
            MEs_[kPNQualitySummary]->setBinContent(id, doMask ? kMBad : kBad);
          else
            MEs_[kPNQualitySummary]->setBinContent(id, doMask ? kMGood : kGood);
        }
      }
    }
  }

  /*static*/
  void
  LaserClient::setMEOrdering(std::map<std::string, unsigned>& _nameToIndex)
  {
    _nameToIndex["Quality"] = kQuality;
    _nameToIndex["AmplitudeMean"] = kAmplitudeMean;
    _nameToIndex["AmplitudeRMS"] = kAmplitudeRMS;
    _nameToIndex["TimingMean"] = kTimingMean;
    _nameToIndex["TimingRMS"] = kTimingRMS;
    _nameToIndex["QualitySummary"] = kQualitySummary;
    _nameToIndex["PNQualitySummary"] = kPNQualitySummary;

    _nameToIndex["Amplitude"] = kAmplitude;
    _nameToIndex["Timing"] = kTiming;
    _nameToIndex["PNAmplitude"] = kPNAmplitude;
  }

  DEFINE_ECALDQM_WORKER(LaserClient);
}
