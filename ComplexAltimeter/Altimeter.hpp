#ifndef altimeter_h
#define alitmeter_h

#include <Adafruit_BMP280.h> 


class KalmanFilter
{
  private:

  float err_measured = 0;
  float err_estimated = 0 ;
  float q = 0;
  float last_estimate = 0;

  public:

  KalmanFilter() {
    this->reset(1,1,0.001);
  } 

  ~KalmanFilter() {};
  
  public:
  double step(double measurement)
  { 
    double kalman_gain = err_estimated/(err_estimated + err_measured);
    double current_estimate = last_estimate + kalman_gain * (measurement - last_estimate);
    
    err_estimated =  (1.0 - kalman_gain)* err_estimated + fabs(last_estimate-current_estimate)*q;
    last_estimate= current_estimate;
  
    return current_estimate;
  };

  void reset(double measuredError, double estimatedError, double gain)
  {
     this->err_measured=measuredError;
     this->err_estimated=estimatedError;
     this->q = gain;
     this->last_estimate = 0;
  };
};


class Altimeter
{
   public:
   Altimeter();
   ~Altimeter();
   
   void start();
   bool isReady();

   double getAltitude();
   double referenceAltitude();

   void scanI2cBus();
      
   private:
   Adafruit_BMP280 barometer;
   bool            barometerReady;
   double          refAltitude;
   KalmanFilter    filter;
};

#endif
