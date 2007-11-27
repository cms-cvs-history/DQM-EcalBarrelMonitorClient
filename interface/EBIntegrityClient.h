#ifndef EBIntegrityClient_H
#define EBIntegrityClient_H

/*
 * \file EBIntegrityClient.h
 *
 * $Date: 2007/11/13 13:20:49 $
 * $Revision: 1.53 $
 * \author G. Della Ricca
 * \author G. Franzoni
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
class MonitorUserInterface;
class DaqMonitorBEInterface;
class EcalCondDBInterface;
class RunIOV;
class MonRunIOV;

class EBIntegrityClient : public EBClient {

friend class EBSummaryClient;

public:

/// Constructor
EBIntegrityClient(const edm::ParameterSet& ps);

/// Destructor
virtual ~EBIntegrityClient();

/// Subscribe/Unsubscribe to Monitoring Elements
void subscribe(void);
void subscribeNew(void);
void unsubscribe(void);

/// softReset
void softReset(void);

/// Analyze
void analyze(void);

/// BeginJob
void beginJob(MonitorUserInterface* mui);

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

std::string prefixME_;

std::vector<int> superModules_;

MonitorUserInterface* mui_;
DaqMonitorBEInterface* dbe_;

TH1F* h00_;

TH2F* h01_[36];
TH2F* h02_[36];
TH2F* h03_[36];
TH2F* h04_[36];
TH2F* h05_[36];
TH2F* h06_[36];
TH2F* h07_[36];
TH2F* h08_[36];
TH2F* h09_[36];
TH2F* h10_[36];

MonitorElement* meg01_[36];
MonitorElement* meg02_[36];

TH2F* h_[36];
TH2F* hmem_[36];

// Quality criteria for data integrity

float threshCry_;

const static int chNum [5][5];

};

#endif
