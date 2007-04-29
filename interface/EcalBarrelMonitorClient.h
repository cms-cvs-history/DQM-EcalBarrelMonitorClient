#ifndef EcalBarrelMonitorClient_H
#define EcalBarrelMonitorClient_H

/*
 * \file EcalBarrelMonitorClient.h
 *
 * $Date: 2007/04/10 09:26:38 $
 * $Revision: 1.70 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <string>
#include <vector>
#include <map>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "DQMServices/Core/interface/MonitorUserInterface.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"
#include "OnlineDB/EcalCondDB/interface/MonRunIOV.h"

#include <DQM/EcalBarrelMonitorClient/interface/EBClient.h>

#include "DQMServices/Core/interface/QTestStatus.h"
#include "DQMServices/QualityTests/interface/QCriterionRoot.h"

#include <DQM/EcalBarrelMonitorClient/interface/EBSummaryClient.h>

#include "TROOT.h"
#include "TH1.h"

class EcalBarrelMonitorClient: public edm::EDAnalyzer{

public:

/// Constructor
EcalBarrelMonitorClient(const edm::ParameterSet & ps);
EcalBarrelMonitorClient(const edm::ParameterSet & ps, MonitorUserInterface* mui);
  
/// Destructor
~EcalBarrelMonitorClient();
  
/// Subscribe/Unsubscribe to Monitoring Elements
void subscribe(void);
void subscribeNew(void);
void unsubscribe(void);

/// softReset
void softReset(void);
  
// Initialize
void initialize(const edm::ParameterSet & ps);

/// Analyze
void analyze(void);
void analyze(const edm::Event & e, const edm::EventSetup & c){ this->analyze(); }
  
/// BeginJob
void beginJob(void);
void beginJob(const edm::EventSetup & c){ this->beginJob(); }
  
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
void htmlOutput(bool current=false);
  
/// BeginRunDB
void beginRunDb(void);
  
/// WriteDB
void writeDb(void);

/// EndRunDB
void endRunDb(void);

inline int                      getEvtPerJob()      { return( ievt_ ); }
inline int                      getEvtPerRun()      { return( jevt_ ); }
inline int                      getEvt( void )      { return( evt_ ); }
inline int                      getRun( void )      { return( run_ ); }
inline string                   getRunType( void )  { return( runtype_ == -1 ? "UNKNOWN" : runTypes_[runtype_] ); }
inline vector<string>           getRunTypes( void ) { return( runTypes_ ); }
inline const vector<EBClient*>  getClients()        { return( clients_ ); }
inline const vector<string>     getClientNames()    { return( clientNames_ ); }
inline RunIOV                   getRunIOV()         { return( runiov_ ); }
inline MonRunIOV                getMonIOV()         { return( moniov_ ); }
inline const TH1F*              getEntryHisto()     { return( h_ ); }

private:

int ievt_;
int jevt_;

bool collateSources_;
bool cloneME_;
bool enableQT_;

bool enableTCC_;
bool enableCluster_;
 
bool verbose_;

bool enableMonitorDaemon_;

string clientName_;

string prefixME_;

string hostName_;
int    hostPort_;

bool enableServer_;
int  serverPort_;
 
string inputFile_;
string outputFile_;
 
string dbName_;
string dbHostName_;
int    dbHostPort_;
string dbUserName_;
string dbPassword_;

string maskFile_;
 
RunIOV runiov_;
MonRunIOV moniov_;

bool enableSubRunDb_;
bool enableSubRunHtml_;
int subrun_;
 
time_t current_time_;
time_t last_time_db_;
time_t last_time_html_;
time_t dbRefreshTime_;
time_t htmlRefreshTime_;
 
string baseHtmlDir_;

vector<int> superModules_;

typedef multimap<EBClient*,int> EBCIMMap; 
EBCIMMap chb_;
vector<string> runTypes_;
vector<EBClient*> clients_; 
vector<string> clientNames_; 

EBSummaryClient* summaryClient_;

MonitorUserInterface* mui_;
 
bool enableStateMachine_;
 
string location_;
int    runtype_;
string status_;
int run_;
int evt_;
 
bool begin_run_;
bool end_run_;
 
bool forced_status_;
 
bool forced_update_;

bool enableExit_;
 
int last_update_;
 
int last_jevt_;
 
int unknowns_;
 
CollateMonitorElement* me_h_;

TH1F* h_;

};

#endif
