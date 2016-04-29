#include "TCanvas.h"
#include "TPad.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"
#include "TString.h"
#include "TRegexp.h"
#include "TDatime.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TLatex.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TStyle.h"
// locals
#include "serial_spy.C"
// C++ includes
#include <stdio.h>  
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <strstream>
using namespace std;

//*************************************************************************;
//*** Macro to plot Slow Control data relevant for SCT Up DAQ *************;
//*** Recieves readings from serial data server               *************;
//*************************************************************************;

// *** Set Axis ranges...: ***
//   TGraph *gr1 = new TGraph (n,x,y);
//   TAxis *axis = gr1->GetXaxis();
//
//   axis->SetLimits(0.,5.);                 // along X
//   gr1->GetHistogram()->SetMaximum(20.);   // along          
//   gr1->GetHistogram()->SetMinimum(-20.);  //   Y     
//

double approxRollingAverage(double avg, double new_sample);

void CCSlowControlClient(Bool_t offline_mode = false, TString offlinefile="reallyall_data.txt", Double_t hours_width_of_plotting = 1){
  

  gErrorIgnoreLevel = 3000; // suppress verbosity when saving canvas as png
  gROOT->SetStyle("Plain");
  gStyle->SetPaperSize(20,26);

  TString  line    = "";
  Int_t    nlines  = 0;
  TString  stime   = "";
  TDatime  dtime;
  ifstream stream;
  Bool_t   fileendreached = false;
  TAxis*   axis;

  if(offline_mode) {
    stream.open(offlinefile);
  }

  // Load client and connect to server for data from serial ports
  //  gROOT->ProcessLine(".L serial_spy.C");
  serial_spy * mySpy = new serial_spy();
  //  mySpy->Connect("plejade01.ifh.de");
  if( !offline_mode ) {
    mySpy->Connect();
  }
   
  Double_t time, date_float, Hybrid1Temperature, HVVoltage, Hybrid2Temperature, HVCurrent, CCTemperature, CCTargetTemperature, ChuckTemperature, ChuckHumidity, ChuckDewpoint;
  Double_t prev_time, prev_date_float, prev_Hybrid1Temperature, prev_HVVoltage, prev_Hybrid2Temperature, prev_HVCurrent, prev_CCTemperature, prev_CCTargetTemperature, prev_ChuckTemperature, prev_ChuckHumidity, prev_ChuckDewpoint, average_slope;
  
  TCanvas *c1 = new TCanvas("c1","Slow Control Monitoring - in Climate Chamber");
  c1->Divide(2, 4, 0.01, 0.001);
  
  Int_t count = 0;
  
  TGraph *h_Hybrid1Temperature         = new TGraph(); 
  TGraph *h_HVVoltage                  = new TGraph();
  TGraph *h_Hybrid2Temperature         = new TGraph();
  TGraph *h_HVCurrent                  = new TGraph();
  TGraph *h_CCTemperature              = new TGraph();
  TGraph *h_ChuckTemperature           = new TGraph();
  TGraph *h_ChuckHumidity              = new TGraph();
  TGraph *h_ChuckDewpoint              = new TGraph();
  
  Int_t Col_h_Hybrid1Temperature       = kGreen+1; 
  Int_t Col_h_HVVoltage                = kGreen+1;
  Int_t Col_h_Hybrid2Temperature       = kGreen+1;
  Int_t Col_h_HVCurrent                = kGreen+1;
  Int_t Col_h_CCTemperature = kGreen+1;
  Int_t Col_h_ChuckTemperature         = kGreen+1;
  Int_t Col_h_ChuckHumidity            = kGreen+1;
  Int_t Col_h_ChuckDewpoint            = kGreen+1;
  
  TDatime date_root;
  TDatime current_time;
  TDatime old_time;    

  date_root.Set();
  TString datafilename = "";
  datafilename+=date_root.GetDate();
  datafilename.Append("_");
  datafilename+=date_root.GetHour();
  datafilename+=date_root.GetMinute();
  datafilename+=date_root.GetSecond();
  datafilename.Append("_data.txt");
  ofstream dataout;
  if(!offline_mode)  dataout.open(datafilename);
  if(!offline_mode)  dataout << "time, Hybrid1Temperature, HVVoltage, Hybrid2Temperature, HVCurrent, CCTemperature, CCTargetTemperature, ChuckTemperature, ChuckHumidity, ChuckDewpoint" << endl;
  
  for (Int_t i=0; 1; ++i) {
    //    c1->SetCanvasSize(c1->GetWindowWidth(), c1->GetWindowHeight());

    if(!offline_mode) { // getting live data from devices on serial ports
      nlines = i+1; // are using nlines to fill for offline mode, adjust it here accordingly.
      if(42 != 42) {    
        std::cout<<
//          "CCTemp: " <<mySpy->Request("Get_CCTemperature")<<
          " hybrid1Temp: "<<mySpy->Request("Get_Hybrid1Temperature")<<
          " hybrid2Temp: "<<mySpy->Request("Get_Hybrid2Temperature")<<
          " ChuckTemp: "  <<mySpy->Request("Get_ChuckTemperature")<<
          " chuckHum: "   <<mySpy->Request("Get_ChuckHumidity")<<
          " Dewpoint: "   <<mySpy->Request("Get_ChuckDewpoint")<<std::endl;
      }
      prev_Hybrid1Temperature       = Hybrid1Temperature       ;
      prev_HVVoltage                = HVVoltage                ;
      prev_Hybrid2Temperature       = Hybrid2Temperature       ;
      prev_HVCurrent                = HVCurrent                ;
      prev_CCTemperature = CCTemperature ;
      prev_CCTargetTemperature = CCTargetTemperature ;
      prev_ChuckTemperature         = ChuckTemperature         ;
      prev_ChuckHumidity            = ChuckHumidity            ;
      prev_ChuckDewpoint            = ChuckDewpoint            ;
      prev_date_float               = date_float               ;
      prev_time                     = time                     ;

      Hybrid1Temperature       = mySpy->Request("Get_Hybrid1Temperature");
      HVVoltage                = mySpy->Request("Get_HVVoltage");
      Hybrid2Temperature       = mySpy->Request("Get_Hybrid2Temperature");
      HVCurrent                = TMath::Abs( mySpy->Request("Get_HVCurrent"));
      CCTemperature = mySpy->Request("Get_CCTemperature");
      if( CCTemperature < -300. ) CCTemperature = -99.99;
      CCTargetTemperature = mySpy->Request("Get_CCTargetTemperature");
      if( CCTargetTemperature < -300. ) CCTargetTemperature = -99.99;
      ChuckTemperature         = mySpy->Request("Get_ChuckTemperature");
      ChuckHumidity            = mySpy->Request("Get_ChuckHumidity");
      ChuckDewpoint            = mySpy->Request("Get_ChuckDewpoint");
      date_root.Set();
      date_float               = 0.0;
      date_float               = date_root.Convert();
      time                     = date_float;
    }
    else { // we are in offline mode, will read a file
      if( line.ReadLine(stream) ) { // still lines to read
        istrstream stream5(line.Data());
        
        stream5 >> stime;
        stream5 >> Hybrid1Temperature;
        stream5 >> HVVoltage;
        stream5 >> Hybrid2Temperature;
        stream5 >> HVCurrent;
        stream5 >> CCTemperature;
        stream5 >> CCTargetTemperature;
        stream5 >> ChuckTemperature;
        stream5 >> ChuckHumidity;
        stream5 >> ChuckDewpoint;              
        
        if( stime.Contains("time") ) {
          continue; // if this does not read like philosopy :)
        }

        if( CCTemperature < -300. ) CCTemperature = -99.99;
        if( CCTargetTemperature < -300. ) CCTargetTemperature = -99.99;
        
        stime.Insert(4,"-");
        stime.Insert(7,"-");
        stime.Replace(10,1.," ");
        date_root.Set(stime.Data());
        time = date_root.Convert();

        if( !(nlines % 100) ) cout<<"At file line nr: "<<nlines<<endl;
        if( 42 != 42 ) {
          cout<<"We have read this: "
              <<time		    <<" "
              <<Hybrid1Temperature	    <<" "
              <<HVVoltage		    <<" "
              <<Hybrid2Temperature	    <<" "
              <<HVCurrent		    <<" "
              <<CCTemperature<<" "
              <<CCTargetTemperature<<" "
              <<ChuckTemperature        <<" "
              <<ChuckHumidity	    <<" "
              <<ChuckDewpoint           <<" "
              <<endl;
          getchar();
        }
        
        nlines++;
      }
      else { // file is done, we stop the loop
        fileendreached = true;
      }
    }

    // colourise BG of canvas in case of danger, e.g. we are aporoaching dewpoint     
    // ALL red states (ALARM):
    // ALL orange states (WARNING):
    // ALL blue states (UNDEFINED):
    // ALL green states (GOOD):
    // current stuff that has to be cleaned up and grouped as above:
//    if( (CCTemperature-5.) > ChuckDewpoint )  {
    if(Hybrid1Temperature > -55.) { // -55 to make sure it is a real measurement
      if( (Hybrid1Temperature-5.) > ChuckDewpoint )  {
	c1->SetFillColor(kGreen);  
	//      Col_h_CCTemperature = kGreen+1; 
	Col_h_Hybrid1Temperature = kGreen+1; 
	Col_h_ChuckDewpoint      = kGreen+1; 
      }
      //    if( (CCTemperature-5.) <= ChuckDewpoint ) {
      if( (Hybrid1Temperature-5.) <= ChuckDewpoint ) {
	c1->SetFillColor(kOrange+1);
	//      Col_h_CCTemperature   = kOrange+1; 
	Col_h_Hybrid1Temperature   = kOrange+1; 
	Col_h_ChuckDewpoint        = kOrange+1; 
      }
      //    if( (CCTemperature-3.) <= ChuckDewpoint ) {
      if( (Hybrid1Temperature-3.) <= ChuckDewpoint ) {
	c1->SetFillColor(kRed);
	//      Col_h_CCTemperature   = kRed; 
	Col_h_Hybrid1Temperature   = kRed; 
	Col_h_ChuckDewpoint        = kRed; 
      }
    }
    if( (Hybrid1Temperature) > 38. ) {
      c1->SetFillColor(kOrange+1);
      Col_h_Hybrid1Temperature  = kOrange+1; 
      if( (Hybrid1Temperature) > 48. ) {
        c1->SetFillColor(kRed);
        Col_h_Hybrid1Temperature  = kRed; 
      }
    }
    else {
      Col_h_Hybrid1Temperature  = kGreen+1; 
    }
    if(Hybrid2Temperature > -55.) { // -55 to make sure it is a real measurement
      if( (Hybrid2Temperature-5.) > ChuckDewpoint )  {
	c1->SetFillColor(kGreen);  
	//      Col_h_CCTemperature = kGreen+1; 
	Col_h_Hybrid2Temperature = kGreen+1; 
	Col_h_ChuckDewpoint      = kGreen+1; 
      }
      //    if( (CCTemperature-5.) <= ChuckDewpoint ) {
      if( (Hybrid2Temperature-5.) <= ChuckDewpoint ) {
	c1->SetFillColor(kOrange+1);
	//      Col_h_CCTemperature   = kOrange+1; 
	Col_h_Hybrid2Temperature   = kOrange+1; 
	Col_h_ChuckDewpoint        = kOrange+1; 
      }
      //    if( (CCTemperature-3.) <= ChuckDewpoint ) {
      if( (Hybrid2Temperature-3.) <= ChuckDewpoint ) {
	c1->SetFillColor(kRed);
	//      Col_h_CCTemperature   = kRed; 
	Col_h_Hybrid2Temperature   = kRed; 
	Col_h_ChuckDewpoint        = kRed; 
      }
    }
    if( (Hybrid2Temperature) > 38. ) {
      c1->SetFillColor(kOrange+1);
      Col_h_Hybrid2Temperature  = kOrange+1; 
      if( (Hybrid2Temperature) > 48. ) {
        c1->SetFillColor(kRed);
        Col_h_Hybrid2Temperature  = kRed; 
      }
    }
    else {
      Col_h_Hybrid2Temperature  = kGreen+1; 
    }
//    if( CCTemperature < -50 ) {
//      c1->SetFillColor(kBlue);
//      Col_h_CCTemperature = kBlue; 
//      Col_h_ChuckDewpoint            = kBlue; 
//    }
    if( Hybrid1Temperature < -50 && Hybrid2Temperature < -50 ) {
      c1->SetFillColor(kBlue);
      Col_h_Hybrid1Temperature = kBlue; 
      Col_h_ChuckDewpoint      = kBlue; 
      Col_h_Hybrid2Temperature = kBlue; 
    }
//    if( Hybrid2Temperature < -50 ) {
//      c1->SetFillColor(kBlue);
//      Col_h_Hybrid2Temperature = kBlue; 
//      Col_h_ChuckDewpoint      = kBlue; 
//    }
    if(!offline_mode)     c1->Modified();
    

    cout<<"Filling graphs at line: "<<i<<endl;
    current_time.Set();
    old_time.Set(current_time.GetYear(), current_time.GetMonth() ,current_time.GetDay(), current_time.GetHour()-hours_width_of_plotting, current_time.GetMinute(), current_time.GetSecond());

    h_Hybrid1Temperature->SetPoint(nlines-1, time, Hybrid1Temperature);
    h_Hybrid2Temperature->SetPoint(nlines-1, time, Hybrid2Temperature);
    h_HVVoltage->SetPoint(nlines-1, time, HVVoltage);
    h_HVCurrent->SetPoint(nlines-1, time, HVCurrent);
    h_CCTemperature->SetPoint(nlines-1, time, CCTemperature);
    h_ChuckTemperature->SetPoint(nlines-1, time, ChuckTemperature);
    h_ChuckHumidity->SetPoint(nlines-1, time, ChuckHumidity);
    h_ChuckDewpoint->SetPoint(nlines-1, time, ChuckDewpoint);

    if( !offline_mode || !(nlines % 10000) || fileendreached ) {
      c1->cd(1);
      //    gSystem->Sleep(500);
      h_Hybrid1Temperature->SetLineColor(kRed);
      if(offline_mode) {
        h_Hybrid1Temperature->SetLineStyle(1);
        h_Hybrid1Temperature->SetMarkerStyle(6);
        h_Hybrid1Temperature->SetMarkerColor(kRed);
        h_Hybrid1Temperature->Draw("AP");
      }
      else {
        if(42 == 42) {       
          axis = h_Hybrid1Temperature->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_Hybrid1Temperature->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(2);
      h_Hybrid2Temperature->SetLineColor(kBlack);
      if(offline_mode) {
        h_Hybrid2Temperature->SetLineStyle(1);
        h_Hybrid2Temperature->SetMarkerStyle(6);
        h_Hybrid2Temperature->SetMarkerColor(kBlack);
        h_Hybrid2Temperature->Draw("AP");
      }
      else {
        if(42 == 42) {       
          axis = h_Hybrid2Temperature->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_Hybrid2Temperature->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(3);
      h_HVVoltage->SetLineColor(kRed);
      if(offline_mode) {
        h_HVVoltage->SetLineStyle(1);
        h_HVVoltage->SetMarkerStyle(6);
        h_HVVoltage->SetMarkerColor(kRed);
        h_HVVoltage->Draw("AP");
      }
      else{
        if(42 == 42) {       
          axis = h_HVVoltage->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_HVVoltage->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(4);
      h_HVCurrent->SetLineColor(kBlack);
      if(offline_mode) {
        h_HVCurrent->SetLineStyle(1);
        h_HVCurrent->SetMarkerStyle(6);
        h_HVCurrent->SetMarkerColor(kBlack);
        h_HVCurrent->Draw("AP");
      }
      else{
        if(42 == 42) {       
          axis = h_HVCurrent->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_HVCurrent->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(5);
      h_CCTemperature->SetLineColor(kBlue);
      if(offline_mode) {
        h_CCTemperature->SetLineStyle(1);
        h_CCTemperature->SetMarkerStyle(6);
        h_CCTemperature->SetMarkerColor(kBlue);
        h_CCTemperature->Draw("AP");
      }
      else{
        if(42 == 42) {       
          axis = h_CCTemperature->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_CCTemperature->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(6);
      h_ChuckTemperature->SetLineColor(kGreen+4);
      if(offline_mode) {
        h_ChuckTemperature->SetLineStyle(1);
        h_ChuckTemperature->SetMarkerStyle(6);
        h_ChuckTemperature->SetMarkerColor(kGreen+4);
        h_ChuckTemperature->Draw("AP");
      }
      else{
        if(42 == 42) {       
          axis = h_ChuckTemperature->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_ChuckTemperature->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(7);
      h_ChuckHumidity->SetLineColor(kRed);
      if(offline_mode) {
        h_ChuckHumidity->SetLineStyle(1);
        h_ChuckHumidity->SetMarkerStyle(6);
        h_ChuckHumidity->SetMarkerColor(kRed);
        h_ChuckHumidity->Draw("AP");
      }
      else{
        if(42 == 42) {       
          axis = h_ChuckHumidity->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_ChuckHumidity->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(8);
      h_ChuckDewpoint->SetLineColor(kRed);
      if(offline_mode) {
        h_ChuckDewpoint->SetLineStyle(1);
        h_ChuckDewpoint->SetMarkerStyle(6);
        h_ChuckDewpoint->SetMarkerColor(kRed);
        h_ChuckDewpoint->Draw("AP");
      }
      else{
        if(42 == 42) {       
          axis = h_ChuckDewpoint->GetXaxis();
          axis->SetLimits(old_time.Convert(), current_time.Convert()); 
        }
        h_ChuckDewpoint->Draw("AL");
      }
      gPad->Modified();
      
      c1->cd(1);
      //h_Hybrid1Temperature->SetTitle("Hybrid1Tmp");//Temperature of the charged hybrid. Temperature of over 30C is not tolerated.
      h_Hybrid1Temperature->GetXaxis()->SetTitle("time(GMT)");
      h_Hybrid1Temperature->GetXaxis()->SetTimeDisplay(1);
      h_Hybrid1Temperature->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_Hybrid1Temperature->GetXaxis()->SetTimeOffset(0,"gmt");    
      h_Hybrid1Temperature->GetYaxis()->SetTitle("Hybrid 1 Temperature (#circC)");
      TString CurrentValue_1 = "";
      CurrentValue_1.Append( Form("%2.1f", Hybrid1Temperature ) );
      CurrentValue_1.Append("#circC Hybrid 1 Temperature");
      TLatex *   lable_1 = new TLatex(0.3,0.925,CurrentValue_1);
      lable_1->SetNDC();
      lable_1->SetTextSize(0.08);
      lable_1->SetTextColor(Col_h_Hybrid1Temperature);
      lable_1->Draw();
      gPad->Modified();
      
      c1->cd(2);
      //h_Hybrid2Temperature->SetTitle("Hybrid2Tmp");//Temperature of the charged hybrid. Temperature of over 30C is not tolerated.
      h_Hybrid2Temperature->GetXaxis()->SetTitle("time(GMT)");
      h_Hybrid2Temperature->GetYaxis()->SetTitle("Hybrid 2 Temperature (#circC)");
      h_Hybrid2Temperature->GetXaxis()->SetTimeDisplay(1);
      h_Hybrid2Temperature->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_Hybrid2Temperature->GetXaxis()->SetTimeOffset(0,"gmt");    
      TString CurrentValue_2 = "";
      CurrentValue_2.Append( Form("%2.1f", Hybrid2Temperature ) );
      CurrentValue_2.Append("#circC Hybrid 2 Temperature");
      TLatex *   lable_2 = new TLatex(0.3,0.925,CurrentValue_2);
      lable_2->SetNDC();
      lable_2->SetTextSize(0.08);
      lable_2->SetTextColor(Col_h_Hybrid2Temperature);
      lable_2->Draw();
      // show last updated string on canvas
      current_time.Set();
      TString current_time_string = "Last Updated: ";
      current_time_string.Append(current_time.AsSQLString());
      if( offline_mode ) {
        current_time_string = "OFFLINE MODE";
      }
      TLatex *   lable_date = new TLatex(0.8,0.95,current_time_string);
      lable_date->SetNDC();
      lable_date->SetTextSize(0.04);
      lable_date->SetTextColor(kGreen+1);
      if( offline_mode ) {
        lable_date->SetTextColor(kBlue);
      }
      
      lable_date->Draw();
      gPad->Modified();
      
      c1->cd(3);
      //h_HVVoltage->SetTitle("SensorVoltage");//
      h_HVVoltage->GetXaxis()->SetTitle("time(GMT)");
      h_HVVoltage->GetYaxis()->SetTitle("Biasvoltage (V)");
      h_HVVoltage->GetXaxis()->SetTimeDisplay(1);
      h_HVVoltage->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_HVVoltage->GetXaxis()->SetTimeOffset(0,"gmt");    
      TString CurrentValue_1_1 = "";
      CurrentValue_1_1.Append( Form("%2.1g", HVVoltage ) );
      CurrentValue_1_1.Append("V Biasvoltage");
      TLatex *   lable_1_1 = new TLatex(0.3,0.925,CurrentValue_1_1);
      lable_1_1->SetNDC();
      lable_1_1->SetTextSize(0.08);
      lable_1_1->SetTextColor(Col_h_HVVoltage);
      lable_1_1->Draw();
      gPad->Modified();
      
      c1->cd(4);
      //h_HVCurrent->SetTitle("SensorCurrent");
      h_HVCurrent->GetXaxis()->SetTitle("time(GMT)");
      h_HVCurrent->GetYaxis()->SetTitle("Total Sensor Current (A)");
      h_HVCurrent->GetXaxis()->SetTimeDisplay(1);
      h_HVCurrent->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_HVCurrent->GetXaxis()->SetTimeOffset(0,"gmt");    
      TString CurrentValue_2_1 = "";
      CurrentValue_2_1.Append( Form("%2.1g", HVCurrent ) );
      CurrentValue_2_1.Append("A Total Sensor Current");
      TLatex *   lable_2_1 = new TLatex(0.3,0.925,CurrentValue_2_1);
      lable_2_1->SetNDC();
      lable_2_1->SetTextSize(0.08);
      lable_2_1->SetTextColor(Col_h_HVCurrent);
      lable_2_1->Draw();
      gPad->Modified();
      
      c1->cd(5);
      //    h_CCTemperature->SetTitle("CCTmp");//Temperature of actual chilling liquid temperature
      h_CCTemperature->GetXaxis()->SetTitle("time(GMT)");
      h_CCTemperature->GetYaxis()->SetTitle("CC Temperature (#circC)");
      h_CCTemperature->GetXaxis()->SetTimeDisplay(1);
      h_CCTemperature->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_CCTemperature->GetXaxis()->SetTimeOffset(0,"gmt");    
      TString CurrentValue_3 = "";
      if ( CCTargetTemperature > -50. ) {
        CurrentValue_3.Append( Form("%2.1f", CCTemperature ) );
      }
      else {
        CurrentValue_3.Append( "UNDEF " );
      }
      CurrentValue_3.Append("#circC CC Temp (Target: ");
      if ( CCTargetTemperature > -50. ) {
        CurrentValue_3.Append( Form("%2.1f", CCTargetTemperature ) );
      }
      else {
        CurrentValue_3.Append( " UNDEF " );
      }
      CurrentValue_3.Append("#circC, Slope ");
      average_slope = approxRollingAverage(average_slope,60.*(CCTemperature-prev_CCTemperature)/(time-prev_time));
      if(i>30) {
	CurrentValue_3.Append( Form("%2.2f", average_slope ) );
      }
      else {
	CurrentValue_3.Append( Form("%2.2f",  60.*(CCTemperature-prev_CCTemperature)/(time-prev_time) ) );
      }
      CurrentValue_3.Append("#circC/min, ");
      if((60.*(CCTemperature-prev_CCTemperature)/(time-prev_time))) {     
//	CurrentValue_3.Append( Form("%2.2f",  TMath::Abs((CCTemperature-CCTargetTemperature)/(60.*(CCTemperature-prev_CCTemperature)/(time-prev_time))) ) ); 
	CurrentValue_3.Append( Form("%2.2f",  TMath::Abs((CCTemperature-CCTargetTemperature)/average_slope) ) ); 
      }
      else {
	CurrentValue_3.Append(" UNDEF"); 
      }
      CurrentValue_3.Append(" min to reach TargetTemp)");

      TLatex *   lable_3 = new TLatex(0.05,0.925,CurrentValue_3);
      lable_3->SetNDC();
      lable_3->SetTextSize(0.066);
      lable_3->SetTextColor(Col_h_CCTemperature);
      lable_3->Draw();
      gPad->Modified();
      
      c1->cd(6);
      //h_ChuckTemperature->SetTitle("AirTemp");//Temperature of air in the box
      h_ChuckTemperature->GetXaxis()->SetTitle("time(GMT)");
      h_ChuckTemperature->GetYaxis()->SetTitle("Chuck Temperature (#circC)");
      h_ChuckTemperature->GetXaxis()->SetTimeDisplay(1);
      h_ChuckTemperature->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_ChuckTemperature->GetXaxis()->SetTimeOffset(0,"gmt");    
      TString CurrentValue_4 = "";
      CurrentValue_4.Append( Form("%2.1f", ChuckTemperature ) );
      CurrentValue_4.Append("#circC Chuck Temperature");
      TLatex *   lable_4 = new TLatex(0.3,0.925,CurrentValue_4);
      lable_4->SetNDC();
      lable_4->SetTextSize(0.08);
      lable_4->SetTextColor(Col_h_ChuckTemperature);
      lable_4->Draw();
      gPad->Modified();
      
      c1->cd(7);
      //    h_ChuckHumidity->SetTitle("RHumidity");//Relative humidity of air in the box(saturation percentage)
      h_ChuckHumidity->GetXaxis()->SetTitle("time(GMT)");
      h_ChuckHumidity->GetYaxis()->SetTitle("Chuck Rel. Humidity (\%)");
      h_ChuckHumidity->GetXaxis()->SetTimeDisplay(1);
      h_ChuckHumidity->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_ChuckHumidity->GetXaxis()->SetTimeOffset(0,"gmt");    
      TString CurrentValue_5 = "";
      CurrentValue_5.Append( Form("%2.1f", ChuckHumidity ) );
      CurrentValue_5.Append("\% Chuck Rel. Humidity");
      TLatex *   lable_5 = new TLatex(0.3,0.925,CurrentValue_5);
      lable_5->SetNDC();
      lable_5->SetTextSize(0.08);
      lable_5->SetTextColor(Col_h_ChuckHumidity);
      lable_5->Draw();
      gPad->Modified();
      
      c1->cd(8);
      //    h_ChuckDewpoint->SetTitle("DewPoint");//Temperature of dew point in the box. Temperature below the dew point is not tolerated.
      h_ChuckDewpoint->GetXaxis()->SetTitle("time(GMT)");
      h_ChuckDewpoint->GetYaxis()->SetTitle("Chuck Dewpoint (#circC)");
      h_ChuckDewpoint->GetXaxis()->SetTimeDisplay(1);
      h_ChuckDewpoint->GetXaxis()->SetTimeFormat("%Y-%m-%d -- %H:%M:%S");
      h_ChuckDewpoint->GetXaxis()->SetTimeOffset(0,"gmt");    
      
      TString CurrentValue_6 = "";
      CurrentValue_6.Append( Form("%2.1f", ChuckDewpoint ) );
      CurrentValue_6.Append("#circC Chuck Dewpoint ");
      TLatex *   lable_6 = new TLatex(0.3,0.925,CurrentValue_6);
      lable_6->SetNDC();
      lable_6->SetTextSize(0.08);
      lable_6->SetTextColor(Col_h_ChuckDewpoint);
      lable_6->Draw();
    }
    
    if(!offline_mode)   gPad->Modified();
    
    if(!offline_mode)     c1->Modified();
    if(!offline_mode)     c1->Update();
    
    if(!offline_mode) {
      c1->Print("current_SC.png");
      c1->Print("current_SC.svg");
      ++count;
      if(count > 30) { // copy SC picture to Ubuntu synced folder every ~5 minutes (149 ticks)
        gSystem->Exec("copy current_SC.png C\:\\Users\\ibloch\\Slowcontrol\\");
//        gSystem->Exec("copy current_SC.svg C\:\\Users\\ibloch\\Slowcontrol\\");
//        gSystem->Exec("copy current_SC.png H\:\\www\\SlowControl");
//        gSystem->Exec("copy current_SC.svg H\:\\www\\SlowControl");
        gSystem->Exec("copy current_SC.png G\:\\Owncloud_desyhh_ibl\\SlowControls");
//        gSystem->Exec("copy current_SC.svg H\:\\www\\SlowControl");
        count = 0;
      }
      dataout << date_root.GetDate()<<"-"<<date_root.GetHour()<<":"<<date_root.GetMinute()<<":"<<date_root.GetSecond()<<" "<< Hybrid1Temperature <<" "<< HVVoltage <<" "<< Hybrid2Temperature <<" "<< HVCurrent <<" "<< CCTemperature <<" "<< CCTargetTemperature <<" "<< ChuckTemperature <<" "<< ChuckHumidity <<" "<< ChuckDewpoint << endl;
      
      if (gSystem->ProcessEvents()) {
        break; 
      }
      gSystem->Sleep(500);
    }
    if(fileendreached) break;
  }
}


double approxRollingAverage(double avg, double new_sample) {

    avg -= avg /30.;
    avg += new_sample /30.;

    return avg;
}
