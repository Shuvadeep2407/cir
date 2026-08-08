/**
  ******************************************************************************
  * @file    gps_compass.c
  * @brief   Combined NEO-6M GPS + GY-271 HMC5883L Compass Sensor Library
  *
  *          NEO-6M GPS:     UART @ 9600, NMEA 0183 (GPGGA/GPRMC)
  *          HMC5883L Compass: I2C @ 0x1E, 3-axis magnetometer, heading
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gps_compass.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*============================================================================*/
/* Private Variables                                                          */
/*============================================================================*/

/* GPS NMEA parser state */
static char nmea_buffer[GPS_BUFFER_SIZE];
static uint16_t nmea_index = 0;
static uint8_t nmea_in_sentence = 0;
static uint8_t nmea_sentence_ready = 0;

static GPS_Data_t gps_data;

/* Compass gain */
static float compass_scale = COMPASS_GAIN_SCALE;

/*============================================================================*/
/* NEO-6M GPS: NMEA Parsing                                                   */
/*============================================================================*/

/**
  * @brief  Parse a NMEA coordinate string (DDMM.MMMM) to decimal degrees
  */
static float GPS_ParseCoord(const char *nmea_str)
{
    double degrees;
    double minutes;
    char temp[16];
    const char *dot;

    dot = strchr(nmea_str, '.');
    if (dot == NULL)
    {
        return 0.0f;
    }

    {
        uint8_t deg_len = (uint8_t)(dot - nmea_str) - 2;
        uint8_t i;

        if (deg_len > 2)
        {
            deg_len = 2;
        }

        for (i = 0; i < deg_len; i++)
        {
            temp[i] = nmea_str[i];
        }
        temp[deg_len] = '\0';
        degrees = atof(temp);
    }

    strncpy(temp, nmea_str + (dot - nmea_str) - 2, 10);
    temp[10] = '\0';
    minutes = atof(temp);

    return (float)(degrees + (minutes / 60.0));
}

/**
  * @brief  Parse GPGGA sentence
  */
static void GPS_ParseGGA(char *sentence)
{
    char *token;
    char *save_ptr;

    /* $GPGGA,hhmmss.ss,lat,N,lon,E,fix,sats,hdop,alt,M,geoid,M */
    strtok_r(sentence, ",", &save_ptr);
    token = strtok_r(NULL, ",", &save_ptr);  /* UTC time */
    if (token != NULL && strlen(token) >= 6)
    {
        gps_data.utc_hour = (uint8_t)((token[0] - '0') * 10 + (token[1] - '0'));
        gps_data.utc_minute = (uint8_t)((token[2] - '0') * 10 + (token[3] - '0'));
        gps_data.utc_second = (uint8_t)((token[4] - '0') * 10 + (token[5] - '0'));
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* Latitude */
    if (token != NULL && strlen(token) > 0)
    {
        gps_data.latitude = GPS_ParseCoord(token);
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* N/S */
    if (token != NULL && token[0] == 'S')
    {
        gps_data.latitude = -gps_data.latitude;
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* Longitude */
    if (token != NULL && strlen(token) > 0)
    {
        gps_data.longitude = GPS_ParseCoord(token);
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* E/W */
    if (token != NULL && token[0] == 'W')
    {
        gps_data.longitude = -gps_data.longitude;
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* Fix quality */
    if (token != NULL)
    {
        gps_data.fix_quality = (uint8_t)atoi(token);
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* Satellites */
    if (token != NULL)
    {
        gps_data.satellites = (uint8_t)atoi(token);
    }

    strtok_r(NULL, ",", &save_ptr);          /* HDOP */

    token = strtok_r(NULL, ",", &save_ptr);  /* Altitude */
    if (token != NULL)
    {
        gps_data.altitude_m = atof(token);
    }

    gps_data.valid = (gps_data.fix_quality > 0) ? 1 : 0;
    gps_data.updated = 1;
}

/**
  * @brief  Parse GPRMC sentence
  */
static void GPS_ParseRMC(char *sentence)
{
    char *token;
    char *save_ptr;

    /* $GPRMC,hhmmss.ss,A,lat,N,lon,E,speed,course,date,mag,var */
    strtok_r(sentence, ",", &save_ptr);
    token = strtok_r(NULL, ",", &save_ptr);  /* UTC time */
    if (token != NULL && strlen(token) >= 6)
    {
        gps_data.utc_hour = (uint8_t)((token[0] - '0') * 10 + (token[1] - '0'));
        gps_data.utc_minute = (uint8_t)((token[2] - '0') * 10 + (token[3] - '0'));
        gps_data.utc_second = (uint8_t)((token[4] - '0') * 10 + (token[5] - '0'));
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* Status A=valid */
    gps_data.valid = (token != NULL && token[0] == 'A') ? 1 : 0;

    token = strtok_r(NULL, ",", &save_ptr);  /* Latitude */
    if (token != NULL && strlen(token) > 0)
    {
        gps_data.latitude = GPS_ParseCoord(token);
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* N/S */
    if (token != NULL && token[0] == 'S')
    {
        gps_data.latitude = -gps_data.latitude;
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* Longitude */
    if (token != NULL && strlen(token) > 0)
    {
        gps_data.longitude = GPS_ParseCoord(token);
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* E/W */
    if (token != NULL && token[0] == 'W')
    {
        gps_data.longitude = -gps_data.longitude;
    }

    token = strtok_r(NULL, ",", &save_ptr);  /* Speed (knots) */
    if (token != NULL)
    {
        gps_data.speed_kmph = atof(token) * 1.852f;
    }

    gps_data.updated = 1;
}

/*============================================================================*/
/* NEO-6M GPS: Public API                                                     */
/*============================================================================*/

/**
  * @brief  Initialize GPS parser
  */
void GPS_Init(void)
{
    memset(&gps_data, 0, sizeof(GPS_Data_t));
    memset(nmea_buffer, 0, sizeof(nmea_buffer));
    nmea_index = 0;
    nmea_in_sentence = 0;
    nmea_sentence_ready = 0;
}

/**
  * @brief  Feed byte into NMEA parser (call from UART RX ISR)
  */
void GPS_FeedByte(uint8_t byte)
{
    if (byte == '$')
    {
        nmea_index = 0;
        nmea_in_sentence = 1;
        nmea_sentence_ready = 0;
        nmea_buffer[nmea_index++] = (char)byte;
    }
    else if (nmea_in_sentence)
    {
        if (byte == '\r' || byte == '\n')
        {
            nmea_buffer[nmea_index] = '\0';
            nmea_in_sentence = 0;
            nmea_sentence_ready = 1;
        }
        else if (nmea_index < (GPS_BUFFER_SIZE - 1))
        {
            nmea_buffer[nmea_index++] = (char)byte;
        }
        else
        {
            nmea_index = 0;
            nmea_in_sentence = 0;
        }
    }
}

/**
  * @brief  Process complete NMEA sentence (call periodically)
  */
void GPS_Process(void)
{
    if (nmea_sentence_ready)
    {
        nmea_sentence_ready = 0;

        /* Strip checksum */
        {
            char *star = strchr(nmea_buffer, '*');
            if (star != NULL)
            {
                *star = '\0';
            }
        }

        if (strncmp(nmea_buffer, "$GPGGA", 6) == 0)
        {
            GPS_ParseGGA(nmea_buffer);
        }
        else if (strncmp(nmea_buffer, "$GPRMC", 6) == 0)
        {
            GPS_ParseRMC(nmea_buffer);
        }
    }
}

/**
  * @brief  Get parsed GPS data
  * @retval 1 if valid fix, 0 otherwise
  */
uint8_t GPS_GetData(GPS_Data_t *data)
{
    if (data == NULL)
    {
        return 0;
    }

    memcpy(data, &gps_data, sizeof(GPS_Data_t));
    gps_data.updated = 0;

    return data->valid;
}

/**
  * @brief  Check if GPS fix is valid
  */
uint8_t GPS_IsValid(void)
{
    return gps_data.valid;
}

/*============================================================================*/
/* HMC5883L Compass: Private Helpers                                          */
/*============================================================================*/

static uint8_t Compass_WriteReg(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c1, (COMPASS_I2C_ADDR << 1),
                             reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
}

static uint8_t Compass_ReadReg(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c1, (COMPASS_I2C_ADDR << 1),
                            reg, I2C_MEMADD_SIZE_8BIT, value, 1, 100);
}

static uint8_t Compass_ReadRegs(uint8_t reg, uint8_t *buffer, uint8_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, (COMPASS_I2C_ADDR << 1),
                            reg, I2C_MEMADD_SIZE_8BIT, buffer, len, 100);
}

/*============================================================================*/
/* HMC5883L Compass: Public API                                               */
/*============================================================================*/

/**
  * @brief  Initialize HMC5883L compass
  * @retval 1 on success, 0 on failure
  */
uint8_t Compass_Init(void)
{
    uint8_t id_a;

    /* Verify sensor identity */
    Compass_ReadReg(COMPASS_REG_ID_A, &id_a);
    if (id_a != 'H')
    {
        return 0;
    }

    /* Configure: 8 samples avg, 15Hz, normal mode */
    Compass_WriteReg(COMPASS_REG_CONFIG_A, COMPASS_CFG_A_8AVG);

    /* Configure gain: ±1.3 Ga */
    Compass_WriteReg(COMPASS_REG_CONFIG_B, COMPASS_CFG_B_GAIN_1_3);

    /* Continuous measurement mode */
    Compass_WriteReg(COMPASS_REG_MODE, COMPASS_MODE_CONTINUOUS);

    return 1;
}

/**
  * @brief  Read raw axes + compute heading
  * @retval 1 on success, 0 on failure
  */
uint8_t Compass_Read(Compass_Data_t *data)
{
    uint8_t buffer[6];

    if (data == NULL)
    {
        return 0;
    }

    if (Compass_ReadRegs(COMPASS_REG_DATA_X_MSB, buffer, 6) != HAL_OK)
    {
        return 0;
    }

    /* Register order: X MSB, X LSB, Z MSB, Z LSB, Y MSB, Y LSB */
    data->x_raw = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->z_raw = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->y_raw = (int16_t)((buffer[4] << 8) | buffer[5]);

    data->heading_deg = Compass_ComputeHeading(data->x_raw, data->y_raw);

    Compass_ReadReg(COMPASS_REG_STATUS, &buffer[0]);
    data->data_ready = (buffer[0] & 0x01) ? 1 : 0;
    data->valid = 1;

    return 1;
}

/**
  * @brief  Compute heading from raw X/Y values
  * @retval Heading in degrees (0-360)
  */
float Compass_ComputeHeading(int16_t x, int16_t y)
{
    float heading;

    heading = atan2f((float)y, (float)x) * 180.0f / 3.14159265f;

    if (heading < 0)
    {
        heading += 360.0f;
    }

    heading += COMPASS_DECLINATION;

    if (heading >= 360.0f)
    {
        heading -= 360.0f;
    }
    else if (heading < 0)
    {
        heading += 360.0f;
    }

    return heading;
}

/*============================================================================*/
/* Combined Sensor API                                                        */
/*============================================================================*/

/**
  * @brief  Initialize both GPS and Compass sensors
  * @retval 1 if at least compass initialized, 0 if both failed
  */
uint8_t result = 0;
uint8_t Sensors_Init(void)
{


    GPS_Init();
    result |= Compass_Init();

    return result;
}

/**
  * @brief  Update all sensor data
  */
void Sensors_Update(Sensor_Data_t *data)
{
    if (data == NULL)
    {
        return;
    }

    /* Process GPS NMEA */
    GPS_Process();
    GPS_GetData(&data->gps);

    /* Read compass */
    Compass_Read(&data->compass);
}
