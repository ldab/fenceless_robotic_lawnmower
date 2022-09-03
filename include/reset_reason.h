/*
 *  Print last reset reason of ESP32
 *  =================================
 *
 *  Use either of the methods print_reset_reason
 *  or verbose_print_reset_reason to display the
 *  cause for the last reset of this device.
 *
 *  Public Domain License.
 *
 *  Author:
 *  Evandro Luis Copercini - 2017
 */

#include "Arduino.h"

#ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
#if CONFIG_IDF_TARGET_ESP32  // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/rtc.h"
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else // ESP32 Before IDF 4.0
#include "rom/rtc.h"
#endif

void print_reset_reason(int reason)
{
  switch (reason) {
  case 1:
    Serial.println("POWERON_RESET");
    break; /**<1,  Vbat power on reset*/
  case 3:
    Serial.println("SW_RESET");
    break; /**<3,  Software reset digital core*/
  case 4:
    Serial.println("OWDT_RESET");
    break; /**<4,  Legacy watch dog reset digital core*/
  case 5:
    Serial.println("DEEPSLEEP_RESET");
    break; /**<5,  Deep Sleep reset digital core*/
  case 6:
    Serial.println("SDIO_RESET");
    break; /**<6,  Reset by SLC module, reset digital core*/
  case 7:
    Serial.println("TG0WDT_SYS_RESET");
    break; /**<7,  Timer Group0 Watch dog reset digital core*/
  case 8:
    Serial.println("TG1WDT_SYS_RESET");
    break; /**<8,  Timer Group1 Watch dog reset digital core*/
  case 9:
    Serial.println("RTCWDT_SYS_RESET");
    break; /**<9,  RTC Watch dog Reset digital core*/
  case 10:
    Serial.println("INTRUSION_RESET");
    break; /**<10, Instrusion tested to reset CPU*/
  case 11:
    Serial.println("TGWDT_CPU_RESET");
    break; /**<11, Time Group reset CPU*/
  case 12:
    Serial.println("SW_CPU_RESET");
    break; /**<12, Software reset CPU*/
  case 13:
    Serial.println("RTCWDT_CPU_RESET");
    break; /**<13, RTC Watch dog Reset CPU*/
  case 14:
    Serial.println("EXT_CPU_RESET");
    break; /**<14, for APP CPU, reseted by PRO CPU*/
  case 15:
    Serial.println("RTCWDT_BROWN_OUT_RESET");
    break; /**<15, Reset when the vdd voltage is not stable*/
  case 16:
    Serial.println("RTCWDT_RTC_RESET");
    break; /**<16, RTC Watch dog reset digital core and rtc module*/
  default:
    Serial.println("NO_MEAN");
  }
}

void verbose_print_reset_reason(void)
{
  for (size_t i = 0; i <= 1; i++) {
    
    switch (rtc_get_reset_reason(i)) {
    case 1:
      log_i("Core%d - Vbat power on reset", i);
      break;
    case 3:
      log_i("Core%d - Software reset digital core", i);
      break;
    case 4:
      log_i("Core%d - Legacy watch dog reset digital core", i);
      break;
    case 5:
      log_i("Core%d - Deep Sleep reset digital core", i);
      break;
    case 6:
      log_i("Core%d - Reset by SLC module, reset digital core", i);
      break;
    case 7:
      log_i("Core%d - Timer Group0 Watch dog reset digital core", i);
      break;
    case 8:
      log_i("Core%d - Timer Group1 Watch dog reset digital core", i);
      break;
    case 9:
      log_i("Core%d - RTC Watch dog Reset digital core", i);
      break;
    case 10:
      log_i("Core%d - Instrusion tested to reset CPU", i);
      break;
    case 11:
      log_i("Core%d - Time Group reset CPU", i);
      break;
    case 12:
      log_i("Core%d - Software reset CPU", i);
      break;
    case 13:
      log_i("Core%d - RTC Watch dog Reset CPU", i);
      break;
    case 14:
      log_i("Core%d - for APP CPU, reseted by PRO CPU", i);
      break;
    case 15:
      log_i("Core%d - Reset when the vdd voltage is not stable", i);
      break;
    case 16:
      log_i("Core%d - RTC Watch dog reset digital core and rtc module", i);
      break;
    default:
      log_i("Core%d - NO_MEAN", i);
    }
  }
}
