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

#include "Imu.hpp"

void Imu::reset() { sensorFusion.begin(frequency); }

void Imu::update()
{
  // Offsets applied to raw x/y/z values
  float mag_offsets[3] = {-0.20F, -5.53F, -35.34F};

  // Soft iron error compensation matrix
  float mag_softiron_matrix[3][3] = {
      {0.934, 0.005, 0.013}, {0.005, 0.948, 0.012}, {0.013, 0.012, 1.129}};

  if (mpuReady) {
    int status = imuSensor.readSensor();
    if (status == -1) {
      return;
    }
    acceleration = Vector(imuSensor.getAccelX_mss(), 
                           imuSensor.getAccelY_mss(),
                           imuSensor.getAccelZ_mss());

    gyro = Vector(imuSensor.getGyroX_rads(), 
                  imuSensor.getGyroY_rads(),
                  imuSensor.getGyroZ_rads());

    float x = imuSensor.getMagX_uT() + mag_offsets[0];
    float y = imuSensor.getMagX_uT() + mag_offsets[1];
    float z = imuSensor.getMagZ_uT() + mag_offsets[2];

    // Apply mag soft iron error compensation
    float mx = x * mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] +
               z * mag_softiron_matrix[0][2];
    float my = x * mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] +
               z * mag_softiron_matrix[1][2];
    float mz = x * mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] +
               z * mag_softiron_matrix[2][2];

    sensorFusion.update(gyro.XAxis, 
                        gyro.YAxis,
                        gyro.ZAxis, 
                        acceleration.XAxis,
                        acceleration.YAxis, 
                        acceleration.ZAxis, 
                        mx, my, mz);

    heading.roll  = sensorFusion.getRoll();
    heading.pitch = sensorFusion.getPitch();
    heading.yaw   = sensorFusion.getYaw();
  }
}

double getAbsoluteAcceleration() { return 0; }

double getEstimatedVelocity() { return 0; }
