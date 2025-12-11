#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include "matrix_display.hpp"
#include "pindefs.hpp"
#include "pico_flash.hpp"
#include "clw_dbgutils.h"
#include "build/matrix.pio.h"
#define STR_BUFFER_LEN 128




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
}

uint counter = 0;
uint scroll_count = 0;
const uint8_t * current_char;
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
    sleep_ms(1000);
    read_name_from_flash(userStringBuffer, STR_BUFFER_LEN);
    printf("pio start:%d\n",init_pio_matrix());
    screen_start();
    
    printf("hello, world! ");

    add_repeating_timer_ms(100,scroll_timer_cb,0,&scroll_timer);
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
            current_char=scroll_buff;
        }
        else{
            current_char = char_to_matrix(128);
        }

        push_display(current_char,0);
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
