#include "WKThermalChamber.hh"

#include <experimental/filesystem>

#include <iostream>
#include <thread>
#include <chrono>

namespace filesystem = std::experimental::filesystem;

void WKThermalChamber::Init(uint32_t d0, uint32_t d1, uint32_t d2){
  // const std::string dir = "/dev";
  // const std::string module_prefix("ttyUSB");
  // for(auto& e: filesystem::directory_iterator(filesystem::absolute(dir))){
  //   filesystem::path file(e);
  //   std::string fname = file.filename().string();
  //   if(!fname.compare(0, module_prefix.size(), module_prefix)){
  //     if(!m_mx){
  // 	m_mx.reset(new nanotec::NanotecMotor(file.string()));
  // 	if(m_mx->getControllerBaudRate()==115200 && m_mx->getMotorID()==0){
  // 	  continue;
  // 	}
  // 	else{
  // 	  m_mx.reset();
  // 	}
  //     }
  //     if(!m_my){
  // 	m_my.reset(new nanotec::NanotecMotor(file.string()));
  // 	if(m_my->getControllerBaudRate()==115200 && m_my->getMotorID()==1){
  // 	  continue;
  // 	}else{
  // 	  m_my.reset();
  // 	}
  //     }
  //     if(!m_ir){
  // 	m_ir.reset(new CameraIR(file.string()));
  // 	//if(test) reset//
  //     }
  //   }
  // }

  std::string devpath_base = "/dev/ttyUSB";
  m_mx.reset(new nanotec::NanotecMotor(devpath_base+std::to_string(d0)));
  m_my.reset(new nanotec::NanotecMotor(devpath_base+std::to_string(d1)));
  
  if(m_mx->getMotorID()!=0){
    m_mx.swap(m_my);
  }

  m_ir.reset(new CameraIR(devpath_base+std::to_string(d2)));
  
}


void WKThermalChamber::FindReferencePostion(){
  m_mx->setLimitSwitchBehaviour(2,8,2,2);//ext_ref_back
  m_mx->setPositionMode(4);//ext_ref
  m_mx->setDirection(0);
  m_mx->setTravelDistance(100);
  m_mx->setRampType(2);

  m_my->setLimitSwitchBehaviour(2,8,1,2);//ext_ref_fwd
  m_my->setPositionMode(4);//ext_ref
  m_my->setDirection(1);
  m_my->setTravelDistance(100);
  m_my->setRampType(2);


  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  while(1){
    if(m_mx->isStatusReady()&&m_my->isStatusReady()){
      m_mx->startMotor();
      m_my->startMotor();
      break;
    }
    else{
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      std::cerr<<"motor can not be ready\n";
    }
  }
  
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  while(1){
    if(m_mx->isStatusZeroPos()&&m_my->isStatusZeroPos()){
      break;
    }
    else{
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::cout<<"waiting...\n";
    }
  }
  

  m_mx->setPositionMode(2);//abs
  m_my->setPositionMode(2);//abs
}


void WKThermalChamber::MoveToPostion(int32_t x, int32_t y){
  bool errPos = false;
  
  m_mx->setTravelDistance(x);
  m_my->setTravelDistance(y);

  if(m_mx->isStatusErrorPos()){
    m_mx->resetPositionError();
  }
  if(m_my->isStatusErrorPos()){
    m_my->resetPositionError();
  }
  
  if(m_mx->isStatusReady()&&m_my->isStatusReady()){
    m_mx->startMotor();
    m_my->startMotor();
  }

  while(1){
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if(m_mx->isStatusReady() && m_my->isStatusReady())
      break;
    if(m_mx->isStatusErrorPos()){
      m_mx->resetPositionError();
      errPos = true;
    }

    if(m_my->isStatusErrorPos()){
      m_my->resetPositionError();
      errPos = true;
    }
  }
  
  if(errPos){
    std::cerr<<"error postion, move again\n"; // NOTE, WARNING: Could it be infinite?
    MoveToPostion(x, y);
  }
    
}


void WKThermalChamber::TakePhoto(){
  m_ir->TakePhoto();
}
