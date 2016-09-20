// Server program which allows clients, "spies", to connect and snoop objects.
// To run this demo do the following:
//   - open two or more windows
//   - start root in all windows
//   - execute in the first window:    .x serial_spyserv.C  (or serial_spyserv.C++)
//   - execute in the other window(s): .x serial_spy.C      (or serial_spy.C++)
//   - in the "serial_spy" client windows click the "Connect" button and snoop
//     the histograms by clicking on the "hpx", "hpxpy" and "hprof"
//     buttons
//
// features to be added:
//   - together with client enable feedback messages when HVRamping (give steps taken)
//   - on HVRamp, when hitting compliance automatically go to safe voltage and end loop
//   - when disconnecting a client, do not crash...
   
#include "TObjString.h"
#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TRandom.h"
#include "TList.h"
#include "TMath.h"
#include "TDatime.h"
#include "TError.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TWebFile.h"
#include "TSocket.h"
#include "CalcDewpoint.C"
#include "../compatible.hh"

class TempInterlock;
class Chiller_Medingen_C20;
class Keithley2410;
class tti_CPX400;

class serial_spyserv {
private:
  TObjString    *fString;    // a string
  TServerSocket *fServ;      // server socket
  TMonitor      *fMon;       // socket monitor
  TList         *fSockets;   // list of open serial_spy sockets
  Double_t      lv_voltagelimit, lv_currentlimit;
  Double_t      hybrid1temp, hybrid2temp, chuck_hum, chuck_temp, chiller_targettemp, chiller_actualtemp, chuck_dewpoint, hv_voltage, hv_current, lv_voltage, lv_current, cc_dewpoint; 
  Double_t      prev_hybrid1temp, prev_hybrid2temp, prev_chuck_hum, prev_chuck_temp, prev_chiller_targettemp, prev_chiller_actualtemp, prev_chuck_dewpoint, prev_hv_voltage, prev_hv_current, prev_lv_voltage, prev_lv_current, prev_cc_dewpoint; 
  Int_t         state_indicator;
  Int_t         time, small, large;
  TString       helpme="";
  TDatime       date_root;
  TString       sdate_root;   
  TWebFile      *wda;
  TSocket       *fSock;
  Double_t      val_one,val_two,val_three,val_four,val_five,val_six,val_final;

  
  TempInterlock*        tinterlock;
  Chiller_Medingen_C20* chiller; 
  Keithley2410*         hv;
  tti_CPX400 *          lv;
  
public:
  serial_spyserv();
  ~serial_spyserv();

  void     HandleSocket(TSocket *s);
  Double_t CCtalk(Int_t CCValueNr, Bool_t set_mode = false, Bool_t toggle_ONOFF=false, Bool_t turnON=false, Double_t temp = 20., Double_t hum = 0.);
};


serial_spyserv::serial_spyserv()
{
  // Create the server process to fills a number of histograms.
  // A serial_spy process can connect to it and ask for the histograms.
  // There is no apriory limit for the number of concurrent serial_spy processes.

  // Load the serial Library, set up serial links
  //gSystem->Load("SCom_V1p5_ROOT_5-28_fixed.dll");
  //gSystem->Load("SCom_r180_ROOT_5-34.dll");
  //gSystem->Load("SerialCom.dll"); // got too lazy to always rename it... 2015-11-03 IBL

  //  gSystem->Load("../libwkserial.so"); // got too lazy to always rename it... 2015-11-03 IBL

  //gROOT->ProcessLine(".L CalcDewpoint.C");

  
  FileStat_t buf;
  if(!gSystem->GetPathInfo("/dev/ttyACM0", buf)){
    tinterlock = new TempInterlock("/dev/ttyACM0");
  }else if(!gSystem->GetPathInfo("/dev/ttyACM1", buf)){
    tinterlock = new TempInterlock("/dev/ttyACM1");
  }
  else{
    std::cout<<std::endl
             <<"Error!"<<std::endl
             <<"Can not find the device at /dev/ttyACM(0|1) for interlock"
             <<std::endl;
    exit(-1);
  }
  chiller    = new Chiller_Medingen_C20("/dev/ttyS0"); 
  hv         = new Keithley2410("/dev/ttyS1");
  lv         = new tti_CPX400("/dev/ttyS2");
  
  helpme.Append("\n");
  helpme.Append("\n");
  helpme.Append("**************************************\n");
  helpme.Append("Possible \"Request\" Options are:\n");
  helpme.Append("*** Get ******************************\n");
  helpme.Append("Get_ChuckHumidity\n");
  helpme.Append("Get_ChuckTemperature\n");
  helpme.Append("Get_ChuckDewpoint\n");
  helpme.Append("Get_Hybrid1Temperature\n");
  helpme.Append("Get_Hybrid2Temperature\n");
  helpme.Append("Get_ChillerTargetTemperature\n");
  helpme.Append("Get_ChillerActualTemperature\n");
  helpme.Append("Get_HVVoltage\n");
  helpme.Append("Get_HVCurrent\n");
  helpme.Append("Get_LVVoltage_<channel (1 or 2)>\n");
  helpme.Append("Get_LVCurrent_<channel (1 or 2)>\n");
  helpme.Append("Get_LVVoltageLimit_<channel (1 or 2)>\n");
  helpme.Append("Get_LVCurrentLimit_<channel (1 or 2)>\n");
  helpme.Append("Get_CCHumidity\n");
  helpme.Append("Get_CCTargetHumidity\n");
  helpme.Append("Get_CCTemperature\n");
  helpme.Append("Get_CCTargetTemperature\n");
  helpme.Append("Get_CCDewpoint\n");
  helpme.Append("*** Set ******************************\n");
  helpme.Append("Set_HVVoltage_<value>\n");
  helpme.Append("Set_HVVoltageRamp_<value>\n");
  helpme.Append("Set_HVVoltageSpeedRamp_<value>\n");
  helpme.Append("Set_LVVoltageLimit_<channel>_<val>\n");
  helpme.Append("Set_LVCurrentLimit_<channel>_<val>\n");
  helpme.Append("Set_LVON_<channel>\n");
  helpme.Append("Set_LVOFF_<channel>\n");
  helpme.Append("Set_ChillerTargetTemperature_<value>\n");
  helpme.Append("Set_CCTargetHumidity_<value>\n");
  helpme.Append("Set_CCTargetHumidityON_<value>\n");
  helpme.Append("Set_CCTargetHumidityOFF_<value>\n");
  helpme.Append("Set_CCTargetTemperature_<value>\n");
  helpme.Append("Set_CCTargetTemperatureON_<value>\n");
  helpme.Append("Set_CCTargetTemperatureOFF_<value>\n");
  helpme.Append("Set_CCON\n");
  helpme.Append("Set_CCOFF\n");
  helpme.Append("**************************************\n");
  helpme.Append("**************************************\n");
  helpme.Append("Possible \"RequestString\" Options are:\n");
  helpme.Append("*** Get ******************************\n");
  helpme.Append("Get_Dust_2H108\n");
  helpme.Append("**************************************\n");
  // Open a server socket looking for connections on a named service or
  // on a specified port
  //TServerSocket *ss = new TServerSocket("serial_spyserv", kTRUE);
  fServ = new TServerSocket(9090, kTRUE);
  if (!fServ->IsValid())
    gSystem->Exit(1);

  // Add server socket to monitor so we are notified when a client needs to be
  // accepted
  fMon  = new TMonitor;
  fMon->Add(fServ);

  // Create a list to contain all client connections
  fSockets = new TList;
  
  
  
  for (Int_t i = 0; ; ++i) {
    
    // Until we figured to only request data on client query, we do it all the time and wait...
    //        gSystem->Sleep(10);
    
    // Check if there is a message waiting on one of the sockets.
    // Wait not longer than 20ms (returns -1 in case of time-out).
    TSocket *s;
    if ((s = fMon->Select(20)) != (TSocket*)-1)    HandleSocket(s);
    if (gROOT->IsInterrupted())
      break;
  }
}

void serial_spyserv::HandleSocket(TSocket *s)
{
  
  
  
  if (s->IsA() == TServerSocket::Class()) {
    // accept new connection from serial_spy
    TSocket *sock = ((TServerSocket*)s)->Accept();
    fMon->Add(sock);
    fSockets->Add(sock);
    
    date_root.Set();
    sdate_root = "";
    sdate_root+=date_root.GetDate();
    sdate_root.Append("_");
    sdate_root+=date_root.GetHour();
    sdate_root.Append(":");
    sdate_root+=date_root.GetMinute();
    sdate_root.Append(":");
    sdate_root+=date_root.GetSecond();
    
    
    printf("%s accepted connection from %s\n",sdate_root.Data(), sock->GetInetAddress().GetHostName());
  } else {
    // we only get string based requests from the serial_spy
    char request[128];
    if (s->Recv(request, sizeof(request)) <= 0) {
      fMon->Remove(s);
      fSockets->Remove(s);
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();
      
      
      printf("%s closed connection from %s\n", sdate_root.Data(), s->GetInetAddress().GetHostName());
      delete s;
      // delete tinterlock;
      // delete chiller;
      // delete hv;
      return;
    }
    
    // send requested object back
    TMessage answer(kMESS_OBJECT);
    //    TMessage answer2(kMESS_OBJECT);
    TString buffer_request = request;
    TString buffer_request_lowercase = "";
    buffer_request_lowercase.Append(buffer_request);
    buffer_request_lowercase.ToLower();
    if (!strcmp(request, "What is the answer?")) {
      fString = new TObjString("42");
      //      	 fString->SetString(fString->GetString().Append("_and_"));
      //      	 fString->SetString(fString->GetString().Append(Form("%12.5g", chuck_dewpoint)));
      answer.WriteObject(fString);
    }
    else if (buffer_request_lowercase.Contains("help")) {
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(helpme));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_Dust_2H108")) {
      //      std::cout<<"getting file"<<std::endl;
      // open the dust web file:
      wda = new TWebFile("http://airquali01.ifh.de/dustmon/current.root");
      std::cout<<"got web dust file"<<std::endl;  
      //      wda->ls();
      //      wda->ReOpen("READ");
      wda->cd();
      //      dust->Scan();
      //IBLOUT2016-02-02      dust->SetBranchAddress("time",&time);
      //IBLOUT2016-02-02      dust->SetBranchAddress("small",&small);
      //IBLOUT2016-02-02      dust->SetBranchAddress("large",&large);
      //IBLOUT2016-02-02      dust->GetEntry(0);
      //      std::cout<<"small: "<<small<<std::endl;
      //      std::cout<<"large: "<<large<<std::endl;
      //      std::cout<<"time:  "<<time<<std::endl;
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%d %d %d",time, small, large)));
      answer.WriteObject(fString);
      wda->Close();
      delete wda;
    }
    else if (!strcmp(request, "Get_CCHumidity")) {
      fString = new TObjString("");
      //      fString->SetString(fString->GetString().Append(buffer));
      fString->SetString(fString->GetString().Append(Form("%12.5g", CCtalk(4))));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_CCTargetHumidity")) {
      //      std::cout<<"Get_CCTargetHumidity"<<std::endl;
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", CCtalk(3))));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_CCTemperature")) {
      //      std::cout<<"Get_CCTemperature"<<std::endl;
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", CCtalk(2))));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_CCTargetTemperature")) {
      //      std::cout<<"Get_CCTargetTemperature"<<std::endl;
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", CCtalk(1))));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_CCDewpoint")) {
      cc_dewpoint = CalcDewpoint(CCtalk(2),CCtalk(4));
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", cc_dewpoint)));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_ChuckHumidity")) {
      chuck_hum = tinterlock->getTemp(11);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", chuck_hum)));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_ChuckTemperature")) {
      chuck_temp = tinterlock->getTemp(10); 
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", chuck_temp)));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_ChuckDewpoint")) {
      chuck_dewpoint = CalcDewpoint(tinterlock->getTemp(10),tinterlock->getTemp(11));
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", chuck_dewpoint)));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_Hybrid1Temperature")) {
      prev_hybrid1temp = hybrid1temp;
      hybrid1temp = tinterlock->getTemp(2); // (0 for wide cable end)
      if( TMath::Abs( prev_hybrid1temp - hybrid1temp) < 10. && hybrid1temp > -50. ) {
        // prev reading was in similar range - accept :)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hybrid1temp)));
      }
      else {
        // prev reading quite different - rather read twice more... bit dumb, but so far only seen single bad readings
        hybrid1temp = tinterlock->getTemp(2); // (0 for wide cable end)
        hybrid1temp = tinterlock->getTemp(2); // (0 for wide cable end)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hybrid1temp)));
      }
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_Hybrid2Temperature")) {
      prev_hybrid2temp = hybrid2temp;
      hybrid2temp = tinterlock->getTemp(3); // (0 for wide cable end)
      if( TMath::Abs( prev_hybrid2temp - hybrid2temp) < 10. && hybrid2temp > -50. ) {
        // prev reading was in similar range - accept :)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hybrid2temp)));
      }
      else {
        // prev reading quite different - rather read twice more... bit dumb, but so far only seen single bad readings
        hybrid2temp = tinterlock->getTemp(3); // (0 for wide cable end)
        hybrid2temp = tinterlock->getTemp(3); // (0 for wide cable end)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hybrid2temp)));
      }
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_ChillerTargetTemperature")) {
      prev_chiller_targettemp = chiller_targettemp;
      chiller_targettemp = chiller->getTargetTemp();
      if( TMath::Abs( prev_chiller_targettemp - chiller_targettemp) < 10. ) {
        // prev reading was in similar range - accept :)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", chiller_targettemp)));
      }
      else {
        // prev reading quite different - rather read twice more... bit dumb, but so far only seen single bad readings
        chiller_targettemp = chiller->getTargetTemp();
        chiller_targettemp = chiller->getTargetTemp();
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", chiller_targettemp)));
      }
      // chiller_targettemp = chiller->getTargetTemp(); 
      // fString = new TObjString("");
      // fString->SetString(fString->GetString().Append(Form("%12.5g", chiller_targettemp)));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_ChillerActualTemperature")) {
      prev_chiller_actualtemp = chiller_actualtemp;
      chiller_actualtemp = chiller->getActualTemp();
      if( TMath::Abs( prev_chiller_actualtemp - chiller_actualtemp) < 10. ) {
        // prev reading was in similar range - accept :)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", chiller_actualtemp)));
      }
      else {
        // prev reading quite different - rather read once more... bit dumb, but so far only seen single bad readings
        chiller_actualtemp = chiller->getActualTemp();
        chiller_actualtemp = chiller->getActualTemp();
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", chiller_actualtemp)));
      }
      // chiller_actualtemp = chiller->getActualTemp(); 
      // fString = new TObjString("");
      // fString->SetString(fString->GetString().Append(Form("%12.5g", chiller_actualtemp)));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_HVVoltage")) {
      prev_hv_voltage = hv_voltage;
      hv_voltage = hv->getVoltage();
      if( TMath::Abs( prev_hv_voltage - hv_voltage) < 10. && TMath::Abs(hv_voltage) < 10e4 ) {
        // prev reading was in similar range - accept :)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hv_voltage)));
      }
      else {
        // prev reading quite different - rather read once more... bit dumb, but so far only seen single bad readings
        hv_voltage = hv->getVoltage();
        hv_voltage = hv->getVoltage();
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hv_voltage)));
      }
      // hv_voltage = hv->getVoltage();
      // fString = new TObjString("");
      // fString->SetString(fString->GetString().Append(Form("%12.5g", hv_voltage)));
      answer.WriteObject(fString);
    }
    else if (!strcmp(request, "Get_HVCurrent")) {
      prev_hv_current = hv_current;
      hv_current = hv->getCurrent();
      if( TMath::Abs( prev_hv_current - hv_current) < 10e-5 && TMath::Abs(hv_current) < 10e4 ) {
        // prev reading was in similar range - accept :)
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hv_current)));
      }
      else {
        // prev reading quite different - rather read once more... bit dumb, but so far only seen single bad readings
        hv_current = hv->getCurrent();
        hv_current = hv->getCurrent();
        fString = new TObjString("");
        fString->SetString(fString->GetString().Append(Form("%12.5g", hv_current)));
      }
      // hv_current = hv->getCurrent();
      // fString = new TObjString("");
      // fString->SetString(fString->GetString().Append(Form("%12.5g", hv_current)));
      answer.WriteObject(fString);      
    }
    else if (buffer_request.Contains("Get_LVVoltage_")) {
      buffer_request.ReplaceAll("Get_LVVoltage_","");
      Int_t channel = buffer_request.Atoi();
      //      std::cout<<"reading channel: "<<channel<<std::endl;
      prev_lv_voltage = lv_voltage;
      lv_voltage = -999;
      //      std::cout<<"lv return value"<<lv->getActualVoltage(channel)<<std::endl;
      lv_voltage = lv->getActualVoltage(channel);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", lv_voltage)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Get_LVCurrent_")) {
      buffer_request.ReplaceAll("Get_LVCurrent_","");
      Int_t channel = buffer_request.Atoi();
      prev_lv_current = lv_current;
      lv_current = lv->getActualCurrent(channel);
      if( TMath::Abs( prev_lv_current - lv_current) < 10e-5 && TMath::Abs(lv_current) < 10e4 ) {
	// prev reading was in similar range - accept :)
	fString = new TObjString("");
	fString->SetString(fString->GetString().Append(Form("%12.5g", lv_current)));
      }
      else {
	// prev reading quite different - rather read once more... bit dumb, but so far only seen single bad readings
	lv_current = lv->getActualCurrent(channel);
	lv_current = lv->getActualCurrent(channel);
	fString = new TObjString("");
	fString->SetString(fString->GetString().Append(Form("%12.5g", lv_current)));
      }
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Get_LVVoltageLimit_")) {
      buffer_request.ReplaceAll("Get_LVVoltageLimit_","");
      Int_t channel = buffer_request.Atoi();
      lv_voltagelimit = lv->getVoltage(channel);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", lv_voltagelimit)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Get_LVCurrentLimit_")) {
      buffer_request.ReplaceAll("Get_LVCurrentLimit_","");
      Int_t channel = buffer_request.Atoi();
      lv_currentlimit = lv->getCurrent(channel);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", lv_currentlimit)));
      answer.WriteObject(fString);
    }
    // **********************************************
    // *** here begin the "set" commands. ***********
    // **********************************************
    else if (buffer_request.Contains("Set_HVVoltage_")) {
      buffer_request.ReplaceAll("Set_HVVoltage_","");
      Double_t targetVoltage = buffer_request.Atof();

      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" HV voltage set to: "<<targetVoltage<<std::endl;
      state_indicator = hv->setVoltage(targetVoltage);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_HVVoltageRamp_")) {
      buffer_request.ReplaceAll("Set_HVVoltageRamp_","");
      Double_t targetVoltage = buffer_request.Atof();
      const Double_t MinVoltage = -460.0;
      const Double_t MaxVoltage =    0.51;
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" HV voltage will be ramped to: "<<targetVoltage<<std::endl;
      // ---------------------------------------------------------------------------------------------------------
      // ADD CHECK FOR COMPIANCE! e.g.
      // add check if new voltage and old voltage are too similar and one might be in compiance... avoid inf loop ;)
      // stay away from target voltage and zero with this check...
      // ---------------------------------------------------------------------------------------------------------
      // code here for ramp
      if( TMath::Abs( hv->getVoltage() - targetVoltage ) < 1.2 ) { 
        // go ahead, we are already at the voltage 
        std::cout<<"HV ("<<hv->getVoltage()<<"V) is close to requested value!"<<std::endl;
        state_indicator = 0;
        if(state_indicator == 0) hv->setVoltage(targetVoltage);
        //	s->Send("Voltage is fine, all set");
      }
      else {
        if( hv->getVoltage() > targetVoltage ) { // voltage needs to be lowered
          while ( TMath::Abs(hv->getVoltage() - targetVoltage ) >= 1.2  && hv->getVoltage() > targetVoltage ) {
            hv->setVoltage(hv->getVoltage()-1.);
            std::cout<<"Ramping DOWN ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
            //	    gSystem->Sleep(10); // IBL 2015-11-03 out for a try...
            state_indicator = 0;
            //	    s->Send("Ramping... will display current voltage soon.");
            if(hv->getVoltage() < MinVoltage || hv->getVoltage() > MaxVoltage ) {
              std::cout<<"EXCESS voltage. Aborting ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
              state_indicator = -1;
              break;
            }
          }
          if(state_indicator == 0) hv->setVoltage(targetVoltage);
        }
        else { // voltage needs to be increased
          while ( TMath::Abs(hv->getVoltage() - targetVoltage ) >= 1.2 && hv->getVoltage() < targetVoltage ) {
            hv->setVoltage(hv->getVoltage()+1.);
            std::cout<<"Ramping UP ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
            //	    gSystem->Sleep(10); // IBL 2015-11-03 out for a try...
            state_indicator = 0;
            //	    s->Send("Ramping UP ... will display current voltage soon.");
            if( hv->getVoltage() < MinVoltage || hv->getVoltage() > MaxVoltage ) {
              std::cout<<"EXCESS voltage. Aborting ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
              state_indicator = -1;
              break; 
            }
          }
          if(state_indicator == 0) hv->setVoltage(targetVoltage);
        }
      }
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" Ramped to bias voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
      //      fString2 = new TObjString("");
      //      fString2->SetString(fString2->GetString().Append("Done Ramping.");
      //      answer2.WriteObject(fString);
      //      s->Send(answer2);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_HVVoltageSpeedRamp_")) {
      buffer_request.ReplaceAll("Set_HVVoltageSpeedRamp_","");
      Double_t targetVoltage = buffer_request.Atof();
      const Double_t MinVoltage = -460.0;
      const Double_t MaxVoltage =    0.51;
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" HV voltage will be speed ramped to: "<<targetVoltage<<std::endl;
      // ---------------------------------------------------------------------------------------------------------
      // ADD CHECK FOR COMPIANCE! e.g.
      // add check if new voltage and old voltage are too similar and one might be in compiance... avoid inf loop ;)
      // stay away from target voltage and zero with this check...
      // ---------------------------------------------------------------------------------------------------------
      // code here for ramp
      if( TMath::Abs( hv->getVoltage() - targetVoltage ) < 12. ) { 
        // go ahead, we are already at the voltage 
        std::cout<<"HV ("<<hv->getVoltage()<<"V) is close to requested value!"<<std::endl;
        state_indicator = 0;
        if(state_indicator == 0) hv->setVoltage(targetVoltage);
        //	s->Send("Voltage is fine, all set");
      }
      else {
        if( hv->getVoltage() > targetVoltage ) { // voltage needs to be lowered
          while ( TMath::Abs(hv->getVoltage() - targetVoltage ) >= 12.  && hv->getVoltage() > targetVoltage ) {
            hv->setVoltage(hv->getVoltage()-10.);
            std::cout<<"SpeedRamping DOWN ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
            //	    gSystem->Sleep(10); // IBL 2015-11-03 out for a try...
            state_indicator = 0;
            //	    s->Send("Ramping... will display current voltage soon.");
            if(hv->getVoltage() < MinVoltage || hv->getVoltage() > MaxVoltage ) {
              std::cout<<"EXCESS voltage. Aborting ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
              state_indicator = -1;
              break;
            }
          }
          if(state_indicator == 0) hv->setVoltage(targetVoltage);
        }
        else { // voltage needs to be increased
          while ( TMath::Abs(hv->getVoltage() - targetVoltage ) >= 12. && hv->getVoltage() < targetVoltage ) {
            hv->setVoltage(hv->getVoltage()+10.);
            std::cout<<"SpeedRamping UP ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
            //	    gSystem->Sleep(10); // IBL 2015-11-03 out for a try...
            state_indicator = 0;
            //	    s->Send("Ramping UP ... will display current voltage soon.");
            if( hv->getVoltage() < MinVoltage || hv->getVoltage() > MaxVoltage ) {
              std::cout<<"EXCESS voltage. Aborting ... current voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
              state_indicator = -1;
              break; 
            }
          }
          if(state_indicator == 0) hv->setVoltage(targetVoltage);
        }
      }
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" SpeedRamped to bias voltage(V):"<<hv->getVoltage()<<" current: "<<hv->getCurrent()<<std::endl;
      //      fString2 = new TObjString("");
      //      fString2->SetString(fString2->GetString().Append("Done Ramping.");
      //      answer2.WriteObject(fString);
      //      s->Send(answer2);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_LVVoltageLimit_")) {
      buffer_request.ReplaceAll("Set_LVVoltageLimit_","");
      TString chanbuf = buffer_request;
      chanbuf.Remove(chanbuf.First('_'),chanbuf.Sizeof()-chanbuf.First('_')-1);
      //      std::cout<<"channel: "<<chanbuf<<std::endl;
      Int_t channel = chanbuf.Atoi();
      buffer_request.Remove(0,buffer_request.Last('_')+1);
      //      std::cout<<"voltagelimit: "<<buffer_request<<std::endl;
      Double_t VoltageLimit = buffer_request.Atof();

      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" Attempt to set LV voltage limit for channel "<<channel<<" to: "<<VoltageLimit<<std::endl;
      state_indicator = lv->setVoltage(channel,VoltageLimit);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_LVCurrentLimit_")) {
      buffer_request.ReplaceAll("Set_LVCurrentLimit_","");
      TString chanbuf = buffer_request;
      chanbuf.Remove(chanbuf.First('_'),chanbuf.Sizeof()-chanbuf.First('_')-1);
      //      std::cout<<"channel: "<<chanbuf<<std::endl;
      Int_t channel = chanbuf.Atoi();
      buffer_request.Remove(0,buffer_request.Last('_')+1);
      //      std::cout<<"currentlimit: "<<buffer_request<<std::endl;
      Double_t CurrentLimit = buffer_request.Atof();

      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" Attempt to set LV current limit for channel "<<channel<<" to: "<<CurrentLimit<<std::endl;
      state_indicator = lv->setCurrent(channel,CurrentLimit);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_LVON_")) {
      buffer_request.ReplaceAll("Set_LVON_","");
      Int_t channel = buffer_request.Atoi();

      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" Attempt to set LV output ON for channel "<<channel<<std::endl;
      state_indicator = lv->setStatus(channel,1);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_LVOFF_")) {
      buffer_request.ReplaceAll("Set_LVOFF_","");
      Int_t channel = buffer_request.Atoi();

      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" Attempt to set LV output OFF for channel "<<channel<<std::endl;
      state_indicator = lv->setStatus(channel,0);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_ChillerTargetTemperature_")) {
      buffer_request.ReplaceAll("Set_ChillerTargetTemperature_","");
      Double_t targetTemperature = buffer_request.Atof();
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" Chiller TargetTemp set to: "<<targetTemperature<<std::endl;
      state_indicator = chiller->setTargetTemp(targetTemperature);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCTargetHumidity_")) {
      buffer_request.ReplaceAll("Set_CCTargetHumidity_","");
      Double_t CCTargetHumidity = buffer_request.Atof();
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC TargetHumidity set to: "<<CCTargetHumidity<<std::endl;
      Double_t CCTargetTemperature = CCtalk(1);
      state_indicator = (Double_t)CCtalk(1,true,false,false,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCTargetHumidityON_")) {
      buffer_request.ReplaceAll("Set_CCTargetHumidityON_","");
      Double_t CCTargetHumidity = buffer_request.Atof();
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC TargetHumidity set to: "<<CCTargetHumidity<<std::endl;
      Double_t CCTargetTemperature = CCtalk(1);
      state_indicator = (Double_t)CCtalk(1,true,true,true,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCTargetHumidityOFF_")) {
      buffer_request.ReplaceAll("Set_CCTargetHumidityOFF_","");
      Double_t CCTargetHumidity = buffer_request.Atof();
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC TargetHumidity set to: "<<CCTargetHumidity<<std::endl;
      Double_t CCTargetTemperature = CCtalk(1);
      state_indicator = (Double_t)CCtalk(1,true,true,false,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCTargetTemperature_")) {
      buffer_request.ReplaceAll("Set_CCTargetTemperature_","");
      Double_t CCTargetTemperature = buffer_request.Atof();
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC TargetTemperature set to: "<<CCTargetTemperature<<std::endl;
      Double_t CCTargetHumidity = CCtalk(3);
      state_indicator = (Double_t)CCtalk(1,true,false,false,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCTargetTemperatureON_")) {
      buffer_request.ReplaceAll("Set_CCTargetTemperatureON_","");
      Double_t CCTargetTemperature = buffer_request.Atof();
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC TargetTemperature set to: "<<CCTargetTemperature<<std::endl;
      Double_t CCTargetHumidity = CCtalk(3);
      state_indicator = (Double_t)CCtalk(1,true,true,true,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCTargetTemperatureOFF_")) {
      buffer_request.ReplaceAll("Set_CCTargetTemperatureOFF_","");
      Double_t CCTargetTemperature = buffer_request.Atof();
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC TargetTemperature set to: "<<CCTargetTemperature<<std::endl;
      Double_t CCTargetHumidity = CCtalk(3);
      state_indicator = (Double_t)CCtalk(1,true,true,false,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCOFF")) {
      buffer_request.ReplaceAll("Set_CCOFF","");
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC turning OFF! "<<std::endl;
      Double_t CCTargetTemperature = CCtalk(1);
      Double_t CCTargetHumidity = CCtalk(3);
      state_indicator = (Double_t)CCtalk(1,true,true,false,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_CCON")) {
      buffer_request.ReplaceAll("Set_CCON","");
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" CC turning ON! "<<std::endl;
      Double_t CCTargetTemperature = CCtalk(1);
      Double_t CCTargetHumidity = CCtalk(3);
      state_indicator = (Double_t)CCtalk(1,true,true,true,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString("");
      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      answer.WriteObject(fString);
    }
    else if (buffer_request.Contains("Set_EXPERIMENT")) {
      buffer_request.ReplaceAll("Set_EXPERIMENT","");
      date_root.Set();
      sdate_root = "";
      sdate_root+=date_root.GetDate();
      sdate_root.Append("_");
      sdate_root+=date_root.GetHour();
      sdate_root.Append(":");
      sdate_root+=date_root.GetMinute();
      sdate_root.Append(":");
      sdate_root+=date_root.GetSecond();

      std::cout<<sdate_root<<" EXPERIMENTAL RAMP!! "<<std::endl;

      hv->setVoltageRamp(-1.0);
      
      //      Double_t CCTargetTemperature = CCtalk(1);
      //      Double_t CCTargetHumidity = CCtalk(3);
      //      state_indicator = (Double_t)CCtalk(1,true,true,true,CCTargetTemperature,CCTargetHumidity);
      fString = new TObjString();
      //      fString->SetString(fString->GetString().Append(Form("%12.5g", (Double_t)state_indicator)));
      //      fString->SetString();
      answer.WriteObject(fString);
    }
    else
      Error("serial_spyserv::HandleSocket", "unexpected message");
    s->Send(answer);
  }
  // delete tinterlock;
  // delete chiller;
  // delete hv;

}


Double_t serial_spyserv::CCtalk(Int_t CCValueNr, Bool_t set_mode, Bool_t toggle_ONOFF, Bool_t turnON, Double_t temp, Double_t hum) {
  
  //      std::cout<<"CCtalk() called..."<<std::endl;
  TString setstring = "";
  char buffer[72] = {0};
  fSock = new TSocket("192.168.227.32",2049,72);
  fSock->SetOption(kNoBlock, 1);
  fSock->SendRaw("$01I\r\n",6);
  fSock->Select();
  fSock->RecvRaw(buffer,sizeof(buffer));
  // either we get or we set.. first we get:
  if ( !set_mode ) {
    //      printf("%s\n",buffer);
    fSock->Close();
    istringstream read(buffer);
    read >> val_one   ;
    read >> val_two   ;
    read >> val_three ;
    read >> val_four  ;
    read >> val_five  ;
    read >> val_six   ;
    
    if(      CCValueNr == 1 ) val_final = val_one  ;
    else if( CCValueNr == 2 ) val_final = val_two  ;
    else if( CCValueNr == 3 ) val_final = val_three;
    else if( CCValueNr == 4 ) val_final = val_four ;
    else if( CCValueNr == 5 ) val_final = val_five ;
    else if( CCValueNr == 6 ) val_final = val_six  ;
    else val_final = -999.;
  }
  // here we set:
  else {
    setstring = Form("$01E %07.2f %07.2f ", temp, hum);
    if ( !toggle_ONOFF ) { // default!
      setstring.Append("01\r\n"); // this string only changes the temp/hum, but does not change the ON/OFF state of the chamber
    }
    else {
      if( turnON ) {
        setstring.Append("010000000000\r\n"); // this string changes the temp/hum and DOES does change the ON/OFF state of the chamber 
      }
      else {
        setstring.Append("000000000000\r\n"); // this string changes the temp/hum and DOES does change the ON/OFF state of the chamber 
      }
    }
    //    const int cstrl = setstring.Length();
    //    const char csetstring[cstrl] = setstring.Data();
    //    csetstring[cstrl] = setstring.Data();
    // worked to turn  on: $01E 10 20 011000000000
    // worked to turn off: $01E 10 20 000000000000
    // std::cout<<"Setting CC to: "<<csetstring<<" (command length: "<<cstrl<<")"<<std::endl;
    //    fSock->SendRaw(csetstring,cstrl);
    fSock->SendRaw(setstring.Data(),setstring.Length());
    //TestCommand    fSock->SendRaw("$01E 0012.34 0023.45 00\r\n",25);
    fSock->Close();
    val_final = temp;
  }
  return val_final;
}

serial_spyserv::~serial_spyserv()
{
  // Clean up
  
  fSockets->Delete();
  delete fString;
  delete fSockets;
  delete fServ;
}

//void serial_spyserv::serial_spyserv()
//{
//  new serial_spyserv;
//}
