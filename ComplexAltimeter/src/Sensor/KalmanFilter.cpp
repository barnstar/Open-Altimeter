#include "KalmanFilter.h"
#include <math.h>

double KalmanFilter::step(double measurement)
{
    double kalman_gain = err_estimated / (err_estimated + err_measured);
    double current_estimate = last_estimate + kalman_gain * (measurement - last_estimate);

    err_estimated = (1.0 - kalman_gain) * err_estimated + fabs(last_estimate - current_estimate) * q;
    last_estimate = current_estimate;

    return current_estimate;
};

double KalmanFilter::lastEstimate()
{
    return last_estimate;
}

void KalmanFilter::reset(double measuredError, double estimatedError, double gain)
{
    this->err_measured = measuredError;
    this->err_estimated = estimatedError;
    this->q = gain;
    this->last_estimate = 0;
}
