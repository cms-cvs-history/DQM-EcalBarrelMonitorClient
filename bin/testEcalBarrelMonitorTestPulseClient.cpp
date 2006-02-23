/*
 * \file EcalBarrelMonitorTestPulseClient.cpp
 *
 *  $Date: 2005/12/30 11:19:34 $
 *  $Revision: 1.19 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "DataFormats/EcalRawData/interface/EcalRawDataCollections.h"

#define COSMIC (PHYSICS+20)
#define BEAM   (PHYSICS+21)

#include "testEcalBarrelMonitorUtils.h"

#include "TROOT.h"
#include "TApplication.h"
#include "TThread.h"

#include <iostream>
#include <math.h>

using namespace std;

TCanvas* c1;
TCanvas* c2;
TCanvas* c3;
TCanvas* c4;
TCanvas* c5;
TCanvas* c6;

MonitorUserInterface* mui;

bool exit_now = false;
bool exit_done = false;

void *pth1(void *) {

  bool stay_in_loop = true;

  // last time monitoring objects were plotted
  int last_plotting = -1;

  while ( stay_in_loop && ! exit_now ) {

    // this is the "main" loop where we receive monitoring
    stay_in_loop = mui->update();

    // subscribe to new monitorable matching pattern
    mui->subscribeNew("*/EcalBarrel/STATUS");
    mui->subscribeNew("*/EcalBarrel/RUN");
    mui->subscribeNew("*/EcalBarrel/EVT");
    mui->subscribeNew("*/EcalBarrel/RUNTYPE");
    mui->subscribeNew("*/EcalBarrel/EBTestPulseTask/Gain01/EBTPT shape SM01 G01");
    mui->subscribeNew("*/EcalBarrel/EBTestPulseTask/Gain01/EBTPT amplitude SM01 G01");
    mui->subscribeNew("*/EcalBarrel/EBTestPulseTask/Gain06/EBTPT shape SM01 G06");
    mui->subscribeNew("*/EcalBarrel/EBTestPulseTask/Gain06/EBTPT amplitude SM01 G06");
    mui->subscribeNew("*/EcalBarrel/EBTestPulseTask/Gain12/EBTPT shape SM01 G12");
    mui->subscribeNew("*/EcalBarrel/EBTestPulseTask/Gain12/EBTPT amplitude SM01 G12");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    string s;
    string status;
    string run;
    string evt;
    string type;

    // draw monitoring objects every monitoring cycle
    if ( updates != last_plotting ) {

      me = mui->get("Collector/FU0/EcalBarrel/STATUS");
      if ( me ) {
        s = me->valueString();
        status = "unknown";
        if ( s.substr(2,1) == "0" ) status = "start-of-run";
        if ( s.substr(2,1) == "1" ) status = "running";
        if ( s.substr(2,1) == "2" ) status = "end-of-run";
        cout << "status = " << status << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/RUN");
      if ( me ) {
        s = me->valueString();
        run = s.substr(2,s.length()-2);
        cout << "run = " << run << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/EVT");
      if ( me ) {
        s = me->valueString();
        evt = s.substr(2,s.length()-2);
        cout << "event = " << evt << endl;
      }

      me = mui->get("Collector/FU0/EcalBarrel/RUNTYPE");
      if ( me ) {
        s = me->valueString();
        if ( atoi(s.substr(2,s.size()-2).c_str()) == PHYSICS ) type = "PHYSICS";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == COSMIC ) type = "COSMIC";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == LASER_STD ) type = "LASER";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == PEDESTAL_STD ) type = "PEDESTAL";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == TESTPULSE_MGPA ) type = "TEST_PULSE";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == BEAM ) type = "BEAM";
        cout << "type = " << type << endl;
      }

      TProfile2D* h;

//      me = mui->get("Collector/FU0/EcalBarrel/EBTestPulseTask/Gain01/EBTPT amplitude SM01 G01");
      me = mui->get("EcalBarrel/Sums/EBTestPulseTask/Gain01/EBTPT amplitude SM01 G01");
      h = getTProfile2D(me);
      if ( h ) {
        c1->cd();
        h->SetOption("col");
        h->Draw();
        c1->Update();
      }

//      me = mui->get("Collector/FU0/EcalBarrel/EBTestPulseTask/Gain06/EBTPT amplitude SM01 G06");
      me = mui->get("EcalBarrel/Sums/EBTestPulseTask/Gain06/EBTPT amplitude SM01 G06");
      h = getTProfile2D(me);
      if ( h ) {
        c2->cd();
        h->SetOption("col");
        h->Draw();
        c2->Update();
      }

//      me = mui->get("Collector/FU0/EcalBarrel/EBTestPulseTask/Gain12/EBTPT amplitude SM01 G12");
      me = mui->get("EcalBarrel/Sums/EBTestPulseTask/Gain12/EBTPT amplitude SM01 G12");
      h = getTProfile2D(me);
      if ( h ) {
        c3->cd();
        h->SetOption("col");
        h->Draw();
        c3->Update();
      }

//      me = mui->get("Collector/FU0/EcalBarrel/EBTestPulseTask/Gain01/EBTPT shape SM01 G01");
      me = mui->get("EcalBarrel/Sums/EBTestPulseTask/Gain01/EBTPT shape SM01 G01");
      h = getTProfile2D(me);
      if ( h ) {
        c4->cd();
        h->SetOption("lego");
        h->Draw();
        c4->Update();
      }

//      me = mui->get("Collector/FU0/EcalBarrel/EBTestPulseTask/Gain06/EBTPT shape SM01 G06");
      me = mui->get("EcalBarrel/Sums/EBTestPulseTask/Gain06/EBTPT shape SM01 G06");
      h = getTProfile2D(me);
      if ( h ) {
        c5->cd();
        h->SetOption("lego");
        h->Draw();
        c5->Update();
      }

//      me = mui->get("Collector/FU0/EcalBarrel/EBTestPulseTask/Gain12/EBTPT shape SM01 G12");
      me = mui->get("EcalBarrel/Sums/EBTestPulseTask/Gain12/EBTPT shape SM01 G12");
      h = getTProfile2D(me);
      if ( h ) {
        c6->cd();
        h->SetOption("lego");
        h->Draw();
        c6->Update();
      }

      last_plotting = updates;
    }

  }

  exit_done = true;

  return 0;
}

int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Barrel Test Pulse Monitor Client ***" << endl;
  cout << endl;

  TApplication app("app", &argc, argv);

  // default client name
  string cfuname = "UserTestPulse";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  c1 = new TCanvas("Ecal Barrel Test Pulse Monitoring amplitude G01","Ecal Barrel Test Pulse Monitoring amplitude G01", 0,  0,800,250);
  c1->Modified();
  c1->Update();
  c2 = new TCanvas("Ecal Barrel Test Pulse Monitoring amplitude G06","Ecal Barrel Test Pulse Monitoring amplitude G06", 0,310,800,250);
  c2->Modified();
  c2->Update();
  c3 = new TCanvas("Ecal Barrel Test Pulse Monitoring amplitude G12","Ecal Barrel Test Pulse Monitoring amplitude G12", 0,620,800,250);
  c3->Modified();
  c3->Update();
  c4 = new TCanvas("Ecal Barrel Test Pulse Monitoring shape G01","Ecal Barrel Test Pulse Monitoring shape G01",820,  0,250,250);
  c4->Modified();
  c4->Update();
  c5 = new TCanvas("Ecal Barrel Test Pulse Monitoring shape G06","Ecal Barrel Test Pulse Monitoring shape G06",820,310,250,250);
  c5->Modified();
  c5->Update();
  c6 = new TCanvas("Ecal Barrel Test Pulse Monitoring shape G12","Ecal Barrel Test Pulse Monitoring shape G12",820,620,250,250);
  c6->Modified();
  c6->Update();

  if ( argc >= 2 ) cfuname = argv[1];
  if ( argc >= 3 ) hostname = argv[2];
  if ( argc >= 4 ) port_no = atoi(argv[3]);

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  mui = new MonitorUIRoot(hostname, port_no, cfuname);

  mui->setVerbose(1);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("*/EcalBarrel/STATUS");
  mui->subscribe("*/EcalBarrel/RUN");
  mui->subscribe("*/EcalBarrel/EVT");
  mui->subscribe("*/EcalBarrel/RUNTYPE");
  mui->subscribe("*/EcalBarrel/EBTestPulseTask/Gain01/EBTPT shape SM01 G01");
  mui->subscribe("*/EcalBarrel/EBTestPulseTask/Gain01/EBTPT amplitude SM01 G01");
  mui->subscribe("*/EcalBarrel/EBTestPulseTask/Gain06/EBTPT shape SM01 G06");
  mui->subscribe("*/EcalBarrel/EBTestPulseTask/Gain06/EBTPT amplitude SM01 G06");
  mui->subscribe("*/EcalBarrel/EBTestPulseTask/Gain12/EBTPT shape SM01 G12");
  mui->subscribe("*/EcalBarrel/EBTestPulseTask/Gain12/EBTPT amplitude SM01 G12");

  CollateMonitorElement* cme;

  cme = mui->collateProf2D("EBTPT shape SM01 G01", "EBTPT shape SM01 G01", "EcalBarrel/Sums/EBTestPulseTask/Gain01");
  mui->add(cme, "*/EcalBarrel/EBTestPulseTask/Gain01/EBTPT shape SM01 G01");

  cme = mui->collateProf2D("EBTPT amplitude SM01 G01", "EBTPT amplitude SM01 G01", "EcalBarrel/Sums/EBTestPulseTask/Gain01");
  mui->add(cme, "*/EcalBarrel/EBTestPulseTask/Gain01/EBTPT amplitude SM01 G01");

  cme = mui->collateProf2D("EBTPT shape SM01 G06", "EBTPT shape SM01 G06", "EcalBarrel/Sums/EBTestPulseTask/Gain06");
  mui->add(cme, "*/EcalBarrel/EBTestPulseTask/Gain06/EBTPT shape SM01 G06");

  cme = mui->collateProf2D("EBTPT amplitude SM01 G06", "EBTPT amplitude SM01 G06", "EcalBarrel/Sums/EBTestPulseTask/Gain06");
  mui->add(cme, "*/EcalBarrel/EBTestPulseTask/Gain06/EBTPT amplitude SM01 G06");

  cme = mui->collateProf2D("EBTPT shape SM01 G12", "EBTPT shape SM01 G12", "EcalBarrel/Sums/EBTestPulseTask/Gain12");
  mui->add(cme, "*/EcalBarrel/EBTestPulseTask/Gain12/EBTPT shape SM01 G12");

  cme = mui->collateProf2D("EBTPT amplitude SM01 G12", "EBTPT amplitude SM01 G12", "EcalBarrel/Sums/EBTestPulseTask/Gain12");
  mui->add(cme, "*/EcalBarrel/EBTestPulseTask/Gain12/EBTPT amplitude SM01 G12");

  TThread *th1 = new TThread("th1",pth1);

  th1->Run();

  try { app.Run(kTRUE); } catch (...) { throw; }

  mui->unsubscribe("*");

  exit_now = true;

  while ( ! exit_done ) { usleep(100); }

  th1->Delete();

  delete mui;

  return 0;

}

