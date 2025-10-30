#include <stdio.h>
#include "pico/stdlib.h"
#include "matrix_display.hpp"
#include "pindefs.hpp"
#include "string.h"

#include "clw_dbgutils.h"


constexpr uint LED_period_us = 100;

const uint8_t symbols[][5] = { //ascii 0x20 - 0x2F
    {(((0x03)<<5)|0b00000), 0b00000, 0b00000, 0b00000, 0b00000}, // (space)
    {(((0x03)<<5)|0b00000), 0b11101, 0b00000, 0b00000, 0b00000}, // !
    {(((0x03)<<5)|0b10000), 0b00000, 0b10000, 0b00000, 0b00000}, // "
    {(((0x05)<<5)|0b01010), 0b11111, 0b01010, 0b11111, 0b01010}, // #
    {(((0x05)<<5)|0b01001), 0b10101, 0b11111, 0b10101, 0b10010}, // $
    {(((0x05)<<5)|0b11001), 0b11010, 0b00100, 0b01011, 0b10011}, // %
    {(((0x05)<<5)|0b00000), 0b00000, 0b00000, 0b00000, 0b00000}, // &?
    {(((0x03)<<5)|0b00000), 0b10000, 0b00000, 0b00000, 0b00000}, // `
    {(((0x04)<<5)|0b00000), 0b01110, 0b10001, 0b00000, 0b00000}, //(
    {(((0x04)<<5)|0b00000), 0b00000, 0b10001, 0b01110, 0b00000}, //)
    {(((0x05)<<5)|0b10101), 0b01110, 0b11111, 0b01110, 0b10101}, //*
    {(((0x05)<<5)|0b00100), 0b00100, 0b11111, 0b00100, 0b00100}, //+
    {(((0x03)<<5)|0b00001), 0b00010, 0b00000, 0b00000, 0b00000}, //,
    {(((0x05)<<5)|0b00100), 0b00100, 0b00100, 0b00100, 0b00100}, //-
    {(((0x03)<<5)|0b00000), 0b00000, 0b00001, 0b00000, 0b00000}, //.
    {(((0x03)<<5)|0b00011), 0b00100, 0b11000, 0b00000, 0b00000}, ///
};
const uint8_t numbers[][5] = {//ascii 0x30-0x39
    {(((0x05)<<5)|0b01110), 0b10001, 0b10001, 0b10001, 0b01110}, // 0
    {(((0x05)<<5)|0b01001), 0b01001, 0b11111, 0b00001, 0b00001}, // 1
    {(((0x05)<<5)|0b10011), 0b10101, 0b10101, 0b10101, 0b01001}, // 2
    {(((0x05)<<5)|0b10101), 0b10101, 0b10101, 0b10101, 0b01010}, // 3
    {(((0x05)<<5)|0b11110), 0b00010, 0b00010, 0b11111, 0b00010}, // 4
    {(((0x05)<<5)|0b11101), 0b10101, 0b10101, 0b10101, 0b10010}, // 5
    {(((0x05)<<5)|0b01110), 0b10101, 0b10101, 0b10101, 0b00010}, // 6
    {(((0x05)<<5)|0b10000), 0b10000, 0b10011, 0b10100, 0b11000}, // 7
    {(((0x05)<<5)|0b01010), 0b10101, 0b10101, 0b10101, 0b01010}, // 8
    {(((0x05)<<5)|0b01000), 0b10101, 0b10101, 0b10101, 0b01110}  // 9
};
const uint8_t symbols2[][5] = { //ascii 0x3A-0x40
    {(((0x03)<<5)|0b00000), 0b01010, 0b00000, 0b00000, 0b00000},//:
    {(((0x03)<<5)|0b00001), 0b01010, 0b00000, 0b00000, 0b00000},//;
    {(((0x05)<<5)|0b00100), 0b01010, 0b01010, 0b10001, 0b10001},//<
    {(((0x05)<<5)|0b01010), 0b01010, 0b01010, 0b01010, 0b01010},//=
    {(((0x05)<<5)|0b10001), 0b10001, 0b01010, 0b01010, 0b00100},//<
    {(((0x05)<<5)|0b00000), 0b10000, 0b10101, 0b11000, 0b00000},//?
    {(((0x05)<<5)|0b11111), 0b11111, 0b11011, 0b11111, 0b11111},//@ (creatively)
};
const uint8_t alphabet[][5] = { //ascii 0x41-0x5A + 0x61-0x7A
    {0b10101111, 0b10100, 0b10100, 0b10100, 0b01111}, // A
    {0b10111111, 0b10101, 0b10101, 0b10101, 0b01010}, // B
    {0b10101110, 0b10001, 0b10001, 0b10001, 0b10001}, // C
    {0b10111111, 0b10001, 0b10001, 0b10001, 0b01110}, // D
    {0b10111111, 0b10101, 0b10101, 0b10101, 0b10001}, // E
    {0b10111111, 0b10100, 0b10100, 0b10100, 0b10000}, // F
    {0b10101110, 0b10001, 0b10001, 0b10101, 0b10111}, // G
    {0b10111111, 0b00100, 0b00100, 0b00100, 0b11111}, // H 
    {0b10110001, 0b10001, 0b11111, 0b10001, 0b10001}, // I
    {0b10110010, 0b10001, 0b10001, 0b11110, 0b10000}, // J
    {0b10111111, 0b00100, 0b00100, 0b01010, 0b10001}, // K
    {0b10111111, 0b00001, 0b00001, 0b00001, 0b00001}, // L
    {0b10111111, 0b10000, 0b01110, 0b10000, 0b11111}, // M
    {0b10111111, 0b10000, 0b01110, 0b00001, 0b11111}, // N
    {0b10101110, 0b10001, 0b10001, 0b10001, 0b01110}, // O
    {0b10111111, 0b10010, 0b10010, 0b10010, 0b01100}, // P
    {0b10101110, 0b10001, 0b10001, 0b10010, 0b01101}, // Q
    {0b10111111, 0b10010, 0b10010, 0b10010, 0b01101}, // R
    {0b10101001, 0b10101, 0b10101, 0b10101, 0b10010}, // S
    {0b10110000, 0b10000, 0b11111, 0b10000, 0b10000}, // T
    {0b10111110, 0b00001, 0b00001, 0b00001, 0b11110}, // U
    {0b10111000, 0b00110, 0b00001, 0b00110, 0b11000}, // V
    {0b10111110, 0b00001, 0b01110, 0b00001, 0b11110}, // W
    {0b10110001, 0b01010, 0b00100, 0b01010, 0b10001}, // X
    {0b10110000, 0b01000, 0b00111, 0b01000, 0b10000}, // Y
    {0b10110010, 0b10101, 0b10101, 0b10101, 0b01001}  // Z
};
const uint8_t symbols3[][5] = { //ascii 0x5B-0x60
    {(((0x03)<<5)|0b11111), 0b10001, 0b00000, 0b00000, 0b00000},//[
    {(((0x03)<<5)|0b11000), 0b00100, 0b00011, 0b00000, 0b00000},//"\"
    {(((0x03)<<5)|0b00000), 0b10001, 0b11111, 0b00000, 0b00000},//[
    {(((0x03)<<5)|0b01000), 0b10000, 0b01000, 0b00000, 0b00000},//^
    {(((0x05)<<5)|0b00001), 0b00001, 0b00001, 0b00001, 0b00001},//_
    {(((0x02)<<5)|0b10000), 0b01000, 0b00000, 0b00000, 0b00000}//`
};
const uint8_t symbols4[][5] = { //ascii 0x7B-0x7E
    {(((0x03)<<5)|0b00100), 0b11111, 0b10001, 0b00000, 0b00000},//{
    {(((0x01)<<5)|0b11111), 0b00000, 0b00000, 0b00000, 0b00000},//|
    {(((0x03)<<5)|0b10001), 0b11111, 0b00100, 0b00000, 0b00000},//}
    {(((0x05)<<5)|0b01000), 0b01000, 0b00100, 0b00010, 0b00100}//~
};
const uint8_t icons[][5] = {
    {(((0x05)<<5)|0b00010), 0b11101, 0b00001, 0b11101, 0b00010} // smiley face
};


/*
const uint8_t alphabet[][5] = {
    {0b01110, 0b10001, 0b11111, 0b10001, 0b10001},//A
    {0b11110, 0b10001, 0b11110, 0b10001, 0b11110},//B
    {0b01111, 0b10000, 0b10000, 0b10000, 0b01111},//C
    {0b11110, 0b10001, 0b10001, 0b10001, 0b11110},//D
    {0b11111, 0b10000, 0b11110, 0b10000, 0b11111},//E
    {0b11111, 0b10000, 0b11110, 0b10000, 0b10000},//F
    {0b01111, 0b10000, 0b10011, 0b10001, 0b01111},//G
    {0b10001, 0b10001, 0b11111, 0b10001, 0b10001},//H
    {0b11111, 0b00100, 0b00100, 0b00100, 0b11111},//I
    {0b11111, 0b00010, 0b00010, 0b10010, 0b01100},//J
    {0b10001, 0b10010, 0b11100, 0b10010, 0b10001},//K
    {0b10000, 0b10000, 0b10000, 0b10000, 0b11111},//L
    {0b11011, 0b10101, 0b10101, 0b10101, 0b10001},//M
    {0b11001, 0b10101, 0b10101, 0b10101, 0b10011},//N
    {0b01110, 0b10001, 0b10001, 0b10001, 0b01110},//O
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000},//P
    {0b01110, 0b10001, 0b10001, 0b10010, 0b01101},//Q
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001},//R
    {0b01111, 0b10000, 0b01110, 0b00001, 0b11110},//S
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100},//T
    {0b10001, 0b10001, 0b10001, 0b10001, 0b01110},//U
    {0b10001, 0b10001, 0b01010, 0b01010, 0b00100},//V
    {0b10001, 0b10101, 0b10101, 0b10101, 0b01010},//W
    {0b10001, 0b01010, 0b00100, 0b01010, 0b10001},//X
    {0b10001, 0b01010, 0b00100, 0b00100, 0b00100},//Y
    {0b11110, 0b00001, 0b01110, 0b10000, 0b01111} //Z
};
const uint8_t symbols[][5] = {
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000},// 
    {0b00100, 0b00100, 0b00100, 0b00000, 0b00100},//!
    {0b01010, 0b00000, 0b00000, 0b00000, 0b00000},//"
    {0b01010, 0b11111, 0b01010, 0b11111, 0b01010},//#
    {0b01111, 0b10100, 0b01110, 0b00101, 0b11110},//$
    {0b11001, 0b11010, 0b00100, 0b01011, 0b10011}//%
    
};
const uint8_t icons[][5] = {
    {0b01010, 0b01010, 0b01010, 0b10001, 0b01110}
};*/
const uint8_t* char_to_matrix(const char charIn){
    if(((charIn >= 'A')&&(charIn <= 'Z'))||((charIn >= 'a')&&(charIn <= 'z'))){
        return alphabet[(charIn&0x1F)-1];
    }
    if((charIn >= '0')&&(charIn <='9')){
        return numbers[charIn&0x0F];
    }
    if((charIn >= ' ')&&(charIn <='/')){
        return symbols[charIn&0x0F];
    }
    if((charIn >= ':')&&(charIn <= '@')){
        return symbols2[charIn-0x3A];
    }
    if((charIn >= '[')&&(charIn <= '`')){
        return symbols3[charIn-0x5B];
    }
    if((charIn >= '{')&&(charIn <= '~')){
        return symbols4[charIn-0x7B];
    }
    if(charIn > 0x7F){
        return icons[charIn&0x7F];
    }
    return symbols[0];
}

inline void set_led(uint x, uint y){
    static const uint rows[] = {LED_R1,LED_R2,LED_R3,LED_R4,LED_R5};
    static const uint cols[] = {LED_C1,LED_C2,LED_C3,LED_C4,LED_C5};
    gpio_put_masked(MASK_ALL_COLS|MASK_ALL_ROWS, (1<<cols[y])|(MASK_ALL_ROWS &~(1<<rows[x])));
    sleep_us(LED_period_us);
}

inline void clear_matrix(void){
    static const uint rows[] = {LED_R1,LED_R2,LED_R3,LED_R4,LED_R5};
    static const uint cols[] = {LED_C1,LED_C2,LED_C3,LED_C4,LED_C5};
    gpio_put_masked(MASK_ALL_COLS|MASK_ALL_ROWS, MASK_ALL_ROWS);
    sleep_us(LED_period_us);
}


uint8_t scroll_buff[15] = {0};
void add_char_to_scroll_start(const uint8_t * character){
    memset(&(scroll_buff[0]),0,14);
    memcpy(&(scroll_buff[0]), character,5);
}
//((character[0]&0xE0)>>5)
void add_char_to_scroll(const uint8_t * character){
    uint8_t prev_char_len = ((scroll_buff[0]&0xE0)>>5)+2;
    memset(&(scroll_buff[8]),0,5);
    memcpy(&(scroll_buff[8]), character,5);
    scroll_buff[13]=0;
    //scroll_buff[prev_char_len+((character[0]&0xE0)>>5)]=0;
}

void print_print_buff(void){
    for(uint8_t i = 0; i < 5; i++){ 
        printf("%s%0d%s%0d%s%0d%s%0d%s%0d %s%0d%s%0d%s%0d%s%0d%s%0d %s%0d%s%0d%s%0d%s%0d%s%0d%s\n", 
            ((scroll_buff[0]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[0]>>(4-i)&0x01),
            ((scroll_buff[1]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[1]>>(4-i)&0x01),
            ((scroll_buff[2]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[2]>>(4-i)&0x01),
            ((scroll_buff[3]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[3]>>(4-i)&0x01),
            ((scroll_buff[4]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[4]>>(4-i)&0x01),
            ((scroll_buff[5]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[5]>>(4-i)&0x01),
            ((scroll_buff[6]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[6]>>(4-i)&0x01),
            ((scroll_buff[7]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[7]>>(4-i)&0x01),
            ((scroll_buff[8]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[8]>>(4-i)&0x01),
            ((scroll_buff[9]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[9]>>(4-i)&0x01),
            ((scroll_buff[10]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[10]>>(4-i)&0x01),
            ((scroll_buff[11]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[11]>>(4-i)&0x01),
            ((scroll_buff[12]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[12]>>(4-i)&0x01),
            ((scroll_buff[13]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[13]>>(4-i)&0x01),
            ((scroll_buff[14]>>(4-i)&0x01)?BG_BLUE:BG_BLACK),(scroll_buff[14]>>(4-i)&0x01),
        COLOUR_NONE);
    }
    printf("------------\n");
}

void print_matrix(const uint8_t * character){
    for(uint8_t i = 0; i < 5; i++){ 
        printf("%0d%0d%0d%0d%0d\n", 
            (character[0]>>(4-i)&0x01),
            (character[1]>>(4-i)&0x01),
            (character[2]>>(4-i)&0x01),
            (character[3]>>(4-i)&0x01),
            (character[4]>>(4-i)&0x01));
    }
    printf("\n");
}

void scroll_chars(void){
    for(uint i = 0; i < 13; i++){
        scroll_buff[i] = scroll_buff[i+1];
    }
    scroll_buff[13] = 0;
}

// Helper function to apply cubic brightness scaling
static inline float apply_brightness_scaling(float brightness) {
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;
    // Cubic scaling for perceptual linearity: 0.5(x+0.25)^3 + 0.02
    return 0.5f * (brightness + 0.25f) * (brightness + 0.25f) * (brightness + 0.25f) + 0.02f;
}

// Helper function to render a single pixel with brightness
static inline void render_pixel(uint8_t row, uint8_t col, float brightness) {
    static const uint rows[] = {LED_R1, LED_R2, LED_R3, LED_R4, LED_R5};
    static const uint cols[] = {LED_C1, LED_C2, LED_C3, LED_C4, LED_C5};
    
    uint on_time_us = (uint)(LED_period_us * brightness);
    uint off_time_us = LED_period_us - on_time_us;
    
    if (on_time_us > 0) {
        gpio_put_masked(MASK_ALL_COLS|MASK_ALL_ROWS, (1<<cols[col])|(MASK_ALL_ROWS &~(1<<rows[row])));
        sleep_us(on_time_us);
    }
    
    if (off_time_us > 0) {
        gpio_put_masked(MASK_ALL_COLS|MASK_ALL_ROWS, MASK_ALL_ROWS);
        sleep_us(off_time_us);
    }
}


void disp_char(const uint8_t * character, float brightness){
    brightness = apply_brightness_scaling(brightness);
    
    for(uint8_t row = 0; row < 5; row++){ 
        for(uint8_t col = 0; col < 5; col++){
            if((character[row]>>(4-col))&0x01){
                render_pixel(row, col, brightness);
            } 
        }
    }
    clear_matrix();
}

// Display character with swipe effect layers in background
void disp_char_with_swipe(const uint8_t * character, float brightness, const uint8_t * swipe_layers, const float * swipe_col_brightness){
    brightness = apply_brightness_scaling(brightness);
    
    for(uint8_t row = 0; row < 5; row++){ 
        for(uint8_t col = 0; col < 5; col++){
            bool text_pixel = (character[row]>>(4-col))&0x01;
            bool swipe_pixel = (swipe_layers[row]>>(4-col))&0x01;
            
            // Text has priority - render at full brightness
            if (text_pixel) {
                render_pixel(row, col, brightness);
            }
            // Swipe effect in background - use row brightness for vertical lines
            else if (swipe_pixel && swipe_col_brightness[col] > 0.0f) {
                float swipe_bright = apply_brightness_scaling(swipe_col_brightness[col]);
                render_pixel(row, col, swipe_bright);
            }
        }
    }
    clear_matrix();
}