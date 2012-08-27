#ifndef LedClient_H
#define LedClient_H

#include "DQWorkerClient.h"

namespace ecaldqm {

  class LedClient : public DQWorkerClient {
  public:
    LedClient(edm::ParameterSet const&, edm::ParameterSet const&);
    ~LedClient() {}

    void beginRun(const edm::Run&, const edm::EventSetup&);

    void producePlots();

    enum MESets {
      kQuality,
      kAmplitudeMean,
      kAmplitudeRMS,
      kTimingMean,
      kTimingRMS,
      kPNAmplitudeMean,
      kPNAmplitudeRMS,
      kQualitySummary,
      kPNQualitySummary,
      nMESets
    };

    enum Sources {
      kAmplitude,
      kTiming,
      kPNAmplitude,
      nSources
    };
 
    static void setMEOrdering(std::map<std::string, unsigned>&);

  protected:
    std::map<int, unsigned> wlToME_;
    std::map<std::pair<int, int>, unsigned> wlGainToME_;

    int minChannelEntries_;
    std::vector<double> expectedAmplitude_;
    std::vector<double> amplitudeThreshold_;
    std::vector<double> amplitudeRMSThreshold_;
    std::vector<double> expectedTiming_;
    std::vector<double> timingThreshold_;
    std::vector<double> timingRMSThreshold_;
    std::vector<double> expectedPNAmplitude_;
    std::vector<double> pnAmplitudeThreshold_;
    std::vector<double> pnAmplitudeRMSThreshold_;

    float towerThreshold_;

    std::map<std::pair<unsigned, int>, float> ampCorrections_;
  };

}

#endif
