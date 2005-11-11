/*
 * \file EBPedPreSampleClient.cc
 * 
 * $Date: 2005/11/10 16:45:05 $
 * $Revision: 1.2 $
 * \author G. Della Ricca
 *
*/

#include <DQM/EcalBarrelMonitorClient/interface/EBPedPreSampleClient.h>

EBPedPreSampleClient::EBPedPreSampleClient(const edm::ParameterSet& ps, MonitorUserInterface* mui){

  mui_ = mui;

}

EBPedPreSampleClient::~EBPedPreSampleClient(){

  this->unsubscribe();

}

void EBPedPreSampleClient::beginJob(const edm::EventSetup& c){

  cout << "EBPedPreSampleClient: beginJob" << endl;

  ievt_ = 0;

//  this->beginRun(c);

}

void EBPedPreSampleClient::beginRun(const edm::EventSetup& c){

  cout << "EBPedPreSampleClient: beginRun" << endl;

  jevt_ = 0;

  this->subscribe();

}

void EBPedPreSampleClient::endJob(void) {

  cout << "EBPedPreSampleClient: endJob, ievt = " << ievt_ << endl;

}

void EBPedPreSampleClient::endRun(EcalCondDBInterface* econn, RunIOV* runiov, RunTag* runtag) {

  cout << "EBPedPreSampleClient: endRun, jevt = " << jevt_ << endl;

  this->htmlOutput();

}

void EBPedPreSampleClient::subscribe(void){

  // subscribe to all monitorable matching pattern
  mui_->subscribe("*/EcalBarrel/EBPedPreSampleTask/Gain01/EBPT pedestal SM*");

}

void EBPedPreSampleClient::subscribeNew(void){

  // subscribe to new monitorable matching pattern
  mui_->subscribeNew("*/EcalBarrel/EBPedPreSampleTask/Gain01/EBPT pedestal SM*");

}

void EBPedPreSampleClient::unsubscribe(void){

  // unsubscribe to all monitorable matching pattern
  mui_->unsubscribe("*/EcalBarrel/EBPedPreSampleTask/Gain01/EBPT pedestal SM*");

}

void EBPedPreSampleClient::analyze(const edm::Event& e, const edm::EventSetup& c){

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 )  
  cout << "EBPedPreSampleClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;

}

void EBPedPreSampleClient::htmlOutput(void){

}

