/**
  ******************************************************************************
  * @file    radar_can.h
  * @brief   LRR220PRO Radar CAN Protocol Definitions (500Kbps)
  *          Based on EN LRR220PRO Protocol Output Documentation
  ******************************************************************************
  */
#ifndef __RADAR_CAN_H
#define __RADAR_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*===================================================================*/
/*              RADAR CAN MESSAGE IDS (Standard 11-bit)              */
/*===================================================================*/
#define RADAR_ID_ESC_STS_RUN_1      0x141   /* ESC status run 1 */
#define RADAR_ID_ESC_STS_RUN_2      0x142   /* ESC status run 2 */
#define RADAR_ID_EPS_STS_RUN        0x221   /* EPS status run */
#define RADAR_ID_ABS_FAULT_INFO     0x310   /* ABS fault info */
#define RADAR_ID_ABS_STS_RUN1       0x330   /* ABS status run 1 */
#define RADAR_ID_ABS_STS_RUN2       0x210   /* ABS status run 2 */
#define RADAR_ID_FRS_STATUS         0x080   /* FRS radar status */
#define RADAR_ID_FRS_OBJ_BASE       0x200   /* FRS object base ID */

/*===================================================================*/
/*              RADAR INPUT MESSAGES (ECU sends to radar)            */
/*===================================================================*/

/* 0x141 - ESC_STS_RUN_1 (8 bytes) */
typedef struct {
    int16_t  lateral_accel;         /* [0:15] bit 8-23, factor 0.027127, offset 0, unit m/s2 */
    int16_t  longitudinal_accel;    /* [16:31] bit 24-39, factor 0.027127, offset 0, unit m/s2 */
    int16_t  yaw_rate;              /* [32:47] bit 40-55, factor 0.00021326, offset 0, unit rad/s */
    uint8_t  yaw_rate_valid : 1;    /* bit 48 */
    uint8_t  long_accel_valid : 1;  /* bit 49 */
    uint8_t  lat_accel_valid : 1;   /* bit 50 */
    uint8_t  reserved : 5;          /* bit 51-55 */
    uint8_t  checksum;              /* bit 56-63 */
} __attribute__((packed)) Radar_ESC_Sts_Run1_t;

/* 0x142 - ESC_STS_RUN_2 (8 bytes) */
typedef struct {
    uint8_t  whl_spd_fl_valid : 2;  /* bit 0-1 */
    uint8_t  whl_spd_fr_valid : 2;  /* bit 2-3 */
    uint8_t  whl_spd_rl_valid : 2;  /* bit 4-5 */
    uint8_t  whl_spd_rr_valid : 2;  /* bit 6-7 */
    uint8_t  reserved[7];           /* bit 8-63 */
} __attribute__((packed)) Radar_ESC_Sts_Run2_t;

/* 0x221 - EPS_STS_RUN (8 bytes) */
typedef struct {
    uint8_t  reserved1[1];          /* bit 0-7 */
    int16_t  steering_angle;        /* [8:23] bit 8-23, factor 0.1, offset 0, unit deg */
    uint8_t  steering_angle_spd;    /* [24:31] bit 24-31, factor 4, offset 0, unit deg/s */
    uint8_t  reserved2[2];          /* bit 32-47 */
    uint8_t  sensor_calibrated : 1; /* bit 30 */
    uint8_t  sensor_failure : 1;    /* bit 31 */
    uint8_t  reserved3[2];          /* bit 32-47 */
    uint8_t  checksum;              /* bit 56-63 */
} __attribute__((packed)) Radar_EPS_Sts_Run_t;

/* 0x310 - ABS_FAULT_INFO (8 bytes) */
typedef struct {
    uint16_t whl_spd_fl : 13;       /* bit 0-12, factor 0.05625, unit kmph */
    uint16_t whl_spd_fr : 13;       /* bit 13-25, factor 0.05625, unit kmph */
    uint16_t whl_spd_rl : 13;       /* bit 26-38, factor 0.05625, unit kmph */
    uint16_t whl_spd_rr : 13;       /* bit 39-51, factor 0.05625, unit kmph */
    uint8_t  reserved[4];           /* bit 52-63 */
} __attribute__((packed)) Radar_ABS_Fault_Info_t;

/* 0x330 - ABS_STS_RUN1 (8 bytes) */
typedef struct {
    uint8_t  reserved1[1];          /* bit 0-7 */
    uint16_t vehicle_spd : 13;      /* bit 8-20, factor 0.05625, unit kmph */
    uint8_t  spd_valid : 1;         /* bit 21 */
    uint8_t  reserved2[5];          /* bit 22-63 */
} __attribute__((packed)) Radar_ABS_Sts_Run1_t;

/* 0x210 - ABS_STS_RUN2 (8 bytes) */
typedef struct {
    uint8_t  reserved1[1];          /* bit 0-7 */
    uint8_t  gear : 2;              /* bit 8-9: 0=undefined, 1=R, 2=D, 3=N */
    uint8_t  reserved2[6];          /* bit 10-63 */
} __attribute__((packed)) Radar_ABS_Sts_Run2_t;

/*===================================================================*/
/*              RADAR OUTPUT MESSAGES (Radar sends to ECU)           */
/*===================================================================*/

/* 0x080 - FRS_STATUS (8 bytes) */
typedef struct {
    uint8_t  reserved1[1];          /* bit 0-7 */
    uint8_t  latency : 6;           /* bit 8-13, factor 2, unit ms */
    uint8_t  reserved2[1];          /* bit 14-15 */
    uint16_t timestamp;             /* bit 16-31, factor 1, unit s */
    uint16_t host_speed : 12;       /* bit 32-43, factor 0.025, offset -20, unit m/s */
    uint8_t  blocked : 1;           /* bit 44: 0=not blocked, 1=blocked */
    uint8_t  radar_fail : 1;        /* bit 45: 0=working, 1=not working */
    uint8_t  meas_enabled : 1;      /* bit 46 */
    int16_t  host_yaw : 11;         /* bit 47-57, factor 0.1, offset -102.4, unit deg/s */
    uint8_t  alive_counter : 4;     /* bit 48-51 */
    uint8_t  misalign : 3;          /* bit 52-54: calibration state */
    uint8_t  hw_err : 1;            /* bit 55: 0=HW ok, 1=HW failed */
    uint8_t  checksum;              /* bit 56-63 */
} __attribute__((packed)) Radar_FRS_Status_t;

/* FRS Object Part 1 (0x200 + obj_id) */
typedef struct {
    uint8_t  obj_id;                /* bit 0-7: 0xFF=invalid */
    uint8_t  xpos_stdev : 7;        /* bit 8-14, factor 0.1, unit m */
    uint8_t  update_flag : 1;       /* bit 15: 0=new, 1=existed */
    uint8_t  ypos_stdev : 7;        /* bit 16-22, factor 0.1, unit m */
    uint8_t  valid_flag : 1;        /* bit 23: 0=invalid, 1=valid */
    uint8_t  obstacle_prob : 5;     /* bit 24-28, factor 3.2258, unit % */
    uint8_t  motion_pattern : 3;    /* bit 29-31: 0=unknown, 2=stopped, 3=moving */
    int8_t   x_accel_rel : 7;       /* bit 33-39, factor 0.15, offset -9.6, unit m/s2 */
    uint8_t  x_vel_rel_stdev : 7;   /* bit 42-48, factor 0.05, unit m/s */
    uint8_t  msg_alive : 4;         /* bit 48-51 */
    uint8_t  exst_prob : 6;         /* bit 52-57, factor 1.5873, unit % */
    uint8_t  checksum;              /* bit 56-63 */
} __attribute__((packed)) Radar_FRS_Obj_Part1_t;

/* FRS Object Part 2 (0x200 + obj_id + offset) */
typedef struct {
    uint8_t  reserved1[1];          /* bit 0-7 */
    int16_t  x_vel_rel : 11;        /* bit 13-23, factor 0.1, offset -102.4, unit m/s */
    int16_t  y_pos : 13;            /* bit 16-28, factor 0.015625, offset -64, unit m */
    uint8_t  obj_type : 2;          /* bit 32-33: 0=unknown, 1=4wheeler */
    uint16_t x_pos : 14;            /* bit 34-47, factor 0.015625, unit m */
    uint8_t  obj_alive : 4;         /* bit 48-51 */
    uint8_t  meas_flag : 1;         /* bit 52: 0=measured, 1=extrapolated */
    int16_t  y_vel_rel : 11;        /* bit 53-63, factor 0.1, offset -102.4, unit m/s */
    uint8_t  checksum;              /* bit 56-63 */
} __attribute__((packed)) Radar_FRS_Obj_Part2_t;

/*===================================================================*/
/*              RADAR CAN API FUNCTIONS                              */
/*===================================================================*/

/* Parse radar input messages (ECU -> Radar) */
void    Radar_SendVehicleSpeed(uint16_t speed_kmph);
void    Radar_SendYawRate(int16_t yaw_rate_rads);
void    Radar_SendGear(uint8_t gear);
void    Radar_SendWheelSpeeds(uint16_t fl, uint16_t fr, uint16_t rl, uint16_t rr);
void    Radar_SendAcceleration(int16_t lat, int16_t lon);

/* Parse radar output messages (Radar -> ECU) */
int     Radar_GetStatus(Radar_FRS_Status_t* status);
int     Radar_GetObject(uint8_t obj_id, Radar_FRS_Obj_Part1_t* p1, Radar_FRS_Obj_Part2_t* p2);

/* CAN bit timing for 500Kbps */
#define CAN_BITRATE_500K         500000

#ifdef __cplusplus
}
#endif

#endif /* __RADAR_CAN_H */