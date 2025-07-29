#ifndef VEHICLE_DATA_H
#define VEHICLE_DATA_H

#include <stdint.h>

// 定義後端控制指令的結構體
// 使用 __attribute__((packed)) 確保結構體成員緊密排列，沒有填充位元組
typedef struct __attribute__((packed)) {
  uint8_t command_byte;     // 命令位元組 (c)
  int8_t motor_speed;      // 馬達速度 (m)
  int16_t direction_angle;  // 方向角度 (d)
  int16_t servo_angle;      // 舵機角度 (a)
} CommandData_t;

// 定義 UNO 感測器數據的結構體
typedef struct __attribute__((packed)) {
  uint8_t status_byte;      // 狀態位元組 (s)
  uint16_t voltage_mv;      // 電壓 (v)，單位毫伏
  int16_t ultrasonic_distance_cm; // 超音波距離 (u)，單位厘米，-1 表示無效
  // 熱成像數據的特徵值
  int16_t thermal_max_temp; // 最高溫度 * 100
  int16_t thermal_min_temp; // 最低溫度 * 100
  uint8_t thermal_hotspot_x; // 最熱點的 X 座標 (0-7)
  uint8_t thermal_hotspot_y; // 最熱點的 Y 座標 (0-7)
} SensorData_t;

#endif // VEHICLE_DATA_H
