/*
 * \file EcalBarrelMonitorIntegrityClient.cpp
 *
 *  $Date: 2006/02/24 08:03:48 $
 *  $Revision: 1.23 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "DataFormats/EcalRawData/interface/EcalRawDataCollections.h"

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
    mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/Gain/EBIT gain SM01");
    mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/ChId/EBIT ChId SM01");
    mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/TTId/EBIT TTId SM01");
    mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize SM01");
    mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/EBIT DCC size error");
     mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId SM01");
     mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain SM01");
     mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId SM01");
     mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize SM01");



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
        if ( atoi(s.substr(2,s.size()-2).c_str()) == COSMIC ) type = "COSMIC";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == LASER_STD ) type = "LASER";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == PEDESTAL_STD ) type = "PEDESTAL";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == TESTPULSE_MGPA ) type = "TEST_PULSE";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == BEAMH4 ) type = "BEAMH4";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == BEAMH2 ) type = "BEAMH2";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == MTCC ) type = "MTCC";
        cout << "type = " << type << endl;
      }

      TH1F* h1;
      TH2F* h2;

      //      me = mui->get("Collector/FU0/EcalBarrel/EBIntegrityTask/EBIT DCC size error");
      me = mui->get("EcalBarrel/Sums/EBIntegrityTask/EBIT DCC size error");
      h1 = getTH1F(me);
      if ( h1 ) {
        c1->cd();
        h1->SetOption("text");
        h1->Draw();
        c1->Update();
      }
    
      //      me = mui->get("Collector/FU0/EcalBarrel/EBIntegrityTask/Gain/EBIT gain SM01");
      me = mui->get("EcalBarrel/Sums/EBIntegrityTask/Gain/EBIT gain SM01");
      h2 = getTH2F(me);
      if ( h2 ) {
        c2->cd(1);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      //      me = mui->get("Collector/FU0/EcalBarrel/EBIntegrityTask/ChId/EBIT ChId SM01");
      me = mui->get("EcalBarrel/Sums/EBIntegrityTask/ChId/EBIT ChId SM01");
      h2 = getTH2F(me);
      if ( h2 ) {
        c2->cd(2);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      //      me = mui->get("Collector/FU0/EcalBarrel/EBIntegrityTask/TTId/EBIT TTId SM01");
      me = mui->get("EcalBarrel/Sums/EBIntegrityTask/TTId/EBIT TTId SM01");
      h2 = getTH2F(me);
      if ( h2 ) {
        c2->cd(3);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      //      me = mui->get("Collector/FU0/EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize SM01");
      me = mui->get("EcalBarrel/Sums/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize SM01");
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
      
      
      
      // me = mui->get("EcalBarrel/Sums/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize SM01");
      me = mui->get("EcalBarrel/Sums/EBIntegrityTask/MemChId/EBIT MemChId SM01");
       h2 = getTH2F(me);
       if ( h2 ) {
         c3->cd(1);
         h2->SetOption("col");
         h2->Draw();
         c3->Update();
       }

      c3->cd();
      c3->Modified();
      c3->Update();

      me = mui->get("EcalBarrel/Sums/EBIntegrityTask/MemGain/EBIT MemGain SM01");
       h2 = getTH2F(me);
       if ( h2 ) {
          c3->cd(2);
         h2->SetOption("col");
         h2->Draw();
         c3->Update();
       }

      c3->cd();
      c3->Modified();
      c3->Update();


      
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
  c3 = new TCanvas("Ecal Barrel Integrity Monitoring 3","Ecal Barrel Integrity Monitoring 3", 430,615,600,370);
  c3->Divide(1,2);
  c3->Modified();
  c3->Update();

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
  mui->subscribe("*/EcalBarrel/EBIntegrityTask/Gain/EBIT gain SM01");
  mui->subscribe("*/EcalBarrel/EBIntegrityTask/ChId/EBIT ChId SM01");
  mui->subscribe("*/EcalBarrel/EBIntegrityTask/TTId/EBIT TTId SM01");
  mui->subscribe("*/EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize SM01");
  mui->subscribe("*/EcalBarrel/EBIntegrityTask/EBIT DCC size error");
  mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId SM01");
  mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain SM01");
  mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId SM01");
  mui->subscribeNew("*/EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize SM01");





  CollateMonitorElement* cme;
  
  cme = mui->collate2D("EBIT gain SM01", "EBIT gain SM01", "EcalBarrel/Sums/EBIntegrityTask/Gain");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/Gain/EBIT gain SM01");

  cme = mui->collate2D("EBIT ChId SM01", "EBIT ChId SM01", "EcalBarrel/Sums/EBIntegrityTask/ChId");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/ChId/EBIT ChId SM01");

  cme = mui->collate2D("EBIT TTId SM01", "EBIT TTId SM01", "EcalBarrel/Sums/EBIntegrityTask/TTId");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/TTId/EBIT TTId SM01");

  cme = mui->collate2D("EBIT TTBlockSize SM01", "EBIT TTBlockSize SM01", "EcalBarrel/Sums/EBIntegrityTask/TTBlockSize");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/TTBlockSize/EBIT TTBlockSize SM01");

  cme = mui->collate1D("EBIT DCC size error", "DCC size error", "EcalBarrel/Sums/EBIntegrityTask");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/EBIT DCC size error");

  cme = mui->collate2D("EBIT MemChId SM01", "EBIT MemChId SM01", "EcalBarrel/Sums/EBIntegrityTask/MemChId");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/MemChId/EBIT MemChId SM01");

  cme = mui->collate2D("EBIT MemGain SM01", "EBIT MemGain SM01", "EcalBarrel/Sums/EBIntegrityTask/MemGain");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/MemGain/EBIT MemGain SM01");

  cme = mui->collate2D("EBIT MemTTId SM01", "EBIT MemTTId SM01", "EcalBarrel/Sums/EBIntegrityTask/MemTTId");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/MemTTId/EBIT MemTTId SM01");

  cme = mui->collate2D("EBIT MemSize SM01", "EBIT MemSize SM01", "EcalBarrel/Sums/EBIntegrityTask/MemSize");
  mui->add(cme, "*/EcalBarrel/EBIntegrityTask/MemSize/EBIT MemSize SM01");


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

