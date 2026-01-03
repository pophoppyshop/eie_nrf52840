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

    // For ON_HOLD state
    uint8_t current_duty_cycle;
    enum machine_states previous_state;
    int hold_duration_direction;          // in miliseconds / +-1
    int counter;                          // miliseconds counter

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
    memset(machine.string_buffer, 0, sizeof(machine.string_buffer));
    machine.current_code = 0;
    machine.code_index = 7;
    machine.string_index = 0;
    machine.LED0_state = false, machine.LED1_state = false;
    
    machine.current_duty_cycle = 0;
    machine.previous_state = ENTER_CODE;
    machine.hold_duration_direction = 0;
    machine.counter = 0;

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
    if (machine.code_index >= 0) {
        machine.current_code += bit * pow(2, machine.code_index);

        machine.code_index--;

        // LED2 on once
        if (machine.code_index == -1)
            LED_set(LED2, true);
    }
}

void clear_string() {
    if (BTN_check_clear_pressed(BTN2)) {
        machine.string_index = 0;
        memset(machine.string_buffer, 0, sizeof(machine.string_buffer));

        smf_set_state(SMF_CTX(&machine), &states[ENTER_CODE]);
    }
}

void set_all_pwm(){
    LED_pwm(LED0, machine.current_duty_cycle);
    LED_pwm(LED1, machine.current_duty_cycle);
    LED_pwm(LED2, machine.current_duty_cycle);
    LED_pwm(LED3, machine.current_duty_cycle);
}

void add_hold_duration(enum machine_states state) {
    if (BTN_is_pressed(BTN0) && BTN_is_pressed(BTN1)) {
        machine.hold_duration_direction++;
    }
    else if (machine.hold_duration_direction != 0){
        // Reset duration counter
        machine.hold_duration_direction = 0;
    }

    // Check if held for 3s and switch to ON_HOLD
    if (machine.hold_duration_direction >= 3000) {
        machine.previous_state = state;

        smf_set_state(SMF_CTX(&machine), &states[ON_HOLD]);
    }
}


/*
    STATE: ENTER_CODE
*/
static void enter_code_entry(void* o) {
    LED_blink(LED3, LED_1HZ);

    // LED2 on once
    if (machine.code_index == -1)
        LED_set(LED2, true);
}

static enum smf_state_result enter_code_run(void* o) {
    add_hold_duration(ENTER_CODE);

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
        // Turn off LED2
        if (machine.code_index == -1)
            LED_set(LED2, false);

        machine.current_code = 0;
        machine.code_index = 7;
    }

    // Save code when BTN3 pressed and 8 bits entered -------------------
    if (BTN_check_clear_pressed(BTN3) && machine.code_index == -1) {
        machine.string_buffer[machine.string_index] = machine.current_code;

        machine.string_index++;

        // Reset code variables
        machine.current_code = 0;
        machine.code_index = 7;

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
    add_hold_duration(SAVE_STRING);

    // Turn on LED0 when BTN0 pressed -----------------
    if (BTN_is_pressed(BTN0) && !machine.LED0_state) {
        LED_set(LED0, true);

        machine.LED0_state = true;

        add_to_code(0);
        
        // Go back to enter code state
        smf_set_state(SMF_CTX(&machine), &states[ENTER_CODE]);
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

        // Go back to enter code state
        smf_set_state(SMF_CTX(&machine), &states[ENTER_CODE]);
    }
    // Turn off LED1 when BTN1 released
    else if (!BTN_is_pressed(BTN1) && machine.LED1_state) {
        LED_set(LED1, false);

        machine.LED1_state = false;
    }

    // Reset string when BTN2 pressed ------------------
    clear_string();

    // Save string when BTN3 pressed -------------------
    if (BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&machine), &states[SEND_MONITOR]);
    }

    return SMF_EVENT_HANDLED;
}


/*
    STATE: SEND_MONITOR
*/
static void send_monitor_entry(void* o) {
    LED_blink(LED3, LED_16HZ);
}

static enum smf_state_result send_monitor_run(void* o) {
    add_hold_duration(SEND_MONITOR);

    // Turn on LED0 when BTN0 pressed -----------------
    if (BTN_is_pressed(BTN0) && !machine.LED0_state) {
        LED_set(LED0, true);

        machine.LED0_state = true;
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
    }
    // Turn off LED1 when BTN1 released
    else if (!BTN_is_pressed(BTN1) && machine.LED1_state) {
        LED_set(LED1, false);

        machine.LED1_state = false;
    }

    // Clear string when BTN2 pressed ------------------
    clear_string();

    // Send to serial monitor when BTN3 pressed -------------------
    if (BTN_check_clear_pressed(BTN3)) {
        printk("String: ");

        // Use char pointer to print
        char* char_ptr = (char*) machine.string_buffer;

        printk("%s", char_ptr);

        printk("\n");
    }

    return SMF_EVENT_HANDLED;
}

/*
    STATE: ON_HOLD
*/
static void on_hold_entry(void* o) {
    set_all_pwm();

    machine.counter = 1015;
}

static enum smf_state_result on_hold_run(void* o) {
    // Main loop start after a short delay 
    if (machine.counter <= 15) {
        // Change direction
        if (machine.current_duty_cycle >= 100) {
            machine.hold_duration_direction = -1;
        }
        else if (machine.current_duty_cycle <= 0) {
            machine.hold_duration_direction = 1;
        }

        // Slows down the pulsing
        if (machine.counter >= 15) {
            machine.current_duty_cycle += machine.hold_duration_direction;
            machine.counter = 0;
        }
        
        machine.counter++;
        set_all_pwm();

        // Switch to previous state when any button pressed
        if (BTN_is_pressed(BTN0) ||
            BTN_is_pressed(BTN1) || 
            BTN_is_pressed(BTN2) || 
            BTN_is_pressed(BTN3)) { 
                machine.current_duty_cycle = 0;
                machine.counter = 0;
                
                set_all_pwm();

                machine.LED0_state = false, machine.LED1_state = false;

                smf_set_state(SMF_CTX(&machine), &states[machine.previous_state]);
        }
    }
    else {
        machine.counter--;
    }

    return SMF_EVENT_HANDLED;
}