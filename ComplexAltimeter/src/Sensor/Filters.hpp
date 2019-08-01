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

class Filter
{
 public:
  virtual double step(double value)        = 0;
  virtual void reset(double startingValue) = 0;

  double getCurrentValue() { return currentValue; }

 protected:
  double currentValue;
};

// Returns the unweighted moving average of an input for n steps
class AveragingFilter : public Filter
{
 public:
  AveragingFilter(int numSteps, double startingValue)
  {
    steps  = numSteps;
    values = new double[steps];
    reset(startingValue);
  }

  ~AveragingFilter() { delete values; }

  double step(double nextValue);
  void reset(double startingValue);

 private:
  int steps;
  double *values;
  int valueIndex;
};

// Simple low pass filter
class LowPassFilter : public Filter
{
 public:
  LowPassFilter(double bias, double startingValue) { this->bias = bias; }

  double step(double value);
  void reset(double startingValue);

 private:
  double bias;
  double currentValue;
};

// Simple KalmanFilter
class KalmanFilter : public Filter
{
 public:
  KalmanFilter(double startingValue) { reset(startingValue); }

  double step(double measurement);
  void reset(double startingValue);
  void configure(double measuredError, double estimatedError, double gain);

 private:
  float err_measured  = 1;
  float err_estimated = 1;
  float q             = 0.001;
  float last_estimate = 0;

  float eErrorStarting = 1;
  float mErrorStarting = 1;
  float gainStarting   = 0.001;

};