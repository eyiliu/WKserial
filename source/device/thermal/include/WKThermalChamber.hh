#ifndef WKTHERMALCHAMBER_HH
#define WKTHERMALCHAMBER_HH

#include "NanotecMotor.hh"
#include "CameraIR.hh"

#include <memory>


class WKThermalChamber{
public:
  WKThermalChamber(){};
  void Init();
  void MoveToPositionX(int x);
  void MoveToPositionY(int y);
  void MoveToPosition(int x, int y);
  void TakePhoto();
  
  std::unique_ptr<nanotec::NanotecMotor> m_mx, m_my;
  std::unique_ptr<CameraIR> m_ir;

  
};

#endif
