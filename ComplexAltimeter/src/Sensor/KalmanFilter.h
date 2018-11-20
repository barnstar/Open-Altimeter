#ifndef FILTER_H
#define FILTER_H

class KalmanFilter
{
private:
  float err_measured = 0;
  float err_estimated = 0;
  float q = 0;
  float last_estimate = 0;

public:
  KalmanFilter()
  {
    this->reset(1, 1, 0.001);
  }

  ~KalmanFilter(){};

  double step(double measurement);
  double lastEstimate();

  void reset(double measuredError, double estimatedError, double gain);
};

#endif
