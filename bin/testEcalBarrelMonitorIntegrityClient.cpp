/*
 * \file EcalBarrelMonitorIntegrityClient.cpp
 *
 *  $Date: 2005/11/20 08:09:17 $
 *  $Revision: 1.17 $
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
    mui->subscribeNew("*/EcalBarrel/EcalIntegrity/Gain/EI gain SM01*");
    mui->subscribeNew("*/EcalBarrel/EcalIntegrity/ChId/EI ChId SM01*");
    mui->subscribeNew("*/EcalBarrel/EcalIntegrity/TTId/EI TTId SM01*");
    mui->subscribeNew("*/EcalBarrel/EcalIntegrity/TTBlockSize/EI TTBlockSize SM01*");
    mui->subscribeNew("*/EcalBarrel/EcalIntegrity/DCC size error");

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

      TH1F* h1;
      TH2F* h2;

      me = mui->get("Collector/FU0/EcalBarrel/EcalIntegrity/DCC size error");
      h1 = getTH1F(me);
      if ( h1 ) {
        c1->cd();
        h1->SetOption("text");
        h1->Draw();
        c1->Update();
      }

      me = mui->get("Collector/FU0/EcalBarrel/EcalIntegrity/Gain/EI gain SM01");
      h2 = getTH2F(me);
      if ( h2 ) {
        c2->cd(1);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      me = mui->get("Collector/FU0/EcalBarrel/EcalIntegrity/ChId/EI ChId SM01");
      h2 = getTH2F(me);
      if ( h2 ) {
        c2->cd(2);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      me = mui->get("Collector/FU0/EcalBarrel/EcalIntegrity/TTId/EI TTId SM01");
      h2 = getTH2F(me);
      if ( h2 ) {
        c2->cd(3);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      me = mui->get("Collector/FU0/EcalBarrel/EcalIntegrity/TTBlockSize/EI TTBlockSize SM01");
      h2 = getTH2F(me);
      if ( h2 ) {
        c2->cd(4);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      c2->cd();
      c2->Modified();
      c2->Update();

      last_plotting = updates;
    }

  }

  exit_done = true;

  return 0;
}

int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Barrel Monitor Integrity Client ***" << endl;
  cout << endl;

  TApplication app("app", &argc, argv);

  // default client name
  string cfuname = "UserIntegrity";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  c1 = new TCanvas("Ecal Barrel Integrity Monitoring 1","Ecal Barrel Integrity Monitoring 1",  0, 0,400,400);
  c1->Modified();
  c1->Update();
  c2 = new TCanvas("Ecal Barrel Integrity Monitoring 2","Ecal Barrel Integrity Monitoring 2",430, 0,600,600);
  c2->Divide(2,2);
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
  mui->subscribe("*/EcalBarrel/EcalIntegrity/Gain/EI gain SM01*");
  mui->subscribe("*/EcalBarrel/EcalIntegrity/ChId/EI ChId SM01*");
  mui->subscribe("*/EcalBarrel/EcalIntegrity/TTId/EI TTId SM01*");
  mui->subscribe("*/EcalBarrel/EcalIntegrity/TTBlockSize/EI TTBlockSize SM01*");
  mui->subscribe("*/EcalBarrel/EcalIntegrity/DCC size error");

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

