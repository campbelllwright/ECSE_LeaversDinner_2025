#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include "hardware/adc.h"
#include "matrix_display.hpp"
#include "pindefs.hpp"
#include "pico_flash.hpp"
#include "clw_dbgutils.h"

#define STR_BUFFER_LEN 128
// Number of temperature samples of the "baseline" room temperature to take on startup
#define BASELINE_SAMPLES 128
// Offset to add to baseline to account for self-heating after turning on the card
#define BASELINE_OFFSET -0.1f
// Number of samples to average for temperature reading
#define AVERAGE_WINDOW 32
// Minimum and maximum brightness levels
#define MAX_BRIGHTNESS 0.95f
#define MIN_BRIGHTNESS 0.05f

#define DEBUG_PRINT 0

void init_gpio(void){
    gpio_init_mask(MASK_ALL_COLS|MASK_ALL_ROWS);
    gpio_set_dir_masked(MASK_ALL_COLS|MASK_ALL_ROWS, MASK_ALL_COLS|MASK_ALL_ROWS);
    gpio_put_masked(MASK_ALL_ROWS,MASK_ALL_ROWS);
    gpio_set_drive_strength(LED_C1, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(LED_C2, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(LED_C3, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(LED_C4, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(LED_C5, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(PB1,0);
    gpio_set_dir(PB2,0);
    gpio_pull_up(PB1);
    gpio_pull_up(PB2);
    
    adc_init();
}

uint counter = 0;
uint scroll_count = 0;
const uint8_t * current_char;

float current_brightness = 0.55f;

// Swipe detection thresholds
#define TOUCH_THRESHOLD 10      // How much below baseline counts as a touch
#define SWIPE_TIMEOUT_MS 300    // Max time for a complete swipe
#define BRIGHTNESS_STEP 0.1f   // How much to change brightness per swipe

// Swipe animation state
struct SwipeAnimation {
    bool active;
    bool direction_up;
    int8_t position;    // -3 to 7 (starts off screen, moves across)
    uint32_t last_update;
} swipe_anim = {false, false, 0, 0};

#define SWIPE_ANIM_DURATION_MS 400
#define SWIPE_ANIM_STEPS 10

// Function to detect swipes and update brightness
void update_brightness_from_swipe(void) {
    static uint32_t last_update = 0;
    static uint32_t last_debug_print = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Update every 20ms
    if (now - last_update < 20) {
        return;
    }
    last_update = now;
    
    // Read all ADC channels (0-3 are touch sensors)
    uint16_t adc_readings[5];
    for (uint8_t i = 0; i < 5; i++) {
        adc_select_input(i);
        adc_readings[i] = adc_read();
    }
    
    // Calibrate baselines dynamically (slow moving average)
    static float baselines[4] = {0};
    static bool baselines_init = false;
    
    if (!baselines_init) {
        for (uint8_t i = 0; i < 4; i++) {
            baselines[i] = adc_readings[i];
        }
        baselines_init = true;
    }
    
    // Detect touches (drop below baseline)
    bool touches[4] = {false};
    
    for (uint8_t i = 0; i < 4; i++) {
        touches[i] = (baselines[i] - adc_readings[i]) > TOUCH_THRESHOLD;
    }
    
    // Update baseline when not touching (prevents drift during touch)
    for (uint8_t i = 0; i < 4; i++) {
        if (!touches[i]) {
            baselines[i] = baselines[i] * 0.95f + adc_readings[i] * 0.05f;
        }
    }
    
    // Top to bottom swipe: 0 -> 3 decreases brightness
    static enum {IDLE, TOUCH_0, TOUCH_1, TOUCH_2, TOUCH_3} swipe_state = IDLE;
    static uint32_t swipe_start_time = 0;
    static bool swipe_detected = false;
    
    if (now - swipe_start_time > SWIPE_TIMEOUT_MS) {
        swipe_state = IDLE;
    }
    
    switch (swipe_state) {
        case IDLE:
            if (touches[0]) {
                swipe_state = TOUCH_0;
                swipe_start_time = now;
            }
            break;
        case TOUCH_0:
            if (touches[1]) swipe_state = TOUCH_1;
            break;
        case TOUCH_1:
            if (touches[2]) swipe_state = TOUCH_2;
            break;
        case TOUCH_2:
            if (touches[3]) {
                current_brightness -= BRIGHTNESS_STEP;
                if (current_brightness < MIN_BRIGHTNESS) current_brightness = MIN_BRIGHTNESS;
                swipe_state = TOUCH_3;
                swipe_detected = true;
                // Trigger downward swipe animation
                swipe_anim.active = true;
                swipe_anim.direction_up = false;
                swipe_anim.position = -3;
                swipe_anim.last_update = now;
            }
            break;
        case TOUCH_3:
            if (!touches[0] && !touches[1] && !touches[2] && !touches[3]) {
                swipe_state = IDLE;
            }
            break;
    }
    
    // Bottom to top swipe: 3 ->0 increases brightness
    static enum {R_IDLE, R_TOUCH_3, R_TOUCH_2, R_TOUCH_1, R_TOUCH_0} r_swipe_state = R_IDLE;
    static uint32_t r_swipe_start_time = 0;
    
    if (now - r_swipe_start_time > SWIPE_TIMEOUT_MS) {
        r_swipe_state = R_IDLE;
    }
    
    switch (r_swipe_state) {
        case R_IDLE:
            if (touches[3]) {
                r_swipe_state = R_TOUCH_3;
                r_swipe_start_time = now;
            }
            break;
        case R_TOUCH_3:
            if (touches[2]) r_swipe_state = R_TOUCH_2;
            break;
        case R_TOUCH_2:
            if (touches[1]) r_swipe_state = R_TOUCH_1;
            break;
        case R_TOUCH_1:
            if (touches[0]) {
                current_brightness += BRIGHTNESS_STEP;
                if (current_brightness > MAX_BRIGHTNESS) current_brightness = MAX_BRIGHTNESS;
                r_swipe_state = R_TOUCH_0;
                swipe_detected = true;
                // Trigger upward swipe animation
                swipe_anim.active = true;
                swipe_anim.direction_up = true;
                swipe_anim.position = -3;
                swipe_anim.last_update = now;
            }
            break;
        case R_TOUCH_0:
            if (!touches[0] && !touches[1] && !touches[2] && !touches[3]) {
                r_swipe_state = R_IDLE;
            }
            break;
    }

#if DEBUG_PRINT
    if (now - last_debug_print > 200) {
        printf("ADC: %4d %4d %4d %4d | Base: %4.0f %4.0f %4.0f %4.0f | Touch: %d%d%d%d | Bright: %.1f%%%s\n",
               adc_readings[0], adc_readings[1], adc_readings[2], adc_readings[3],
               baselines[0], baselines[1], baselines[2], baselines[3],
               touches[0], touches[1], touches[2], touches[3],
               current_brightness * 100.0f,
               swipe_detected ? " <- SWIPE!" : "");
        
        swipe_detected = false;
        last_debug_print = now;
    }
#endif
}

// Update and get swipe animation layers
// Returns true if animation is active, fills swipe_layers and brightness arrays
bool get_swipe_animation(uint8_t swipe_layers[5], float swipe_row_brightness[5]) {
    // Empty
    for (int i = 0; i < 5; i++) {
        swipe_layers[i] = 0;
        swipe_row_brightness[i] = 0.0f;
    }
    
    if (!swipe_anim.active) {
        return false;
    }
    
    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint32_t elapsed = now - swipe_anim.last_update;
    
    // Update position every ~40ms
    if (elapsed >= (SWIPE_ANIM_DURATION_MS / SWIPE_ANIM_STEPS)) {
        swipe_anim.position++;
        swipe_anim.last_update = now;
        
        // End animation when all lines have passed through
        if (swipe_anim.position > 7) {
            swipe_anim.active = false;
            return false;
        }
    }
    
    // Calculate row positions
    int8_t row1, row2, row3;
    if (swipe_anim.direction_up) {
        row1 = 4 - swipe_anim.position;       // Leading vertical line
        row2 = 4 - (swipe_anim.position - 1); // Middle layer
        row3 = 4 - (swipe_anim.position - 2); // Trailing layer
    } else {
        row1 = swipe_anim.position;           // Leading horizontal line
        row2 = swipe_anim.position - 1;       // Middle layer
        row3 = swipe_anim.position - 2;       // Trailing layer
    }
    
    // Set horizontal lines by setting the same bit in all rows for each row
    for (int row = 0; row < 5; row++) {
        if (row1 >= 0 && row1 < 5) {
            swipe_layers[row] |= (1 << (4 - row1));
        }
        if (row2 >= 0 && row2 < 5) {
            swipe_layers[row] |= (1 << (4 - row2));
        }
        if (row3 >= 0 && row3 < 5) {
            swipe_layers[row] |= (1 << (4 - row3));
        }
    }

    // Set row brightnesses - first row is 80%, then 50%, then 15%
    if (row1 >= 0 && row1 < 5) {
        swipe_row_brightness[row1] = 0.80f;
    }
    if (row2 >= 0 && row2 < 5) {
        swipe_row_brightness[row2] = 0.50f;
    }
    if (row3 >= 0 && row3 < 5) {
        swipe_row_brightness[row3] = 0.15f;
    }
    
    return true;
}

//const char * testString = ;
enum disp_mode{
    USER = 0,
    ECSE = 1,
    EASTER = 2
};

disp_mode display_mode = USER;


char userStringBuffer[STR_BUFFER_LEN] = " Use PuTTY to Program (115200b)";
char presetStringBuffer[STR_BUFFER_LEN] = " ECSE LEAVERS 2025";
char easterEggStr[STR_BUFFER_LEN] = " COMPSYS ON TOP";
char tempBuffer[STR_BUFFER_LEN] = {0};

char * dispBuff = userStringBuffer;

char * strings[] = {
    userStringBuffer,
    presetStringBuffer,
    easterEggStr
};
uint8_t tempBufferIdx = 0;
void scroll_screen(void){
    scroll_chars();
    scroll_count++;
    if(scroll_count==7){
        counter = (counter+1) % strlen(strings[display_mode]);
        const uint8_t * disp_char = char_to_matrix(strings[display_mode][counter]);
        add_char_to_scroll(disp_char);
        scroll_count=5-((disp_char[0]&0xE0)>>5); //3MSB of first col of char = length (0-7)
    }
}

void screen_start(void){
    const uint8_t * disp_char = char_to_matrix(strings[display_mode][counter]);
    add_char_to_scroll(disp_char);
    scroll_count=5-((disp_char[0]&0xE0)>>5); //3MSB of first col of char = length (0-7)
}

repeating_timer_t scroll_timer = {0};
bool scroll_timer_cb(repeating_timer_t * timer){
    scroll_screen();
    return true;
}

void print_info(void){
    printf("------------------------------------------------\n");
    printf(BR_BLUE "ECSE LEAVERS DINNER INVITATIONS 2025\n" COLOUR_NONE);
    printf("------------------------------------------------\n");
    printf("Hello! Congratulations on Finishing your undergraduate degree.\n");
    printf("To update the displayed string, simply send a new one over serial.\n");
    printf("All ASCII printable characters should be implemented.\n");
    printf("Try the buttons to see if you can find any easter eggs,\n");
    printf("or check the git repo for more info.\n");
    printf("github.com/campbelllwright/ECSE_LeaversDinner_2025\n");
    printf("--------------------------------------------------\n");
    printf(" - PCB by : Campbell Wright, James West\n");
    printf(" - Code by: Campbell Wright\n");
    printf("---------------------------------------\n");
}



int main()
{
    tempBuffer[0] = ' ';
    tempBufferIdx = 1;
    stdio_init_all();
    init_gpio();
    read_name_from_flash(userStringBuffer, STR_BUFFER_LEN);
    screen_start();
    printf("hello, world!");
    add_repeating_timer_ms(-100,scroll_timer_cb,0,&scroll_timer);
    
    while (true) {
        static bool pb1_last = 1, pb2_last = 1;
        bool pb1_val = gpio_get(PB1);
        bool pb2_val = gpio_get(PB2);
        if((pb1_last != pb1_val)||(pb2_last!=pb2_val)){
            if((pb1_val==0) &&(pb2_val ==0)){
                display_mode = EASTER;
                counter = 0;
                const uint8_t * disp_char = char_to_matrix(strings[display_mode][counter]);
                add_char_to_scroll_start(disp_char);
                add_char_to_scroll(disp_char);
                scroll_count=5-((disp_char[0]&0xE0)>>5); //3MSB of first col of char = length (0-7)
            }else if((pb1_val == 0)){
                display_mode = ECSE;
                counter = 0;
                const uint8_t * disp_char = char_to_matrix(strings[display_mode][counter]);
                add_char_to_scroll_start(disp_char);
                add_char_to_scroll(disp_char);
                scroll_count=5-((disp_char[0]&0xE0)>>5); //3MSB of first col of char = length (0-7)
                print_info();
            }else if(pb2_val == 0){
                display_mode = USER;
                counter = 0;
                const uint8_t * disp_char = char_to_matrix(strings[display_mode][counter]);
                add_char_to_scroll_start(disp_char);
                add_char_to_scroll(disp_char);
                scroll_count=5-((disp_char[0]&0xE0)>>5); //3MSB of first col of char = length (0-7)
            }
            
            //add_char_to_scroll(char_to_matrix(stringBuffer[counter]));
        }
        pb1_last = pb1_val;

        if(gpio_get(PB2)){
            current_char=scroll_buff;//char_to_matrix(stringBuffer[counter]);
        }
        else{
            //scroll_screen();
            //current_char = char_to_matrix(128);
            //print_print_buff();
        }
        for(int i = 0; i < 100; i++){
            update_brightness_from_swipe();

            uint8_t swipe_layers[5];
            float swipe_row_brightness[5];
            bool has_swipe = get_swipe_animation(swipe_layers, swipe_row_brightness);

            // Display with or without swipe effect
            if (has_swipe) {
                disp_char_with_swipe(current_char, current_brightness, swipe_layers, swipe_row_brightness);
            } else {
                disp_char(current_char, current_brightness);
            }
            
            //This is janky - ISRs were being weird so we just do 100 display cycles for every button poll
            //which means our polling rate is worst case 100us*25*100 = 250ms.
            //if ISRs still funky maybe throw this on core 1? would be cool and leave core 0 available for user code/polling.
        }
        //scroll_screen();
        char inChar = getchar_timeout_us(10);
        if(inChar != 0xFE){
            if(inChar != '\n'){
                tempBuffer[tempBufferIdx] = inChar;
                tempBufferIdx++;
            }
            else{
                tempBuffer[tempBufferIdx] = 0;
                printf("Displaying String \"%s\"\n", tempBuffer);
                memccpy(userStringBuffer, tempBuffer, 0, 64);
                uint rc = write_name_to_flash(userStringBuffer);
                if(!rc){
                    printf("wrote string \"%s\" (%d bytes) to flash\n", userStringBuffer, strlen(userStringBuffer)+1);
                }
                counter = 0;
                tempBuffer[0] = ' ';
                tempBufferIdx = 1;
            }
        }
        
    }
}
