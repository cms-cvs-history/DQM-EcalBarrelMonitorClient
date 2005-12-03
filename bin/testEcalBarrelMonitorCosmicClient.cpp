/*
 * \file EcalBarrelMonitorCosmicClient.cpp
 *
 *  $Date: 2005/11/20 13:58:45 $
 *  $Revision: 1.16 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "testEcalBarrelMonitorUtils.h"

#include "TROOT.h"
#include "TApplication.h"
#include "TThread.h"

#include <iostream>
#include <math.h>

using namespace std;

TCanvas* c1;
TCanvas* c2;

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
    mui->subscribeNew("*/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM01");
    mui->subscribeNew("*/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM01");

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
        if ( s.substr(2,1) == "0" ) type = "cosmic";
        if ( s.substr(2,1) == "1" ) type = "laser";
        if ( s.substr(2,1) == "2" ) type = "pedestal";
        if ( s.substr(2,1) == "3" ) type = "testpulse";
        cout << "type = " << type << endl;
      }

      TProfile2D* h;

//      me = mui->get("Collector/FU0/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM01");
      me = mui->get("EcalBarrel/Sums/EBCosmicTask/Cut/EBCT amplitude cut SM01");
      h = getTProfile2D(me);
      if ( h ) {
        c1->cd();
        h->SetMaximum(1000.);
        h->SetOption("col");
        h->Draw();
        c1->Update();
      }

//      me = mui->get("Collector/FU0/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM01");
      me = mui->get("EcalBarrel/Sums/EBCosmicTask/Sel/EBCT amplitude sel SM01");
      h = getTProfile2D(me);
      if ( h ) {
        c2->cd();
        h->SetMaximum(1000.);
        h->SetOption("col");
        h->Draw();
        c2->Update();
      }

      last_plotting = updates;
    }

  }

  exit_done = true;

  return 0;
}

int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Barrel Monitor Cosmic Client ***" << endl;
  cout << endl;

  TApplication app("app", &argc, argv);

  // default client name
  string cfuname = "UserCosmic";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  c1 = new TCanvas("Ecal Barrel Cosmic Monitoring 1","Ecal Barrel Cosmic Monitoring 1", 0,  0,1000,400);
  c1->Modified();
  c1->Update();
  c2 = new TCanvas("Ecal Barrel Cosmic Monitoring 2","Ecal Barrel Cosmic Monitoring 2", 0,460,1000,400);
  c2->Modified();
  c2->Update();

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
  mui->subscribe("*/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM01");
  mui->subscribe("*/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM01");

  CollateMonitorElement* cme;

  cme = mui->collateProf2D("EBCT amplitude cut SM01", "EBCT amplitude cut SM01", "EcalBarrel/Sums/EBCosmicTask/Cut");
  mui->add(cme, "*/EcalBarrel/EBCosmicTask/Cut/EBCT amplitude cut SM01");

  cme = mui->collateProf2D("EBCT amplitude sel SM01", "EBCT amplitude sel SM01", "EcalBarrel/Sums/EBCosmicTask/Sel");
  mui->add(cme, "*/EcalBarrel/EBCosmicTask/Sel/EBCT amplitude sel SM01");

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

