#ifndef CAMERAIR_HH
#define CAMERAIR_HH

#define KEY_S_LONG    "2,2B,4B"
#define KEY_S_SHORT   "2,2B,8B"
#define KEY_A_LONG    "2,27,47"
#define KEY_A_SHORT   "2,27,87"
#define KEY_T_LONG    "2,23,43"
#define KEY_T_SHORT   "2,23,83"
#define KEY_C         "2,38,98"
#define KEY_E         "2,2F,8F"
#define KEY_LEFT_PRS  "1,32"
#define KEY_LEFT_ACC  "1,52"
#define KEY_LEFT_RLS  "1,92"
#define KEY_RIGHT_PRS "1,2E"
#define KEY_RIGHT_ACC "1,4E"
#define KEY_RIGHT_RLS "1,8E"
#define KEY_UP_PRS    "1,2D"
#define KEY_UP_ACC    "1,4D"
#define KEY_UP_RLS    "1,8D"
#define KEY_DOWN_PRS  "1,31"
#define KEY_DOWN_ACC  "1,51"
#define KEY_DOWN_RLS  "1,91"



class CameraIR: public WKserial{
public:
  CameraIR(std::string devpath);
  void AutoFocus(){};
  void AutoImage(){};
  void Focus(){};
  void Calibrate(){};
  double GetCameraTemp(){};
private:
  

}



#endif
