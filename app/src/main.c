/**
 * @file main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1

int main(void) {
  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  int8_t oxygen1 = 100;
  int8_t oxygen2 = 100;
  int8_t oxygen3 = 100;
  int8_t oxygen4 = 100;
  uint8_t oxygenCooldown = 118;
  uint8_t numberOfLEDsOn = 4;
  uint8_t oxygenSubtract = 1;

  while (1) {
    k_msleep(SLEEP_MS);

    oxygenCooldown--;

    if (oxygenCooldown <= 0) {
      oxygenCooldown = 118;

      // Count how many LEDs are on
      numberOfLEDsOn = 0;

      // Decrease oxygen levels
      if (oxygen1 > 0) {
        oxygen1 -= oxygenSubtract;
        numberOfLEDsOn++;

        if (oxygen1 < 0) oxygen1 = 0;
      }
      if (oxygen2 > 0) {
        oxygen2 -= oxygenSubtract;
        numberOfLEDsOn++;

        if (oxygen2 < 0) oxygen2 = 0;
      }
      if (oxygen3 > 0) {
        oxygen3 -= oxygenSubtract;
        numberOfLEDsOn++;

        if (oxygen3 < 0) oxygen3 = 0;
      }
      else oxygen3 = 0;
      if (oxygen4 > 0) {
        oxygen4 -= oxygenSubtract;
        numberOfLEDsOn++;

        if (oxygen4 < 0) oxygen4 = 0;
      }

      // Speed up oxygen decrease based on number of LEDs on
      if (numberOfLEDsOn <= 1) {
        // GAME OVER
      }
      else if (numberOfLEDsOn == 2) {
        oxygenSubtract = 3;
      }
      else {
        oxygenSubtract = 1;
      }

      printk("Oxygen subract: %i\n", oxygen3);

      // Increase oxygen when buttons pressed
      if (BTN_is_pressed(BTN0) && oxygen1 < 100) {\
        if (oxygen1 <= 0) {
          numberOfLEDsOn++;
        }

        oxygen1 += 10;
      }
      if (BTN_is_pressed(BTN1) && oxygen2 < 100) {
        if (oxygen2 <= 0) {
          numberOfLEDsOn++;
        }

        oxygen2 += 10;
      }
      if (BTN_is_pressed(BTN2) && oxygen3 < 100) {
        if (oxygen3 <= 0) {
          numberOfLEDsOn++;
        }

        oxygen3 += 10;
      }
      if (BTN_is_pressed(BTN3) && oxygen4 < 100) {
        if (oxygen4 <= 0) {
          numberOfLEDsOn++;
        }

        oxygen4 += 10;
      }
    }

    // Update LED brightness based on oxygen levels
    LED_pwm(LED0, oxygen1);
    LED_pwm(LED1, oxygen2);
    LED_pwm(LED2, oxygen3);
    LED_pwm(LED3, oxygen4);
  }
  return 0;
}

// General Premise
// Quiz center (Kahoot Mini Pro XL 6th Edition (Taylor's series))
// Main menu (Scroller menu with quizes (option to edit quiz), button to add quiz)
// Can create own quiz 
// Kahoot style (four choices)
// 
