/**
 * @file main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/random/random.h>

#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/settings/settings.h>

#include <lvgl.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1

// BLE UUIDs
#define BLE_SERVICE_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)
#define BLE_CHARACTERISTIC_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1)

#define BLE_CHARACTERISTIC_VALUE_SIZE 20

// Advertising packets
static const struct bt_data ble_advertising_data[] = {
  BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
  BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME)),
};

// Characteristic data buffer (initial value)
static uint8_t ble_characteristic_value[BLE_CHARACTERISTIC_VALUE_SIZE + 1]= {"S", "t", "a", "r", "t", "!"}; // Initial value of characteristic

// Service & characteristic description
static const struct bt_uuid_128 ble_service_uuid = BT_UUID_INIT_128(BLE_SERVICE_UUID);
static const struct bt_uuid_128 ble_characteristic_uuid = BT_UUID_INIT_128(BLE_CHARACTERISTIC_UUID);

static const struct device* display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t* screen = NULL;

/* POTHOLE */
static int16_t potholeHealth = 500;
static uint8_t potholeCooldown = 200;

/* OXYGEN */
static int8_t oxygens[] = {100, 100, 100, 100};
static uint8_t oxygenCooldown = 118;
static uint8_t numberOfLEDsOn = 4;
static uint8_t oxygenSubtract = 1;
static enum btn_id_t buttons[] = {BTN0, BTN1, BTN2, BTN3};

static void updatePothole();
static void updateOxygen();

static void pothole_Event(lv_event_t* e);

/* BLE FUNCTIONS */
static ssize_t ble_characteristic_read_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    void* buf, uint16_t len, uint16_t offset);

static ssize_t ble_characteristic_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    const void* buf, uint16_t len, uint16_t offset, uint8_t flags);

static void ble_service_notify();

BT_GATT_SERVICE_DEFINE(
  ble_service, // Name of struct with config
  BT_GATT_PRIMARY_SERVICE(&ble_service_uuid), // setting the service UUID

  // Separate service for notify subscription
  BT_GATT_CHARACTERISTIC(
    &ble_characteristic_uuid.uuid,  // Setting the characteristic UUID
    BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,  // Possible operations
    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,  // Permissions that connecting devices have
    ble_characteristic_read_cb,     // Callback for when this characteristic is read from
    ble_characteristic_write_cb,    // Callback for when this characteristic is written to
    ble_characteristic_value    // Initial data stored in this characteristic
  ),

  BT_GATT_CCC(  // Client characteristic configuration for the above custom characteristic
    NULL,     // Callback for when this characteristic is changed
    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE  // Permissions that connecting devices have
  ),
);

int main(void) {
  int result = bt_enable(NULL);

  if (result) {
    printk("Bluetooth init failed (err %d)\n", result);
    return 0;
  }

  result = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ble_advertising_data, ARRAY_SIZE(ble_advertising_data), 
    NULL, 0);

  if (result) {
    printk("Bluetooth init failed (err %d)\n", result);
    return 0;
  }

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

  /* POTHOLE WINDOW */
  LV_IMAGE_DECLARE(Alien_thing);

  lv_obj_t* btn = lv_imgbtn_create(screen);
  lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, &Alien_thing, NULL);   // Set image
  lv_obj_add_event_cb(btn, pothole_Event, LV_EVENT_ALL, NULL);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);                               

  lv_obj_t * label = lv_label_create(btn);
  lv_obj_center(label);
  lv_label_set_text(label, "");     // Remove text from button

  display_blanking_off(display_dev);

  while (1) {
    k_msleep(SLEEP_MS);

    lv_timer_handler();

    updatePothole();
    updateOxygen();

    potholeCooldown--;

    ble_service_notify();
  }
  return 0;
}

/* SUBTRACT POTHOLE HEALTH */
static void updatePothole() {
  if (potholeCooldown > 0) {
    return; // Skip if cooldown is active
  }

  printk("Pothole Health: %i\n", potholeHealth);
  
  potholeCooldown = 200; // Reset cooldown

  uint8_t a;
  
  if (potholeHealth <= 310) {
    a = sys_rand8_get() % 10;   // Get a random number between 0 and 9
  }
  else {
    a = sys_rand8_get() % 5;   // Get a random number between 0 and 4
  }

  if (potholeHealth > 0) {
    potholeHealth -= a;
  }
  if (potholeHealth < 0) {
    potholeHealth = 0;

    // GAME OVER
  }
}

/* SUBTRACT OXYGEN */
static void updateOxygen() {
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

/* POTHOLE EVENT HANDLER */
static void pothole_Event(lv_event_t* e) {
  // Handle events here
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED && potholeHealth < 500) {
    // Handle click event
    potholeHealth += 25; // Increase health when clicked

    if (potholeHealth > 500) {
      potholeHealth = 500; // Cap health at 500
    }
  }
}

// Read callback (send current stored value in characteristic)
static ssize_t ble_characteristic_read_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    void* buf, uint16_t len, uint16_t offset) {
      const char* value = attr->user_data;

      return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}

// Write callback (save to characteristic data buffer)
static ssize_t ble_characteristic_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    const void* buf, uint16_t len, uint16_t offset,
    uint8_t flags) {

  uint8_t* value_ptr = attr->user_data;

  // Check for valid offset and length
  if (offset + len > BLE_CHARACTERISTIC_VALUE_SIZE) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  memcpy(value_ptr + offset, buf, len);
  value_ptr[offset + len] = 0;

  return len;
}

static void ble_service_notify() {
  bt_gatt_notify(NULL, &ble_service.attrs[2], ble_characteristic_value, strlen(ble_characteristic_value));
}