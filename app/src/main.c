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

#define SLEEP_MS 1000

// Defines UUIDS (service, characteristic)
#define BLE_CUSTOM_SERVICE_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)
#define BLE_CUSTOM_CHARACTERISTIC_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1)

#define BLE_SERVICE_2_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2)
#define BLE_CHARACTERISTIC_2_UUID BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef3)

#define BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH 20

// Advertising packets
static const struct bt_data ble_advertising_data[] = {
  BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR) ),
  BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME)),
};

// Characteristic data buffer
static uint8_t ble_custom_characteristic_user_data[BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH + 1] =
    {'E', 'i', 'E'};;

// Service & characteristic description
static const struct bt_uuid_128 ble_custom_service_uuid = BT_UUID_INIT_128(BLE_CUSTOM_SERVICE_UUID);
static const struct bt_uuid_128 ble_custom_characteristic_uuid = BT_UUID_INIT_128(BLE_CUSTOM_CHARACTERISTIC_UUID);

static const struct bt_uuid_128 ble_service_2_uuid = BT_UUID_INIT_128(BLE_SERVICE_2_UUID);
static const struct bt_uuid_128 ble_characteristic_2_uuid = BT_UUID_INIT_128(BLE_CHARACTERISTIC_2_UUID);


// FUNCTIONS

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

BT_GATT_SERVICE_DEFINE(
  ble_custom_service, // Name of struct with config
  BT_GATT_PRIMARY_SERVICE(&ble_custom_service_uuid), // setting the service UUID

  // Separate service for notify subscription
  BT_GATT_CHARACTERISTIC(
    &ble_custom_characteristic_uuid.uuid,  // Setting the characteristic UUID
    BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,  // Possible operations
    BT_GATT_PERM_READ,  // Permissions that connecting devices have
    ble_custom_characteristic_read_cb,     // Callback for when this characteristic is read from
    NULL,
    ble_custom_characteristic_user_data    // Initial data stored in this characteristic
  ),
  
  BT_GATT_CCC(  // Client characteristic configuration for the above custom characteristic
        NULL,     // Callback for when this characteristic is changed
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE  // Permissions that connecting devices have
        ),
);

BT_GATT_SERVICE_DEFINE(
  ble_write_service,
  BT_GATT_PRIMARY_SERVICE(&ble_service_2_uuid),

  BT_GATT_CHARACTERISTIC(
      &ble_characteristic_2_uuid.uuid,  // Setting the characteristic UUID
      BT_GATT_CHRC_WRITE,  // Possible operations
      BT_GATT_PERM_WRITE,  // Permissions that connecting devices have
      NULL,     // Callback for when this characteristic is read from
      ble_custom_characteristic_write_cb,    // Callback for when this characteristic is written to
      ble_custom_characteristic_user_data    // Initial data stored in this characteristic
    ),
);

static void ble_custom_service_notify(){
  static uint32_t counter = 0;
  bt_gatt_notify(NULL, &ble_custom_service.attrs[2], &counter, sizeof(counter));
  counter++;
}


// MAIN
int main(void) {
  int result = bt_enable(NULL);

  if (result) {
    printk("Bluetooth init failed (err %d)\n", result);
    return 0;
  } else {
    printk("Bluetooth initialized!\n");
  }

  result = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ble_advertising_data, ARRAY_SIZE(ble_advertising_data), 
    NULL, 0);

  if (result) {
    printk("Bluetooth init failed (err %d)\n", result);
    return 0;
  } else {
    printk("Advertising initialized!\n");
  }
  
  while(1) {
    k_msleep(SLEEP_MS);

    ble_custom_service_notify();
  }
	return 0;
}
