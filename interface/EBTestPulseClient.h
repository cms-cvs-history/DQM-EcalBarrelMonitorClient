#ifndef EBTestPulseClient_H
#define EBTestPulseClient_H

/*
 * \file EBTestPulseClient.h
 *
 * $Date: 2006/03/03 09:02:42 $
 * $Revision: 1.22 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"
#include "DQMServices/Daemon/interface/MonitorDaemon.h"

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"
#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"
#include "OnlineDB/EcalCondDB/interface/MonRunIOV.h"

#include "OnlineDB/EcalCondDB/interface/MonTestPulseDat.h"
#include "OnlineDB/EcalCondDB/interface/MonPulseShapeDat.h"

#include "OnlineDB/EcalCondDB/interface/MonPNMGPADat.h"

#include "TROOT.h"
#include "TStyle.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace cms;
using namespace edm;
using namespace std;

class EBTestPulseClient{

public:

/// Constructor
EBTestPulseClient(const ParameterSet& ps, MonitorUserInterface* mui);

/// Destructor
virtual ~EBTestPulseClient();

/// Subscribe/Unsubscribe to Monitoring Elements
void subscribe(void);
void subscribeNew(void);
void unsubscribe(void);

/// Analyze
void analyze(void);

/// BeginJob
void beginJob(void);

/// EndJob
void endJob(void);

/// BeginRun
void beginRun(void);

/// EndRun
void endRun(void);

/// Setup
void setup(void);

/// Cleanup
void cleanup(void);

/// HtmlOutput
void htmlOutput(int run, int jsm, string htmlDir, string htmlName);

/// WriteDB
void writeDb(EcalCondDBInterface* econn, MonRunIOV* moniov);

private:

int ievt_;
int jevt_;

bool collateSources_;
bool cloneME_;

bool verbose_;

MonitorUserInterface* mui_;

CollateMonitorElement* me_ha01_[36];
CollateMonitorElement* me_ha02_[36];
CollateMonitorElement* me_ha03_[36];

CollateMonitorElement* me_hs01_[36];
CollateMonitorElement* me_hs02_[36];
CollateMonitorElement* me_hs03_[36];

CollateMonitorElement* me_he01_[36];
CollateMonitorElement* me_he02_[36];
CollateMonitorElement* me_he03_[36];

TProfile2D* ha01_[36];
TProfile2D* ha02_[36];
TProfile2D* ha03_[36];

TProfile2D* hs01_[36];
TProfile2D* hs02_[36];
TProfile2D* hs03_[36];

TH2F* he01_[36];
TH2F* he02_[36];
TH2F* he03_[36];

TH2F* g01_[36];
TH2F* g02_[36];
TH2F* g03_[36];

TH1F* a01_[36];
TH1F* a02_[36];
TH1F* a03_[36];

CollateMonitorElement* me_i01_[36];
CollateMonitorElement* me_i02_[36];
CollateMonitorElement* me_i03_[36];
CollateMonitorElement* me_i04_[36];

TProfile2D* i01_[36];
TProfile2D* i02_[36];
TProfile2D* i03_[36];
TProfile2D* i04_[36];

// Quality check on crystals, one per each gain

float amplitudeThreshold_;
float RMSThreshold_;
float threshold_on_AmplitudeErrorsNumber_;

};

#endif
