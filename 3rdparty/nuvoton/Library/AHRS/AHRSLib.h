/*============================================================================*
 * O     O          __                   ______  __                           *
 *  \   /      /\  / /_      _    __    / /___/ / /_     _                    *
 *   [+]      /  \/ / \\    //__ / /__ / /____ / / \\   //                    *
 *  /   \    / /\  /   \\__// --/ /---/ /----// /   \\_//                     *
 * O     O  /_/  \/     \__/    \_\/ /_/     /_/ ____/_/                      *
 *                                                                            *
 *                                                                            *
 * Multi-Rotor controller firmware for Nuvoton Cortex M4 series               *
 *                                                                            *
 * Written by by T.L. Shen for Nuvoton Technology.                            *
 * tlshen@nuvoton.com/tzulan611126@gmail.com                                  *
 *                                                                            *
 *============================================================================*
 */
#ifndef __AHRS_LIB_H__
#define __AHRS_LIB_H__
#include <stdint.h>
#define ROLL       0
#define PITCH      1
#define YAW      	 2

#define STATUS_NORMAL             0
#define STATUS_ERROR             -1
#define STATUS_GYRO_STEADY        0
#define STATUS_GYRO_NOT_STEADY   -1
#define STATUS_GYRO_CAL_BEGINE   -1
#define STATUS_GYRO_CAL_RUNNING   0
#define STATUS_GYRO_CAL_DONE      1
#define STATUS_GYRO_AXIS_CAL_DONE 2

#define STATUS_BUFFER_FILLED      0
#define STATUS_BUFFER_NOT_FILLED -1
#define STATUS_CAL_DONE           1

#define AXIS_X             0
#define AXIS_Y             1
#define AXIS_Z             2

#define CAL_X_UP           0
#define CAL_X_DOWN         1
#define CAL_Y_UP           2
#define CAL_Y_DOWN         3
#define CAL_Z_UP           4
#define CAL_Z_DOWN         5

#define ACC  0
#define GYRO 1
#define MAG  2
#define BARO 3
#define SENSOR_ACC        (1<<ACC)
#define SENSOR_GYRO       (1<<GYRO)
#define SENSOR_MAG        (1<<MAG)
#define SENSOR_BARO       (1<<BARO)
#define GYRO_CAL_DATA_SIZE    6
#define ACC_CAL_DATA_SIZE     6
#define MAG_CAL_DATA_SIZE    10
void nvtGetEulerRPY(float*);
/* w x y z */
void nvtGetQuaternion(float*);
void nvtGetNormAttitude(float*);
void nvtGetNormACC(float*);
void nvtGetMAGHeading(float*);
void nvtGetEulerNormMAG(float*);
void nvtGetNormMAG(float*);
void nvtGetGYRODegree(float*);
void nvtGetVelocity(float* Velocity);
void nvtGetMove(float* Move);
void nvtSetMove(float* Move);
void nvtResetMove(void);

void nvtGetCalibratedGYRO(float*);
void nvtGetCalibratedACC(float*);
void nvtGetCalibratedMAG(float*);

void nvtAHRSInit(void);
void nvtUpdateAHRS(uint8_t UPDATE);
void nvtMillisecondTick(void);
void nvtInputSensorRawACC(int16_t *raw);
void nvtInputSensorRawGYRO(int16_t *raw);
void nvtInputSensorRawMAG(int16_t *raw);
void nvtInputSensorRawBARO(int16_t *raw);
void nvtInputSensorRaw9D(int16_t *RawACC, int16_t *RawGYRO, int16_t *RawMAG);

void nvtGetAccZWithoutGravity(float *ZWithoutGravity, float *AccZMag);
void nvtGetAccOffset(float*);
void nvtGetAccScale(float*);
void nvtGetGyroOffset(float* );
void nvtGetGyroScale(float*);
void nvtGetMagCalMatrix(float*);
void nvtSetAccOffset(float* AccMean);
void nvtSetAccScale(float* AccScale);
void nvtSetGyroOffset(float* GyroMean);
void nvtSetGyroScale(float* GyroScale);
void nvtSetGYRODegPLSB(float DPLSB);
void nvtSetAccG_PER_LSB(float G_PER_LSB);
void nvtSetMagCalMatrix(float* MagCalMatrix);

void nvtGetSensorRawACC(int16_t *raw);
void nvtGetSensorRawGYRO(int16_t *raw);
void nvtGetSensorRawMAG(int16_t *raw);
void nvtGetSensorRawBARO(uint16_t *raw);
void nvtGetSensorRaw9D(int16_t *RawACC, int16_t *RawGYRO, int16_t *RawMAG);
//void nvtSetSensorEnable(char SensorType, char enable);

void nvtCalACCInit(void);
signed char nvtCalACCBufferFill(int8_t Dir);

signed char nvtGyroScaleCalibrate(int8_t axis);
signed char nvtGyroCenterCalibrate(void);
signed char nvtGyroIsSteady(void);
void nvtCalGyroInit(char axis);

signed char nvtCalMAGBufferFill(void);
void nvtCalMAGInit(void);
void nvtSetMagGaussPLSB(float);
uint8_t nvtGetMagCalQFactor(void);
void nvtSmoothSensorRawData(unsigned char enable, char sensor);
#endif	//__AHRS_LIB_H__


