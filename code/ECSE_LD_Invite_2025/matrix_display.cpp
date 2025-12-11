#include <stdio.h>
#include "pico/stdlib.h"
#include "matrix_display.hpp"
#include "pindefs.hpp"
#include "string.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "build/matrix.pio.h"
#include "clw_dbgutils.h"




//==========================================
// PIO MATRIX CONTROL

// Pack 5x5 character into 25-bit frame.
static uint32_t pack_character_frame(const uint8_t character[5], uint8_t delay) {
    uint32_t frame = 0;
    for(uint i = 0; i < 5; i++){
        frame |= ((character[i]&0x1F)<<(5*i));
    }
    return frame |= (uint)(delay&0x7F)<<25;
}

void push_display(const uint8_t character[5], uint8_t brightness){
    uint32_t frame = pack_character_frame(character, brightness);
    pio_sm_put_blocking(LED_PIO, LED_SM, frame);
}


//==========================================================================

static const uint rows[] = {LED_R1,LED_R2,LED_R3,LED_R4,LED_R5};
static const uint cols[] = {LED_C1,LED_C2,LED_C3,LED_C4,LED_C5};


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

uint8_t scroll_buff[15] = {0};
void add_char_to_scroll_start(const uint8_t * character){
    memset(&(scroll_buff[0]),0,14);
    memcpy(&(scroll_buff[0]), character,5);
}

void add_char_to_scroll(const uint8_t * character){
    uint8_t prev_char_len = ((scroll_buff[0]&0xE0)>>5)+2;
    memset(&(scroll_buff[8]),0,5);
    memcpy(&(scroll_buff[8]), character,5);
    scroll_buff[13]=0;
}
void scroll_chars(void){
    for(uint i = 0; i < 13; i++){
        scroll_buff[i] = scroll_buff[i+1];
    }
    scroll_buff[13] = 0;
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





