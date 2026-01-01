#include <zephyr/smf.h>

#include "string.h"
#include "BTN.h"
#include "LED.h"
#include "string.h"

#include "state_machine.h"

/*
    FUNCTION PROTOTYPES
*/
static void enter_code_entry(void* o);
static enum enter_code_run(void* o);

static void save_string_entry(void* o);
static enum save_string_run(void* o);

static void send_monitor_entry(void* o);
static enum send_monitor_run(void* o);

static void on_hold_entry(void* o);
static enum on_hold_run(void* o);

/*
    TYPEDEFS (enum)
*/
enum machine_states {
    ENTER_CODE,     // S0
    SAVE_STRING,    // S1
    SEND_MONITOR,   // S2
    ON_HOLD         // S3
}

// Contains variables and current state
typedef struct {
    struct smf_ctx ctx;

    uint8_t string_buffer[100]; // stores string input
    uint8_t current_code;       // stores current code input 
    int code_index;             // index for code input (0 to 7)
    bool LED0_state, LED1_state;
} state_machine_t;

/*
    Local Variables
*/
static const struct smf_state states[] = {
    [ENTER_CODE] = SMF_CREATE_STATE(enter_code_entry, enter_code_run, NULL, NULL, NULL),
    [SAVE_STRING] = SMF_CREATE_STATE(save_string_entry, save_string_run, NULL, NULL, NULL),
    [SEND_MONITOR] = SMF_CREATE_STATE(send_monitor_entry, send_monitor_run, NULL, NULL, NULL),
    [ON_HOLD] = SMF_CREATE_STATE(on_hold_entry, on_hold_run, NULL, NULL, NULL),
}

static state_machine_t machine;

void state_machine_init() {
    // Initialize variables
    machine.current_code = 0;
    machine.code_index = 0;
    LED0_state = false, LED1_state = false;
    memset(machine.string_buffer, 0, sizeof(machine.string_buffer));

    // Initial state
    smf_set_initial(SMF_CTX(&machine), &states[ENTER_CODE]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&machine));
}


/*
    STATE: ENTER_CODE
*/
static void enter_code_entry(void* o) {
    LED_blink(LED3, LED_1HZ);
}

static enum enter_code_run(void* o) {
    // Check btn0
    if (BTN_is_pressed(BTN0) && !machine.LED0_state) {
        LED_set(LED0, true);
        
        machine.LED0_state = true;
    }
    else if (!BTN_is_pressed(BTN0) && machine.LED0_state) {
        LED_set(LED0, false);

        machine.LED0_state = false;
    }
    
    // Check btn1
    if (BTN_is_pressed(BTN1) && !machine.LED1_state) {
        LED_set(LED1, true);
        
        machine.LED1_state = true;
    }
    else if (!BTN_is_pressed(BTN1) && machine.LED1_state) {
        LED_set(LED1, false);

        machine.LED1_state = false;
    }
}


/*
    STATE: SAVE_STRING
*/
static void save_string_entry(void* o) {
    LED_blink(LED3, LED_4HZ);
}


/*
    STATE: SEND_MONITOR
*/
static void send_monitor_entry(void* o) {
    LED_blink(LED3, LED_16HZ);
}