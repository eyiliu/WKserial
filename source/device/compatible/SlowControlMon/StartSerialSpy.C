#include "serial_spy.C"

serial_spy * spy;

void StartSerialSpy( void ) {
  //  gROOT->ProcessLine(".L serial_spy.C");
  spy = new serial_spy();
  spy->Connect();
//  spy->Request("Get_ChuckDewpoint");
  std::cout<<"**********************************************************"<<std::endl;
  std::cout<<"**********************************************************"<<std::endl;
  std::cout<<"**** Serial Spy instance is called ->spy<-            ****"<<std::endl;
  std::cout<<"**** Usage e.g.                                       ****"<<std::endl;
  std::cout<<"**** root [N]  spy->Request(\"Set_HVVoltageRamp_-15\")  ****"<<std::endl;
  std::cout<<"**** Further query options see above...               ****"<<std::endl;
  std::cout<<"**********************************************************"<<std::endl;
  std::cout<<"**********************************************************"<<std::endl;
}
