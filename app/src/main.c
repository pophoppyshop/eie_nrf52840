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

static const struct device* display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t* screen = NULL;

static void event_handler(lv_event_t* e) {
  // Handle events here
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    // Handle click event
    printk("Clicked!\n");
  }
  else if (code == LV_EVENT_VALUE_CHANGED) {
    // Handle press event
    printk("Toggled!\n");
  }
}

int main(void) {
  if (!device_is_ready(display_dev)) {
    printk("Display device not found\n");
    return 0;
  }

  screen = lv_screen_active();

  if (screen == NULL) {
    printk("Failed to get active screen\n");
    return 0;
  }

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  /* TEST IMAGE BUTTON */
  LV_IMG_DECLATE()
  lv_obj_t * label;

  lv_obj_t* btn = lv_imgbtn_create(screen);
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn, LV_SIZE_CONTENT);

  label = lv_label_create(btn);
  lv_label_set_text(label, "Click me!");
  lv_obj_center(label);

  display_blanking_off(display_dev);

  /* OXYGEN */
  int8_t oxygens[] = {100, 100, 100, 100};
  uint8_t oxygenCooldown = 118;
  uint8_t numberOfLEDsOn = 4;
  uint8_t oxygenSubtract = 1;
  enum btn_id_t buttons[] = {BTN0, BTN1, BTN2, BTN3};

  while (1) {
    k_msleep(SLEEP_MS);

    lv_timer_handler();

    oxygenCooldown--;

    if (oxygenCooldown <= 0) {
      oxygenCooldown = 118;

      // Count how many LEDs are on
      numberOfLEDsOn = 0;

      // Decrease oxygen levels
      for (int i = 0; i < 4; i++) {
        if (oxygens[i] > 0) {
          oxygens[i] -= oxygenSubtract;
          numberOfLEDsOn++;
        }
        if (oxygens[i] < 0) oxygens[i] = 0;
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

      // Increase oxygen when buttons pressed
      for (int i = 0; i < 4; i++) {
        enum btn_id_t button = buttons[i];

        if (BTN_is_pressed(button) && oxygens[i] < 100) {
          if (oxygens[i] <= 0) {
            numberOfLEDsOn++;
          }

          oxygens[i] += 10;
        }
      }
    }

    // Update LED brightness based on oxygen levels
    LED_pwm(LED0, oxygens[0]);
    LED_pwm(LED1, oxygens[1]);
    LED_pwm(LED2, oxygens[2]);
    LED_pwm(LED3, oxygens[3]);

  }
  return 0;
}

/*
void updateOxygen(uint8_t* oxygenCooldown, uint8_t* numberOfLEDsOn, uint8_t* oxygenSubtract) {

}
*/

// General Premise
// Quiz center (Kahoot Mini Pro XL 6th Edition (Taylor's series))
// Main menu (Scroller menu with quizes (option to edit quiz), button to add quiz)
// Can create own quiz 
// Kahoot style (four choices)
// 
