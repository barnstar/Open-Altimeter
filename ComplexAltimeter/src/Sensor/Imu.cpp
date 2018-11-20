#include "Imu.hpp"

void Imu::reset()
{
  sensorFusion.begin(frequency);
}

void Imu::update()
{
  // Offsets applied to raw x/y/z values
  float mag_offsets[3] = {-0.20F, -5.53F, -35.34F};

  // Soft iron error compensation matrix
  float mag_softiron_matrix[3][3] = {{0.934, 0.005, 0.013},
                                     {0.005, 0.948, 0.012},
                                     {0.013, 0.012, 1.129}};

  if (mpuReady)
  {
    int status = imuSensor.readSensor();
    if (status == -1) {
      return;
    }
    accelleration = Vector(imuSensor.getAccelX_mss(),
                           imuSensor.getAccelY_mss(),
                           imuSensor.getAccelZ_mss());

    float x = imuSensor.getMagX_uT() + mag_offsets[0];
    float y = imuSensor.getMagX_uT() + mag_offsets[1];
    float z = imuSensor.getMagZ_uT() + mag_offsets[2];

    // Apply mag soft iron error compensation
    float mx = x * mag_softiron_matrix[0][0] + y * mag_softiron_matrix[0][1] + z * mag_softiron_matrix[0][2];
    float my = x * mag_softiron_matrix[1][0] + y * mag_softiron_matrix[1][1] + z * mag_softiron_matrix[1][2];
    float mz = x * mag_softiron_matrix[2][0] + y * mag_softiron_matrix[2][1] + z * mag_softiron_matrix[2][2];

    sensorFusion.update(
        imuSensor.getGyroX_rads(),
        imuSensor.getGyroY_rads(),
        imuSensor.getGyroZ_rads(),
        accelleration.XAxis,
        accelleration.YAxis,
        accelleration.ZAxis,
        mx,
        my,
        mz);

    heading.roll = sensorFusion.getRoll();
    heading.pitch = sensorFusion.getPitch();
    heading.yaw = sensorFusion.getYaw();
  }
}

double getAbsoluteAccelleration()
{
    return 0;
}

double getEstimatedVelocity()
{
    return 0;
}
