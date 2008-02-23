#ifndef EBStatusFlagsClient_H
#define EBStatusFlagsClient_H

/*
 * \file EBStatusFlagsClient.h
 *
 * $Date: 2008/01/18 18:04:05 $
 * $Revision: 1.2 $
 * \author G. Della Ricca
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

class EBStatusFlagsClient : public EBClient {

friend class EBSummaryClient;

public:

/// Constructor
EBStatusFlagsClient(const edm::ParameterSet& ps);

/// Destructor
virtual ~EBStatusFlagsClient();

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

MonitorElement* meh01_[36];

TH2F* h01_[36];

MonitorElement* meh02_[36];

TH1F* h02_[36];

};

#endif