#ifndef WKTHERMALCHAMBER_HH
#define WKTHERMALCHAMBER_HH

#include "NanotecMotor.hh"
#include "CameraIR.hh"

#include <memory>


class WKThermalChamber{
public:
  WKThermalChamber(){};
  void Init(uint32_t d0, uint32_t d1, uint32_t d2);
  void FindReferencePostion();
  void MoveToPostion(int32_t x, int32_t y);
  void TakePhoto();

  
  std::unique_ptr<nanotec::NanotecMotor> m_mx, m_my;
  std::unique_ptr<CameraIR> m_ir;

  
};

#endif
