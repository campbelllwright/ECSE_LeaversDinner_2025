#ifndef MATRIX_DISPLAY_HPP
#define MATRIX_DISPLAY_HPP
#include <stdio.h>
#include <pico/stdlib.h>
extern uint8_t scroll_buff[15];
const uint8_t* char_to_matrix(const char charIn);
void disp_char(const uint8_t * character, float brightness);
void disp_char_with_swipe(const uint8_t * character, float brightness, const uint8_t * swipe_layers, const float * swipe_row_brightness);
void scroll_chars(void);
void add_char_to_scroll(const uint8_t * character);
void add_char_to_scroll_start(const uint8_t * character);
void print_matrix(const uint8_t * character);
void print_print_buff(void);
#endif