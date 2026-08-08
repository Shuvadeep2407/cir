/**
  ******************************************************************************
  * @file    gps_compass.h
  * @brief   Combined NEO-6M GPS + GY-271 HMC5883L Compass Sensor Library
  *
  *          NEO-6M GPS:     UART @ 9600, NMEA 0183 (GPGGA/GPRMC)
  *          HMC5883L Compass: I2C @ 0x1E, 3-axis magnetometer, heading
  ******************************************************************************
  */

#ifndef __GPS_COMPASS_H
#define __GPS_COMPASS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "i2c.h"

/*============================================================================*/
/* NEO-6M GPS Configuration                                                   */
/*============================================================================*/
#define GPS_UART_BAUD           9600
#define GPS_NMEA_MAX_LEN        100
#define GPS_BUFFER_SIZE         GPS_NMEA_MAX_LEN

/*============================================================================*/
/* HMC5883L Compass Configuration                                             */
/*============================================================================*/
#define COMPASS_I2C_ADDR        0x1E    /* 7-bit I2C address */

#define COMPASS_REG_CONFIG_A    0x00
#define COMPASS_REG_CONFIG_B    0x01
#define COMPASS_REG_MODE        0x02
#define COMPASS_REG_DATA_X_MSB  0x03
#define COMPASS_REG_DATA_Z_MSB  0x05
#define COMPASS_REG_DATA_Y_MSB  0x07
#define COMPASS_REG_STATUS      0x09
#define COMPASS_REG_ID_A        0x0A

#define COMPASS_CFG_A_8AVG      0x70    /* 8 samples, 15Hz */
#define COMPASS_CFG_B_GAIN_1_3  0x20    /* ±1.3 Ga, 1090 LSB/G */
#define COMPASS_GAIN_SCALE      0.92f   /* mG per LSB */
#define COMPASS_MODE_CONTINUOUS 0x00

/* Magnetic declination (degrees) - adjust for your location */
#define COMPASS_DECLINATION     0.07f

/*============================================================================*/
/* Data Structures                                                            */
/*============================================================================*/

/* GPS data */
typedef struct
{
    float   latitude;           /* Decimal degrees (+N, -S) */
    float   longitude;          /* Decimal degrees (+E, -W) */
    float   altitude_m;         /* Altitude in meters */
    float   speed_kmph;         /* Speed in km/h */
    uint8_t fix_quality;        /* 0=invalid, 1=GPS fix, 2=DGPS */
    uint8_t satellites;         /* Number of satellites */
    uint8_t utc_hour;
    uint8_t utc_minute;
    uint8_t utc_second;
    uint8_t valid;              /* 1 = valid fix */
    uint8_t updated;            /* 1 = new data */
} GPS_Data_t;

/* Compass data */
typedef struct
{
    int16_t x_raw;
    int16_t y_raw;
    int16_t z_raw;
    float   heading_deg;        /* 0-360 clockwise from North */
    uint8_t data_ready;
    uint8_t valid;              /* 1 = sensor present */
} Compass_Data_t;

/* Combined sensor data */
typedef struct
{
    GPS_Data_t     gps;
    Compass_Data_t compass;
} Sensor_Data_t;

/*============================================================================*/
/* API Functions                                                              */
/*============================================================================*/

/* GPS: Initialize UART + configure parser */
void     GPS_Init(void);

/* GPS: Feed byte into NMEA parser (call from UART RX ISR) */
void     GPS_FeedByte(uint8_t byte);

/* GPS: Process complete NMEA sentences (call periodically) */
void     GPS_Process(void);

/* GPS: Get parsed data */
uint8_t  GPS_GetData(GPS_Data_t *data);

/* GPS: Check if fix is valid */
uint8_t  GPS_IsValid(void);

/* Compass: Initialize HMC5883L over I2C */
uint8_t  Compass_Init(void);

/* Compass: Read raw axes + compute heading */
uint8_t  Compass_Read(Compass_Data_t *data);

/* Compass: Compute heading from raw X/Y */
float    Compass_ComputeHeading(int16_t x, int16_t y);

/* Combined: Initialize both sensors */
uint8_t  Sensors_Init(void);

/* Combined: Read all sensors */
void     Sensors_Update(Sensor_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __GPS_COMPASS_H */