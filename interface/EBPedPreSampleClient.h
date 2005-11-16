#ifndef EBPedPreSampleClient_H
#define EBPedPreSampleClient_H

/*
 * \file EBPedPreSampleClient.h
 *
 * $Date: 2005/11/16 10:41:48 $
 * $Revision: 1.8 $
 * \author G. Della Ricca
 *
*/

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include <FWCore/Framework/interface/EDAnalyzer.h>

#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"
#include "DQMServices/Daemon/interface/MonitorDaemon.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "CalibCalorimetry/EcalDBInterface/interface/EcalCondDBInterface.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunTag.h"
#include "CalibCalorimetry/EcalDBInterface/interface/RunIOV.h"

//#include "CalibCalorimetry/EcalDBInterface/interface/MonPedPreSampleDat.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TPaveStats.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace cms;
using namespace std;

class EBPedPreSampleClient: public edm::EDAnalyzer{

friend class EcalBarrelMonitorClient;

public:

/// Constructor
EBPedPreSampleClient(const edm::ParameterSet& ps, MonitorUserInterface* mui);

/// Destructor
virtual ~EBPedPreSampleClient();

protected:

/// Subscribe/Unsubscribe to Monitoring Elements
void subscribe();
void subscribeNew();
void unsubscribe();

/// Analyze
void analyze(const edm::Event& e, const edm::EventSetup& c);

// BeginJob
void beginJob(const edm::EventSetup& c);

// EndJob
void endJob(void);

// BeginRun
void beginRun(const edm::EventSetup& c);

// EndRun
void endRun(EcalCondDBInterface* econn, RunIOV* runiov, RunTag* runtag);

// HtmlOutput
virtual void htmlOutput(int run, string htmlDir);

private:

int ievt_;
int jevt_;

MonitorUserInterface* mui_;

TProfile2D* h01_[36];

TH2F* g01_[36];

TH1F* p01_[36];

TH1F* r01_[36];

// Quality check on crystals, one per each gain

float expectedMean_;
float discrepancyMean_;
float RMSThreshold_; 

};

#endif
