#ifndef EBPedestalOnlineClient_H
#define EBPedestalOnlineClient_H

/*
 * \file EBPedestalOnlineClient.h
 *
 * $Date: 2006/03/05 09:50:40 $
 * $Revision: 1.6 $
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

#include "OnlineDB/EcalCondDB/interface/MonPedestalsOnlineDat.h"

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

class EBPedestalOnlineClient{

public:

/// Constructor
EBPedestalOnlineClient(const ParameterSet& ps, MonitorUserInterface* mui);

/// Destructor
virtual ~EBPedestalOnlineClient();

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

CollateMonitorElement* me_h03_[36];

MonitorElement* meh03_[36];

TProfile2D* h03_[36];

TH2F* g03_[36];

TH1F* p03_[36];

TH1F* r03_[36];

// Quality check on crystals, one per each gain

float expectedMean_;
float discrepancyMean_;
float RMSThreshold_;

};

#endif
