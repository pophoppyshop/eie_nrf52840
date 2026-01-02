#include <zephyr/smf.h>

#include "string.h"
#include "BTN.h"
#include "LED.h"
#include "string.h"
#include <math.h>

#include "state_machine.h"

/*
    FUNCTION PROTOTYPES
*/
static void enter_code_entry(void* o);
static enum smf_state_result enter_code_run(void* o);

static void save_string_entry(void* o);
static enum smf_state_result save_string_run(void* o);

static void send_monitor_entry(void* o);
static enum smf_state_result send_monitor_run(void* o);

static void on_hold_entry(void* o);
static enum smf_state_result on_hold_run(void* o);

/*
    TYPEDEFS (enum)
*/
enum machine_states {
    ENTER_CODE,     // S0
    SAVE_STRING,    // S1
    SEND_MONITOR,   // S2
    ON_HOLD         // S3
};

// Contains variables and current state
typedef struct {
    struct smf_ctx ctx;

    uint8_t string_buffer[100]; // stores string input
    uint8_t current_code;       // stores current code input 
    int code_index;             // index for code input (0 to 7)
    int string_index;           // index for string input (0 to 99)
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
};

static state_machine_t machine;

void state_machine_init() {
    // Initialize variables
    machine.current_code = 0;
    machine.code_index = 0;
    machine.string_index = 0;
    machine.LED0_state = false, machine.LED1_state = false;
    memset(machine.string_buffer, 0, sizeof(machine.string_buffer));

    // Initial state
    smf_set_initial(SMF_CTX(&machine), &states[ENTER_CODE]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&machine));
}

/*
    Additional Functions
*/
void add_to_code(int bit){
    // Add code and increase index
    if (machine.code_index < 8) {
        machine.current_code += bit * pow(2, machine.code_index);

        machine.code_index++;

        // LED2 on once
        if (machine.code_index == 8)
            LED_set(LED2, true);
    }
}


/*
    STATE: ENTER_CODE
*/
static void enter_code_entry(void* o) {
    LED_blink(LED3, LED_1HZ);
}

static enum smf_state_result enter_code_run(void* o) {
    // Turn on LED0 when BTN0 pressed -----------------
    if (BTN_is_pressed(BTN0) && !machine.LED0_state) {
        LED_set(LED0, true);

        machine.LED0_state = true;

        add_to_code(0);
    }
    // Turn off LED0 when BTN0 released
    else if (!BTN_is_pressed(BTN0) && machine.LED0_state) {
        LED_set(LED0, false);

        machine.LED0_state = false;
    }
    
    // Turn on LED1 when BTN1 pressed -----------------
    if (BTN_is_pressed(BTN1) && !machine.LED1_state) {
        LED_set(LED1, true);

        machine.LED1_state = true;

        add_to_code(1); 
    }
    // Turn off LED1 when BTN1 released
    else if (!BTN_is_pressed(BTN1) && machine.LED1_state) {
        LED_set(LED1, false);

        machine.LED1_state = false;
    }

    // Clear code when BTN2 pressed ------------------
    if (BTN_check_clear_pressed(BTN2)) {
        machine.current_code = 0;
        machine.code_index = 0;

        // Turn off LED2
        if (machine.code_index == 8)
            LED_set(LED2, false);
    }

    // Save code when BTN3 pressed -------------------
    if (BTN_check_clear_pressed(BTN3) && machine.code_index == 8) {
        string_buffer[machine.string_index] = machine.current_code;

        machine.string_index++;

        // Turn off LED2
        LED_set(LED2, false);

        smf_set_state(SMF_CTX(&machine), &states[SAVE_STRING]);
    }

    return SMF_EVENT_HANDLED;
}


/*
    STATE: SAVE_STRING
*/
static void save_string_entry(void* o) {
    LED_blink(LED3, LED_4HZ);
}

static enum smf_state_result save_string_run(void* o) {
    return SMF_EVENT_HANDLED;
}


/*
    STATE: SEND_MONITOR
*/
static void send_monitor_entry(void* o) {
    LED_blink(LED3, LED_16HZ);
}

static enum smf_state_result send_monitor_run(void* o) {
    return SMF_EVENT_HANDLED;
}

/*
    STATE: ON_HOLD
*/
static void on_hold_entry(void* o) {

}

static enum smf_state_result on_hold_run(void* o) {
    return SMF_EVENT_HANDLED;
}