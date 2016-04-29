
#include "serial_spy.C"
void testbatch(){
  serial_spy * mySpy = new serial_spy();
  mySpy->Connect();
  
  while(1){
    mySpy->Request("Get_Hybrid1Temperature");
    mySpy->Request("Get_HVVoltage");
    mySpy->Request("Get_Hybrid2Temperature");
    mySpy->Request("Get_HVCurrent");
    mySpy->Request("Get_ChillerActualTemperature");
    mySpy->Request("Get_ChillerTargetTemperature");
    mySpy->Request("Get_ChuckTemperature");
    mySpy->Request("Get_ChuckHumidity");
    mySpy->Request("Get_ChuckDewpoint");
  }
    
}
