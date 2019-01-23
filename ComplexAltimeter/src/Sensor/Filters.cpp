/*********************************************************************************
 * Open Altimeter
 *
 * Mid power rocket avionics software for altitude recording and dual deployment
 *
 * Copyright 2018, Jonathan Nobels
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **********************************************************************************/

#include "Filters.hpp"
#include <Math.h>

//////////  Averaging /////////////

double AveragingFilter::step(double nextValue)
{
  values[valueIndex] = nextValue;
  valueIndex++;
  if (valueIndex >= steps) {
    valueIndex = 0;
  }

  double sum = 0;
  for (int i = 0; i < steps; i++) {
    sum += values[i];
  }
  currentValue = sum / steps;

  return currentValue;
}

void AveragingFilter::reset(double startingValue)
{
  for (int i = 0; i < steps; i++) values[i] = startingValue;
  currentValue = startingValue;
  valueIndex   = 0;
}

//////////  LowPass /////////////

double LowPassFilter::step(double nextValue)
{
  currentValue = currentValue * bias + nextValue * (1 - bias);
  return currentValue;
}

void LowPassFilter::reset(double startingValue)
{
  currentValue = startingValue;
}

//////////  Kalman /////////////

double KalmanFilter::step(double measurement)
{
  double kalman_gain = err_estimated / (err_estimated + err_measured);
  double current_estimate =
      last_estimate + kalman_gain * (measurement - last_estimate);

  err_estimated = (1.0 - kalman_gain) * err_estimated +
                  fabs(last_estimate - current_estimate) * q;
  last_estimate = current_estimate;

  currentValue = current_estimate;
  return current_estimate;
};

void KalmanFilter::reset(double startingValue)
{
  configure(1, 1, 0.001);
  this->last_estimate = startingValue;
  for (int i = 0; i < 4; i++) {
    step(startingValue);
  }
}

void KalmanFilter::configure(double measuredError, double estimatedError,
                             double gain)
{
  this->err_measured  = measuredError;
  this->err_estimated = estimatedError;
  this->q             = gain;
}