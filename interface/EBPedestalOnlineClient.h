#ifndef EBPedestalOnlineClient_H
#define EBPedestalOnlineClient_H

/*
 * \file EBPedestalOnlineClient.h
 *
 * $Date: 2008/01/18 18:04:04 $
 * $Revision: 1.37 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <vector>
#include <string>

#include "TROOT.h"
#include "TProfile2D.h"
#include "TH1F.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQM/EcalBarrelMonitorClient/interface/EBClient.h"

class MonitorElement;
class DaqMonitorBEInterface;
class EcalCondDBInterface;
class RunIOV;
class MonRunIOV;

class EBPedestalOnlineClient : public EBClient {

friend class EBSummaryClient;

public:

/// Constructor
EBPedestalOnlineClient(const edm::ParameterSet& ps);

/// Destructor
virtual ~EBPedestalOnlineClient();

/// Analyze
void analyze(void);

/// BeginJob
void beginJob(DaqMonitorBEInterface* mui);

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
void htmlOutput(int run, std::string htmlDir, std::string htmlName);

/// WriteDB
bool writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov);

/// Get Functions
inline int getEvtPerJob() { return ievt_; }
inline int getEvtPerRun() { return jevt_; }

private:

int ievt_;
int jevt_;

bool cloneME_;

bool verbose_;

bool enableMonitorDaemon_;

bool enableCleanup_;

std::string prefixME_;

std::vector<int> superModules_;

DaqMonitorBEInterface* dbe_;

MonitorElement* meh03_[36];

TProfile2D* h03_[36];

MonitorElement* meg03_[36];

MonitorElement* mep03_[36];

MonitorElement* mer03_[36];

// Quality check on crystals, one per each gain

float expectedMean_;
float discrepancyMean_;
float RMSThreshold_;

};

#endif
