#ifndef EBClusterClient_H
#define EBClusterClient_H

/*
 * \file EBClusterClient.h
 *
 * $Date: 2007/11/08 15:43:50 $
 * $Revision: 1.15 $
 * \author G. Della Ricca
 * \author F. Cossutti
 * \author E. Di Marco
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

class EBClusterClient : public EBClient {

friend class EBSummaryClient;

public:

/// Constructor
EBClusterClient(const edm::ParameterSet& ps);

/// Destructor
virtual ~EBClusterClient();

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
void htmlOutput(int run, string htmlDir, string htmlName);

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

string prefixME_;

vector<int> superModules_;

MonitorUserInterface* mui_;
DaqMonitorBEInterface* dbe_;

TH1F* h01_[3];
TProfile2D* h02_[2];
TProfile* h02ProjEta_[2], *h02ProjPhi_[2]; 
TH2F* h03_;
TH1F* h03ProjEta_, *h03ProjPhi_;
TProfile2D* h04_;
TProfile* h04ProjEta_, *h04ProjPhi_;
TH1F* i01_[3];

TH1F* s01_[3];

};

#endif
