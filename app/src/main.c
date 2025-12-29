/*
 * main.c
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/printk.h>

#include <inttypes.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1

// Defines two UUIDS (service, characteristic)
#define s BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

// Advertising packets
static const struct bt_data ble_advertising_data[] = {
  BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NOBREDR) ),
  BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME)),
}

// Characteristic data buffer
static uint8_t ble_custom_characteristic_user_data[20] = {};

// Read callback (send current stored value in characteristic)
static ssize_t ble_custom_characteristic_read_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    void* buf, uint16_t len, uint16_t offset) {
      const char* value = attr->user_data;
      return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
}

// Write callback (save to characteristic data buffer)
static ssize_t ble_custom_characteristic_write_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    const void* buf, uint16_t len, uint16_t offset,
    uint8_t flags) {
      uint8_t* value_ptr = attr->user_data;

      if (offset + len > BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
      }

      memcpy(value_ptr + offset, buf, len);
      value_ptr[offset + len] = 0;

      return len;
    }

// Service & characteristic description
static const struct bt_uuid_128 ble_custom_service_uuid = BT_UUID_INIT_128(s);
static const struct bt_uuid_128 ble_custrom_characteristic_uuid = BT_UUID_INIT_128(s);

int main(void) {

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  while(1) {
    k_msleep(SLEEP_MS);
  }
	return 0;
}
