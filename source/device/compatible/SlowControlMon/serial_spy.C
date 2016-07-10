// Client program which allows the snooping of objects from a serial_spyserv process.
// To run this do the following (also see serial_spyserv.C):
//   - open two or more windows
//   - start root in all windows
//   - execute in the first window:    .x serial_spyserv.C  (or serial_spyserv.C++)
//   - execute in the other window(s): .L serial_spy.C      (or serial_spy.C++)
//   - in the "serial_spy" client windows get a new spy by:
//  
//Author: Fons Rademakers
   
#include "TObjString.h"
#include "TSocket.h"
#include "TMessage.h"
#include "RQ_OBJECT.h"
#include "TString.h"
#include "TRegexp.h"
#include "TDatime.h"
#include "TColor.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TMath.h"
// C++ includes
#include <stdio.h>  
#include <iostream>
#include <fstream>
#include <sstream>
//#include <strstream>
using namespace std;

class serial_spy {

RQ_OBJECT("serial_spy")

private:
   TSocket             *fSock;
   TObjString          *fString;

public:
   serial_spy();
   ~serial_spy();

   void Connect();
   void GetAnswer();
   Double_t GetAnswer(TString command);
   Double_t Get(TString command);
   Double_t Request(TString command);
   TString  RequestString(TString command);
   void Help(void);

};

void serial_spy::GetAnswer()
{
   // Ask for the answer

   if (!fSock->IsValid())
      return;

   fSock->Send("What is the answer?");

   TMessage *mess;
   if (fSock->Recv(mess) <= 0) {
      Error("serial_spy::GetAnswer", "error receiving message");
      return;
   }

//   if (mess->GetClass()->InheritsFrom(TString::Class())) {
      fString = (TObjString*) mess->ReadObject(mess->GetClass());
      std::cout<<"The answer is: "<<fString->GetString()<<std::endl;
//   }

   delete mess;
}

Double_t serial_spy::Request(TString command)
{
   // Ask for the answer

   Double_t measured_value = -999.; 

   if (!fSock->IsValid())
      return -1.;

   fSock->Send(command);

   TMessage *mess;
   if (fSock->Recv(mess) <= 0) {
      Error("serial_spy::Request", "error receiving message");
      return -1.;
   }

    fString = (TObjString*) mess->ReadObject(mess->GetClass());
    std::cout<<"serial_spy::Request returned: "<<fString->GetString()<<" for command: "<<command<<std::endl;

   delete mess;
   measured_value = fString->GetString().Atof();
   return measured_value;
}

TString serial_spy::RequestString(TString command)
{
   // Ask for the answer
   if (!fSock->IsValid())
      return -1.;

   fSock->Send(command);

   TMessage *mess;
   if (fSock->Recv(mess) <= 0) {
      Error("serial_spy::RequestString", "error receiving message");
      return -1.;
   }

    fString = (TObjString*) mess->ReadObject(mess->GetClass());
    std::cout<<"serial_spy::RequestString returned: "<<fString->GetString()<<" for command: "<<command<<std::endl;

   delete mess;
//   measured_value = ;
   return fString->GetString();
}

Double_t serial_spy::Get(TString command)
{
   // Ask for the answer, "Get" kept for backwards compatibility after
   // renaming to "Request" 140304 IBL
   return Request(command);
}

void serial_spy::Help(void)
{
   // 
   Request("Help");
}

void serial_spy::Connect()
{
   // Connect to serial_spyserv
   fSock = new TSocket("plejade02.ifh.de", 9090);
   std::cout<<"connected..."<<std::endl;
   Request("Help");
}

serial_spy::serial_spy()
{
  std::cout<<"Welcome to a new serial_spy..."<<std::endl;

}

serial_spy::~serial_spy()
{
   // Clean up

  //   delete fHist;
   delete fSock;
}
