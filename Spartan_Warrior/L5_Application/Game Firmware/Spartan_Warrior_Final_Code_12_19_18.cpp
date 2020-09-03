#include <stdio.h>
#include <stdint.h>
#include "utilities.h"
#include "io.hpp"
#include "LPC17xx.h"
#include "tasks.hpp"
#include "FreeRTOS.h"
#include "stdlib.h"
#include <lpc_isr.h>
#include "event_groups.h"
#include <time.h>

// Start and End screen flags
bool playing_game = false;
bool play_again_flag = false;

// Task Handles
TaskHandle_t start_screen;
TaskHandle_t game_over_screen;
TaskHandle_t iterate_through_led_matrix_rows_start_end_handle;
TaskHandle_t iterate_through_led_matrix_rows_handle;
TaskHandle_t draw_enemy_1_handle;
TaskHandle_t draw_enemy_2_handle;
TaskHandle_t draw_enemy_3_handle;
TaskHandle_t draw_enemy_4_handle;
TaskHandle_t draw_enemy_5_handle;
TaskHandle_t player_sprite_handle;
TaskHandle_t player_sprite_move_handle;
TaskHandle_t detect_a_hit_handle;
TaskHandle_t fire_a_shot_handle;

// LED Matrix variables
uint32_t adafruit_columns[32] = { };
uint32_t * adafruit_columns_ptr = adafruit_columns;

// Hit Detection variables
bool hit_flag_1 = false;
bool hit_flag_2 = false;
bool hit_flag_3 = false;
bool hit_flag_4 = false;
bool hit_flag_5 = false;
uint32_t * shot_Ptr = nullptr;
uint32_t * enemy_Ptr = nullptr;

// Acceleration Sensor variables
int acceleration_sensor_feedback_Y;
int acceleration_sensor_feedback_Y_avg;
int previous_AS_value;
int AS_prev_curr_compare;
int AS_feedback_bit_shift = 0;

// Player Sprite variables
uint32_t player_sprite = 0x000FF000;

// Score tracking
int score = 0;

uint32_t reg1[32] = { };
uint32_t reg2[32] = { };
uint32_t reg3[32] = { };
uint8_t h;
uint8_t t;
uint8_t u;

void detect_a_hit(void * p)
{
    while (1) {
        for (int column = 0; column < 32; column++) {
            if (((*(adafruit_columns_ptr + 22) >> column) & 1) && ((*(adafruit_columns_ptr + 23) >> column) & 1)
                    && ((*(adafruit_columns_ptr + 24) >> column) & 1) && ((*(adafruit_columns_ptr + 25) >> column) & 1)) {
                hit_flag_5 = true;
            }

            if (((*(adafruit_columns_ptr + 17) >> column) & 1) && ((*(adafruit_columns_ptr + 18) >> column) & 1)
                    && ((*(adafruit_columns_ptr + 20) >> column) & 1)) {
                hit_flag_2 = true;
            }

            if (((*(adafruit_columns_ptr + 12) >> column) & 1)
                    && (((*(adafruit_columns_ptr + 14) >> column) & 1) || ((*(adafruit_columns_ptr + 15) >> column) & 1))) {
                hit_flag_1 = true;
            }

            if (((*(adafruit_columns_ptr + 7) >> column) & 1) && ((*(adafruit_columns_ptr + 8) >> column) & 1)
                    && ((*(adafruit_columns_ptr + 10) >> column) & 1)) {
                hit_flag_4 = true;
            }

            if (((*(adafruit_columns_ptr + 2) >> column) & 1) && ((*(adafruit_columns_ptr + 5) >> column) & 1)) {
                hit_flag_3 = true;
            }
        }
        vTaskDelay(1);
    }
}

void initialize_LPC_To_LED_Matrix_GPIO_Pins(void)
{
    // Configure RGB Pins
    // set P0.26 as GPIO (G1)
    LPC_PINCON->PINSEL3 &= ~(1 << 21);
    LPC_PINCON->PINSEL3 &= ~(1 << 20);

    // set P0.1 as GPIO (R1)
    LPC_PINCON->PINSEL0 &= ~(1 << 3);
    LPC_PINCON->PINSEL0 &= ~(1 << 2);

    // set P0.0 as GPIO (B1)
    LPC_PINCON->PINSEL0 &= ~(1 << 1);
    LPC_PINCON->PINSEL0 &= ~(1 << 0);

    // set P1.19 as GPIO (R2)
    LPC_PINCON->PINSEL3 &= ~(1 << 7);
    LPC_PINCON->PINSEL3 &= ~(1 << 6);

    // set P1.20 as GPIO (B2)
    LPC_PINCON->PINSEL3 &= ~(1 << 9);
    LPC_PINCON->PINSEL3 &= ~(1 << 8);

    // set P1.31 as GPIO (G2)
    LPC_PINCON->PINSEL3 &= ~(1 << 31);
    LPC_PINCON->PINSEL3 &= ~(1 << 30);

    // set P1.28 as GPIO (A)
    LPC_PINCON->PINSEL4 &= ~(1 << 1);
    LPC_PINCON->PINSEL4 &= ~(1 << 0);

    // set P1.22 as GPIO (B)
    LPC_PINCON->PINSEL3 &= ~(1 << 13);
    LPC_PINCON->PINSEL3 &= ~(1 << 12);

    // set P1.29 as GPIO (C)
    LPC_PINCON->PINSEL4 &= ~(1 << 3);
    LPC_PINCON->PINSEL4 &= ~(1 << 2);

    // set P1.23 as GPIO (D)
    LPC_PINCON->PINSEL3 &= ~(1 << 15);
    LPC_PINCON->PINSEL3 &= ~(1 << 14);

    // set P2.6 as GPIO (clock)
    LPC_PINCON->PINSEL4 &= ~(1 << 13);
    LPC_PINCON->PINSEL4 &= ~(1 << 12);

    // set P2.7 as GPIO (latch)
    LPC_PINCON->PINSEL4 &= ~(1 << 15);
    LPC_PINCON->PINSEL4 &= ~(1 << 14);

    // set P2.5 as (output enable)
    LPC_PINCON->PINSEL4 &= ~(1 << 11);
    LPC_PINCON->PINSEL4 &= ~(1 << 10);

    // Set P1.0 as output to test hit detection
    LPC_PINCON->PINSEL2 &= ~(3 << 0);

    // Set P1.9 as input to use SW0 as trigger for shooting
    LPC_PINCON->PINSEL2 &= ~(3 << 18);

    // Set P1.29 as MP3 signal
    LPC_PINCON->PINSEL3 &= ~(3 << 26);

    LPC_PINCON->PINSEL1 &= ~(3 << 26);

    // Configure all GPIO pins being used as outputs (as output pins)
    LPC_GPIO0->FIODIR |= (1 << 26); // G1 = o
    LPC_GPIO0->FIODIR |= (1 << 1); // R1 = o
    LPC_GPIO0->FIODIR |= (1 << 0); // B1 = o
    LPC_GPIO1->FIODIR |= (1 << 19); // R2 = o
    LPC_GPIO1->FIODIR |= (1 << 20); // B2 = o
    LPC_GPIO1->FIODIR |= (1 << 31); // G2 = o
    LPC_GPIO2->FIODIR |= (1 << 0); // A = o
    LPC_GPIO1->FIODIR |= (1 << 22); // B = o
    LPC_GPIO2->FIODIR |= (1 << 1); // C = o
    LPC_GPIO1->FIODIR |= (1 << 23); // D = o
    LPC_GPIO2->FIODIR |= (1 << 6); // clock = o
    LPC_GPIO2->FIODIR |= (1 << 7); // latch = o
    LPC_GPIO2->FIODIR |= (1 << 5); // output enable = o
    LPC_GPIO1->FIODIR |= (1 << 0); // LED 0 (local to LPC 1758)
    LPC_GPIO1->FIODIR &= ~(1 << 9); // SW 0 (local to the LPC 1758) test trigger
    LPC_GPIO1->FIODIR |= (1 << 29); // MP3 signal
    LPC_GPIO0->FIODIR &= ~(1 << 29); // trigger signal

}

void print_score(void * p)
{
    while (1) {
        printf("Score: %d\n", score);
        vTaskDelay(100);
    }
}

void draw_enemy_1(void * p)
{
    srand(0);

    while (1) {

        for (int column = 0; column < 32; column = (column + 1) % 32) {
            if (column >= 31 || hit_flag_1) {
                adafruit_columns[11] = 0x00000000;
                adafruit_columns[12] = 0x00000000;
                adafruit_columns[13] = 0x00000000;
                vTaskDelay(rand() % 5000);
                if (hit_flag_1) {
                    score += 1;
                }
                hit_flag_1 = false;
                break;
            }
            else {
                adafruit_columns[11] = (0x0000018F << column);
                adafruit_columns[12] = (0x000001FF << column);
                adafruit_columns[13] = (0x0000018F << column);
                vTaskDelay(100);
            }
        }
    }
}

void draw_enemy_2(void * p)
{
    srand(0);

    while (1) {

        for (int column = 0; column < 32; column = (column + 1) % 32) {
            if (column >= 31 || hit_flag_2) {
                adafruit_columns[16] = 0x00000000;
                adafruit_columns[17] = 0x00000000;
                adafruit_columns[18] = 0x00000000;
                adafruit_columns[19] = 0x00000000;
                vTaskDelay(rand() % 2000);
                if (hit_flag_2) {
                    score += 5;
                }
                hit_flag_2 = false;
                break;

            }
            else {
                adafruit_columns[16] = (0x0000000E << column);
                adafruit_columns[17] = (0x0000003F << column);
                adafruit_columns[18] = (0x0000003F << column);
                adafruit_columns[19] = (0x0000000E << column);
                vTaskDelay(30);
            }
        }
    }
}

void draw_enemy_3(void * p)
{
    srand(0);

    while (1) {

        for (int column = 0; column < 32; column = (column + 1) % 32) {
            if (column >= 31 || hit_flag_3) {
                adafruit_columns[0] = 0x00000000;
                adafruit_columns[1] = 0x00000000;
                adafruit_columns[2] = 0x00000000;
                adafruit_columns[3] = 0x00000000;
                adafruit_columns[4] = 0x00000000;
                vTaskDelay(rand() % 5000);
                if (hit_flag_3) {
                    score += 7;
                }
                hit_flag_3 = false;
                break;
            }
            else {
                adafruit_columns[0] = (0x00000011 << column);
                adafruit_columns[1] = (0x0000000A << column);
                adafruit_columns[2] = (0x000000FE << column);
                adafruit_columns[3] = (0x0000000A << column);
                adafruit_columns[4] = (0x00000011 << column);
                vTaskDelay(30);
            }
        }
    }
}

void draw_enemy_4(void * p)
{
    srand(0);

    while (1) {

        for (int column = 0; column < 32; column = (column + 1) % 32) {
            if (column >= 31 || hit_flag_4) {
                adafruit_columns[6] = 0x00000000;
                adafruit_columns[7] = 0x00000000;
                adafruit_columns[8] = 0x00000000;
                adafruit_columns[9] = 0x00000000;
                vTaskDelay(rand() % 4000);
                if (hit_flag_4) {
                    score += 3;
                }
                hit_flag_4 = false;
                break;
            }
            else {
                adafruit_columns[6] = (0x0000001E << column);
                adafruit_columns[7] = (0x000000FF << column);
                adafruit_columns[8] = (0x000000FF << column);
                adafruit_columns[9] = (0x0000001E << column);
                vTaskDelay(50);
            }
        }
    }
}

void draw_enemy_5(void * p)
{
    srand(0);

    while (1) {

        for (int column = 0; column < 32; column = (column + 1) % 32) {
            if (column >= 31 || hit_flag_5) {
                adafruit_columns[21] = 0x00000000;
                adafruit_columns[22] = 0x00000000;
                adafruit_columns[23] = 0x00000000;
                adafruit_columns[24] = 0x00000000;
                vTaskDelay(rand() % 4000);
                if (hit_flag_5) {
                    score += 2;
                }
                hit_flag_5 = false;
                break;
            }
            else {
                adafruit_columns[21] = (0x0000003F << column);
                adafruit_columns[22] = (0x0000000E << column);
                adafruit_columns[23] = (0x0000000E << column);
                adafruit_columns[24] = (0x0000003F << column);
                vTaskDelay(40);
            }
        }
    }
}

void draw_player_sprite(void * p)
{
    while (1) {
        if (AS_feedback_bit_shift > 0) {
            adafruit_columns[28] = (0x00030000 >> abs(AS_feedback_bit_shift));
            adafruit_columns[29] = (0x00078000 >> abs(AS_feedback_bit_shift));
            adafruit_columns[30] = (0x00078000 >> abs(AS_feedback_bit_shift));
            adafruit_columns[31] = (0x00030000 >> abs(AS_feedback_bit_shift));
        }
        else {
            adafruit_columns[28] = (0x00030000 << abs(AS_feedback_bit_shift));
            adafruit_columns[29] = (0x00078000 << abs(AS_feedback_bit_shift));
            adafruit_columns[30] = (0x00078000 << abs(AS_feedback_bit_shift));
            adafruit_columns[31] = (0x00030000 << abs(AS_feedback_bit_shift));
        }
        vTaskDelay(10);
    }

    while (1) {

    }
}

void move_player_sprite(void * p)
{
    const int acceleration_sensor_step_value = 60;

    while (1) {
        acceleration_sensor_feedback_Y = 0;
        acceleration_sensor_feedback_Y = AS.getY();

        AS_prev_curr_compare = previous_AS_value - acceleration_sensor_feedback_Y;

        if (abs(AS_prev_curr_compare) > 500) {

            if ((acceleration_sensor_feedback_Y > -50) && (acceleration_sensor_feedback_Y < 50)) {
                acceleration_sensor_feedback_Y = 0;
            }
            else if ((acceleration_sensor_feedback_Y > 50) && (acceleration_sensor_feedback_Y < 150)) {
                acceleration_sensor_feedback_Y = 100;
            }
            else if ((acceleration_sensor_feedback_Y > 150) && (acceleration_sensor_feedback_Y < 250)) {
                acceleration_sensor_feedback_Y = 200;
            }
            else if ((acceleration_sensor_feedback_Y > 250) && (acceleration_sensor_feedback_Y < 350)) {
                acceleration_sensor_feedback_Y = 300;
            }
            else if ((acceleration_sensor_feedback_Y > 350) && (acceleration_sensor_feedback_Y < 450)) {
                acceleration_sensor_feedback_Y = 400;
            }
            else if ((acceleration_sensor_feedback_Y > 450) && (acceleration_sensor_feedback_Y < 550)) {
                acceleration_sensor_feedback_Y = 500;
            }
            else if ((acceleration_sensor_feedback_Y > 550) && (acceleration_sensor_feedback_Y < 650)) {
                acceleration_sensor_feedback_Y = 600;
            }
            else if ((acceleration_sensor_feedback_Y > 650) && (acceleration_sensor_feedback_Y < 750)) {
                acceleration_sensor_feedback_Y = 700;
            }
            else if ((acceleration_sensor_feedback_Y > 750) && (acceleration_sensor_feedback_Y < 850)) {
                acceleration_sensor_feedback_Y = 800;
            }
            else if ((acceleration_sensor_feedback_Y > 850) && (acceleration_sensor_feedback_Y < 950)) {
                acceleration_sensor_feedback_Y = 900;
            }
            else if ((acceleration_sensor_feedback_Y > 950)) {
                acceleration_sensor_feedback_Y = 1000;
            }
            else if (acceleration_sensor_feedback_Y < -1000) {
                acceleration_sensor_feedback_Y = -1000;
            }
            else if ((acceleration_sensor_feedback_Y < -850) && (acceleration_sensor_feedback_Y > -950)) {
                acceleration_sensor_feedback_Y = -900;
            }
            else if ((acceleration_sensor_feedback_Y < -750) && (acceleration_sensor_feedback_Y > -850)) {
                acceleration_sensor_feedback_Y = -800;
            }
            else if ((acceleration_sensor_feedback_Y < -650) && (acceleration_sensor_feedback_Y > -750)) {
                acceleration_sensor_feedback_Y = -700;
            }
            else if ((acceleration_sensor_feedback_Y < -550) && (acceleration_sensor_feedback_Y > -650)) {
                acceleration_sensor_feedback_Y = -600;
            }
            else if ((acceleration_sensor_feedback_Y < -450) && (acceleration_sensor_feedback_Y > -550)) {
                acceleration_sensor_feedback_Y = -500;
            }
            else if ((acceleration_sensor_feedback_Y < -350) && (acceleration_sensor_feedback_Y > -450)) {
                acceleration_sensor_feedback_Y = -400;
            }
            else if ((acceleration_sensor_feedback_Y < -250) && (acceleration_sensor_feedback_Y > -350)) {
                acceleration_sensor_feedback_Y = -300;
            }
            else if ((acceleration_sensor_feedback_Y < -150) && (acceleration_sensor_feedback_Y > -250)) {
                acceleration_sensor_feedback_Y = -200;
            }
            else if ((acceleration_sensor_feedback_Y < -50) && (acceleration_sensor_feedback_Y > -150)) {
                acceleration_sensor_feedback_Y = -100;
            }
        }

        AS_feedback_bit_shift = (int) (acceleration_sensor_feedback_Y / acceleration_sensor_step_value);
        previous_AS_value = acceleration_sensor_feedback_Y;
        vTaskDelay(100);
    }
}

void projectile_path(uint32_t projectile_column)
{
    for (int row = 27; row >= 0; row--) {
        adafruit_columns[row] |= (1 << projectile_column);

        adafruit_columns[row + 1] &= ~(1 << projectile_column);
        if (row == 0) {
            adafruit_columns[row] &= ~(1 << projectile_column);
        }

        vTaskDelay(10);
    }
}

void fire_a_shot(void * p)
{
    while (1) {
        if (LPC_GPIO0->FIOPIN & (1 << 29)) {
            for (int column = 0; column < 32; column++) {
                if ((*(adafruit_columns_ptr + 28) >> column) & 1) {
                    projectile_path(column);
                    break;
                }
            }
        }
        vTaskDelay(1);
    }
}

void set_leds_in_row_start_end(uint32_t * leds_to_set, uint8_t offset)
{

    LPC_GPIO0->FIOPIN &= ~(1 << 1); // R1
    LPC_GPIO0->FIOPIN &= ~(1 << 0); // B1
    LPC_GPIO0->FIOPIN &= ~(1 << 26); // G1 = o

    LPC_GPIO1->FIOPIN &= ~(1 << 19); //R2
    LPC_GPIO1->FIOPIN &= ~(1 << 20); // B2
    LPC_GPIO1->FIOPIN &= ~(1 << 31); //G2

    for (int k = 0; k < 32; k++) {
        if (((*(leds_to_set + offset) >> k) & 1)) {
            LPC_GPIO0->FIOPIN |= (1 << 0); // B1
            LPC_GPIO0->FIOPIN |= (1 << 26); // G1 = o
            LPC_GPIO0->FIOPIN |= (1 << 1); // R1
        }
        else {
            LPC_GPIO0->FIOPIN &= ~(1 << 0); // B1
            LPC_GPIO0->FIOPIN &= ~(1 << 26); // G1 = o
            LPC_GPIO0->FIOPIN &= ~(1 << 1); // R1
        }
        if (((*(leds_to_set + offset + 16) >> k) & 1)) {
            LPC_GPIO1->FIOPIN |= (1 << 19); //R2
            LPC_GPIO1->FIOPIN |= (1 << 20); // B2
        }
        else {
            LPC_GPIO1->FIOPIN &= ~(1 << 19); //R2
            LPC_GPIO1->FIOPIN &= ~(1 << 20); // B2
        }

        LPC_GPIO2->FIOPIN |= (1 << 6);
        LPC_GPIO2->FIOPIN &= ~(1 << 6);
    }
}

void set_leds_in_row(uint32_t * leds_to_set, uint8_t offset)
{

    LPC_GPIO0->FIOPIN &= ~(1 << 1); // R1
    LPC_GPIO0->FIOPIN &= ~(1 << 0); // B1
    LPC_GPIO0->FIOPIN &= ~(1 << 26); // G1 = o

    LPC_GPIO1->FIOPIN &= ~(1 << 19); //R2
    LPC_GPIO1->FIOPIN &= ~(1 << 20); // B2
    LPC_GPIO1->FIOPIN &= ~(1 << 31); //G2

    // LED Matrix Control
    for (int k = 0; k < 32; k++) {

        // Panel 1 LED Control
        if (offset >= 0 && offset <= 5) {
            if (((*(leds_to_set + offset) >> k) & 1)) {
                LPC_GPIO0->FIOPIN |= (1 << 1); // R1
                LPC_GPIO0->FIOPIN |= (1 << 0); // B1
                LPC_GPIO0->FIOPIN |= (1 << 26); // G1 = o
            }
            else {
                LPC_GPIO0->FIOPIN &= ~(1 << 1); // R1
                LPC_GPIO0->FIOPIN &= ~(1 << 0); // B1
                LPC_GPIO0->FIOPIN &= ~(1 << 26); // G1 = o
            }
        }
        else if (offset > 5 && offset <= 10) {
            if (((*(leds_to_set + offset) >> k) & 1)) {
                LPC_GPIO0->FIOPIN |= (1 << 26); // G1
                LPC_GPIO0->FIOPIN |= (1 << 0); // B1
            }
            else {
                LPC_GPIO0->FIOPIN &= ~(1 << 26); // G1
                LPC_GPIO0->FIOPIN &= ~(1 << 0); // B1
            }
        }
        else if (offset > 10 && offset < 14) {
            if (((*(leds_to_set + offset) >> k) & 1)) {
                LPC_GPIO0->FIOPIN |= (1 << 26); // G1 = o
            }
            else {
                LPC_GPIO0->FIOPIN &= ~(1 << 26); // G1 = o
            }
        }

        // Panel 2 LED Control
        if ((offset + 16 >= 16) && (offset + 16 <= 19)) {
            LPC_GPIO1->FIOPIN &= ~(1 << 20); // B2
            if (((*(leds_to_set + offset + 16) >> k) & 1)) {
                LPC_GPIO1->FIOPIN |= (1 << 19); //R2

            }
            else {
                LPC_GPIO1->FIOPIN &= ~(1 << 19); //R2
            }
        }
        else if ((offset + 16 > 19) && (offset + 16 <= 24)) {
            LPC_GPIO1->FIOPIN &= ~(1 << 19); //R2
            LPC_GPIO1->FIOPIN &= ~(1 << 20); // B2
            if (((*(leds_to_set + offset + 16) >> k) & 1)) {
                LPC_GPIO1->FIOPIN |= (1 << 19); //R2
                LPC_GPIO1->FIOPIN |= (1 << 20); // B2
            }
            else {
                LPC_GPIO1->FIOPIN &= ~(1 << 19); //R2
                LPC_GPIO1->FIOPIN &= ~(1 << 20); // B2
            }
        }
        else if ((offset + 16 > 24) && (offset + 16 <= 31)) {
            LPC_GPIO1->FIOPIN &= ~(1 << 20); // B2
            if (((*(leds_to_set + offset + 16) >> k) & 1)) {
                LPC_GPIO1->FIOPIN |= (1 << 31); //G2
            }
            else {
                LPC_GPIO1->FIOPIN &= ~(1 << 31); //G2
            }
        }

        LPC_GPIO2->FIOPIN |= (1 << 6);
        LPC_GPIO2->FIOPIN &= ~(1 << 6);
    }
}

union ROW_MUX {
    uint8_t Buf;
    struct {
        uint8_t A :1;
        uint8_t B :1;
        uint8_t C :1;
        uint8_t D :1;
        uint8_t E :1; // dummy bit (not used)
        uint8_t F :1; // dummy bit (not used)
        uint8_t G :1; // dummy bit (not used)
        uint8_t H :1; // dummy bit (not used)
    }__attribute__((packed));
} M;

void iterate_through_led_matrix_rows_start_end(void * p)
{
    while (1) {

        for (int i = 0; i <= 15; i = (i + 1) % 16) {

            if (i % 16 == 0) {
                M.Buf = 0;
                i = 0;
            }

            M.Buf += 1;

            LPC_GPIO2->FIOPIN &= ~(1 << 6); // pull clock low
            LPC_GPIO2->FIOPIN &= ~(1 << 7); // pull latch low
            LPC_GPIO2->FIOPIN |= (1 << 5); // set output enable high

            // Rows 0 and 16
            if (M.A == 0 && M.B == 0 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 0);
            }
            // Rows 1 and 17
            else if (M.A == 1 && M.B == 0 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 1);
            }
            // Rows 2 and 18
            else if (M.A == 0 && M.B == 1 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 2);
            }
            // Rows 3 and 19
            else if (M.A == 1 && M.B == 1 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 3);
            }
            // Rows 4 and 20
            else if (M.A == 0 && M.B == 0 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 4);
            }
            // Rows 5 and 21
            else if (M.A == 1 && M.B == 0 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 5);
            }
            // Rows 6 and 22
            else if (M.A == 0 && M.B == 1 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 6);
            }
            // Rows 7 and 23
            else if (M.A == 1 && M.B == 1 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 7);
            }
            // Rows 8 and 24
            else if (M.A == 0 && M.B == 0 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 8);
            }
            // Rows 9 and 25
            else if (M.A == 1 && M.B == 0 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 9);
            }
            // Rows 10 and 26
            else if (M.A == 0 && M.B == 1 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 10);
            }
            // Rows 11 and 27
            else if (M.A == 1 && M.B == 1 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 11);
            }
            // Rows 12 and 28
            else if (M.A == 0 && M.B == 0 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 12);
            }
            // Rows 13 and 29
            else if (M.A == 1 && M.B == 0 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 13);
            }
            // Rows 14 and 30
            else if (M.A == 0 && M.B == 1 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 14);
            }
            // Rows 15 and 31
            else if (M.A == 1 && M.B == 1 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row_start_end(adafruit_columns_ptr, 15);
            }

            LPC_GPIO2->FIOPIN |= (1 << 7); // set latch high
            LPC_GPIO2->FIOPIN &= ~(1 << 5); // set output enable low
            vTaskDelay(1);
        }
    }
}

void iterate_through_led_matrix_rows(void * p)
{
    while (1) {

        for (int i = 0; i <= 15; i = (i + 1) % 16) {

            if (i % 16 == 0) {
                M.Buf = 0;
                i = 0;
            }

            M.Buf += 1;

            LPC_GPIO2->FIOPIN &= ~(1 << 6); // pull clock low
            LPC_GPIO2->FIOPIN &= ~(1 << 7); // pull latch low
            LPC_GPIO2->FIOPIN |= (1 << 5); // set output enable high

            // Rows 0 and 16
            if (M.A == 0 && M.B == 0 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 0);
            }
            // Rows 1 and 17
            else if (M.A == 1 && M.B == 0 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 1);
            }
            // Rows 2 and 18
            else if (M.A == 0 && M.B == 1 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 2);
            }
            // Rows 3 and 19
            else if (M.A == 1 && M.B == 1 && M.C == 0 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 3);
            }
            // Rows 4 and 20
            else if (M.A == 0 && M.B == 0 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 4);
            }
            // Rows 5 and 21
            else if (M.A == 1 && M.B == 0 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 5);
            }
            // Rows 6 and 22
            else if (M.A == 0 && M.B == 1 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 6);
            }
            // Rows 7 and 23
            else if (M.A == 1 && M.B == 1 && M.C == 1 && M.D == 0) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN &= ~(1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 7);
            }
            // Rows 8 and 24
            else if (M.A == 0 && M.B == 0 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 8);
            }
            // Rows 9 and 25
            else if (M.A == 1 && M.B == 0 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 9);
            }
            // Rows 10 and 26
            else if (M.A == 0 && M.B == 1 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 10);
            }
            // Rows 11 and 27
            else if (M.A == 1 && M.B == 1 && M.C == 0 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN &= ~(1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 11);
            }
            // Rows 12 and 28
            else if (M.A == 0 && M.B == 0 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 12);
            }
            // Rows 13 and 29
            else if (M.A == 1 && M.B == 0 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN &= ~(1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 13);
            }
            // Rows 14 and 30
            else if (M.A == 0 && M.B == 1 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN &= ~(1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 14);
            }
            // Rows 15 and 31
            else if (M.A == 1 && M.B == 1 && M.C == 1 && M.D == 1) {
                LPC_GPIO2->FIOPIN |= (1 << 0); // A
                LPC_GPIO1->FIOPIN |= (1 << 22); // B
                LPC_GPIO2->FIOPIN |= (1 << 1); // C
                LPC_GPIO1->FIOPIN |= (1 << 23); // D

                set_leds_in_row(adafruit_columns_ptr, 15);
            }

            LPC_GPIO2->FIOPIN |= (1 << 7); // set latch high
            LPC_GPIO2->FIOPIN &= ~(1 << 5); // set output enable low
            vTaskDelay(1);
        }
    }
}

void draw_start_screen(void * p)
{
    score = 0;
    vTaskSuspend(draw_enemy_1_handle);
    vTaskSuspend(draw_enemy_2_handle);
    vTaskSuspend(draw_enemy_3_handle);
    vTaskSuspend(draw_enemy_4_handle);
    vTaskSuspend(draw_enemy_5_handle);
    vTaskSuspend(player_sprite_handle);
    vTaskSuspend(player_sprite_move_handle);
    vTaskSuspend(detect_a_hit_handle);
    vTaskSuspend(fire_a_shot_handle);
    vTaskSuspend(iterate_through_led_matrix_rows_handle);
    LPC_GPIO1->FIOPIN &= ~(1 << 29); // MP3 signal

    while (1) {
        for (int i = 0; i < 32; i++) {
            adafruit_columns[i] = 0b000000000000000000000000000000000;
        }

        LPC_GPIO0->FIOPIN &= ~(1 << 29);
        if (LPC_GPIO0->FIOPIN & (1 << 29)) {
            playing_game = true;
        }
        while (playing_game) {
            adafruit_columns[0] = 0b010001011101110111011101111011110;
            adafruit_columns[1] = 0b010011010100100101010101001000010;
            adafruit_columns[2] = 0b010101011100100011011101111011110;
            adafruit_columns[3] = 0b011001010100100101010100001010000;
            adafruit_columns[4] = 0b010001010100100101010100001011110;
            adafruit_columns[5] = 0b000000000000000000000000000000000;
            adafruit_columns[6] = 0b000111011101110111011101110100010;
            adafruit_columns[7] = 0b000101010100100101010101010100010;
            adafruit_columns[8] = 0b000011010100100011001101110101010;
            adafruit_columns[9] = 0b000101010100100101010101010110110;
            adafruit_columns[10] = 0b000101011101110101010101010100010;
            adafruit_columns[11] = 0b000000000000000000000000000000000;
            adafruit_columns[12] = 0b000000000000000000000000000000000;
            adafruit_columns[13] = 0b000000000000000000000000000000000;
            adafruit_columns[14] = 0b000000000000000000000000000000000;
            adafruit_columns[15] = 0b000000000000000000000000000000000;
            adafruit_columns[16] = 0b000000000000000000000000000000000;
            adafruit_columns[17] = 0b000000000000000000000000000000000;
            adafruit_columns[18] = 0b000000000000000000000000000000000;
            adafruit_columns[19] = 0b000000000000000000000000000000000;
            adafruit_columns[20] = 0b000000000000000000000000000000000;
            adafruit_columns[21] = 0b000000000000000000000000000000000;
            adafruit_columns[22] = 0b000000000000000000000000000000000;
            adafruit_columns[23] = 0b000000000000000000000000000000000;
            adafruit_columns[24] = 0b000000000000000000000000000000000;
            adafruit_columns[25] = 0b000000000000000000000000000000000;
            adafruit_columns[26] = 0b000000000000000000000000000000000;
            adafruit_columns[27] = 0b000000000000000000000000000000000;
            adafruit_columns[28] = 0b000000000000000000000000000000000;
            adafruit_columns[29] = 0b000000000000000000000000000000000;
            adafruit_columns[30] = 0b000000000000000000000000000000000;
            adafruit_columns[31] = 0b000000000000000000000000000000000;
            delay_ms(3000);
            for (int i = 0; i < 32; i++) {
                adafruit_columns[i] = 0b000000000000000000000000000000000;
            }

            adafruit_columns[5] = 0b000111101110111101111011111011110;
            adafruit_columns[6] = 0b000000100100100101001000100000010;
            adafruit_columns[7] = 0b000111100100011101111000100011110;
            adafruit_columns[8] = 0b000100000100100101001000100010000;
            adafruit_columns[9] = 0b000111100100100101001000100011110;

            delay_ms(1000);
            for (int i = 0; i < 32; i++) {
                adafruit_columns[i] = 0b000000000000000000000000000000000;
            }
            adafruit_columns[5] = 0b000000000000110001101111100000000;
            adafruit_columns[6] = 0b000000000000110011100010000000000;
            adafruit_columns[7] = 0b000000000000110101100010000000000;
            adafruit_columns[8] = 0b000000000000111001100010000000000;
            adafruit_columns[9] = 0b000000000000110001101111100000000;
            delay_ms(1000);
            for (int i = 0; i < 32; i++) {
                adafruit_columns[i] = 0b000000000000000000000000000000000;
            }
            adafruit_columns[5] = 0b000000000000011111100000000000000;
            adafruit_columns[6] = 0b000000000000011000000000000000000;
            adafruit_columns[7] = 0b000000000000011110000000000000000;
            adafruit_columns[8] = 0b000011011011011000000000000000000;
            adafruit_columns[9] = 0b000011011011011111100000000000000;
            delay_ms(1000);
            for (int i = 0; i < 32; i++) {
                adafruit_columns[i] = 0b000000000000000000000000000000000;
            }
            adafruit_columns[5] = 0b000000000000001111100000000000000;
            adafruit_columns[6] = 0b000000000000001000000000000000000;
            adafruit_columns[7] = 0b000000000000001111100000000000000;
            adafruit_columns[8] = 0b000011011011000000100000000000000;
            adafruit_columns[9] = 0b000011011011001111100000000000000;
            delay_ms(1000);
            for (int i = 0; i < 32; i++) {
                adafruit_columns[i] = 0b000000000000000000000000000000000;
            }
            adafruit_columns[5] = 0b000000000000000001110000000000000;
            adafruit_columns[6] = 0b000000000000000001111000000000000;
            adafruit_columns[7] = 0b000000000000000001100100000000000;
            adafruit_columns[8] = 0b000011011011000001100000000000000;
            adafruit_columns[9] = 0b000011011011000111111100000000000;
            delay_ms(1000);
            for (int i = 0; i < 32; i++) {
                adafruit_columns[i] = 0b000000000000000000000000000000000;
            }

            LPC_GPIO1->FIOPIN |= (1 << 29); // MP3 signal
            vTaskResume(iterate_through_led_matrix_rows_handle);
            vTaskResume(draw_enemy_1_handle);
            vTaskResume(draw_enemy_2_handle);
            vTaskResume(draw_enemy_3_handle);
            vTaskResume(draw_enemy_4_handle);
            vTaskResume(draw_enemy_5_handle);
            vTaskResume(player_sprite_handle);
            vTaskResume(player_sprite_move_handle);
            vTaskResume(detect_a_hit_handle);
            vTaskResume(fire_a_shot_handle);
            vTaskResume(game_over_screen);
            vTaskSuspend(iterate_through_led_matrix_rows_start_end_handle);
            vTaskSuspend(NULL);
        }

    }
}

void get_score(uint32_t score);

void draw_game_over_screen(void * p)
{
    delay_ms(60000);
    LPC_GPIO1->FIOPIN &= ~(1 << 29); // MP3 signal
    vTaskSuspend(iterate_through_led_matrix_rows_handle);
    vTaskSuspend(draw_enemy_1_handle);
    vTaskSuspend(draw_enemy_2_handle);
    vTaskSuspend(draw_enemy_3_handle);
    vTaskSuspend(draw_enemy_4_handle);
    vTaskSuspend(draw_enemy_5_handle);
    vTaskSuspend(player_sprite_handle);
    vTaskSuspend(player_sprite_move_handle);
    vTaskSuspend(detect_a_hit_handle);
    vTaskSuspend(fire_a_shot_handle);
    vTaskResume(iterate_through_led_matrix_rows_start_end_handle);

    while (1) {
        for (int i = 0; i < 32; i++) {
            adafruit_columns[i] = 0b000000000000000000000000000000000;
        }

        adafruit_columns[0] = 0b000000011110110001101111110011110;
        adafruit_columns[1] = 0b000000000010111011101100110000010;
        adafruit_columns[2] = 0b000000011110110101101111110111010;
        adafruit_columns[3] = 0b000000000010110001101100110010010;
        adafruit_columns[4] = 0b000000011110110001101100110011110;
        adafruit_columns[5] = 0b000000000000000000000000000000000;
        adafruit_columns[6] = 0b000111101111010000000101111100000;
        adafruit_columns[7] = 0b000100100001001000001001000100000;
        adafruit_columns[8] = 0b000011101111000100010001000100000;
        adafruit_columns[9] = 0b000100100001000010100001000100000;
        adafruit_columns[10] = 0b000100101111000001000001111100000;

        adafruit_columns[16] = 0b000011110011110011110011110011110;
        adafruit_columns[17] = 0b000000010010010010010000010000010;
        adafruit_columns[18] = 0b000001110011110010010000010011110;
        adafruit_columns[19] = 0b000000010001010010010000010010000;
        adafruit_columns[20] = 0b000011110010010011110011110011110;
        adafruit_columns[21] = 0b000000000000000000000000000000000;

        get_score(score);

        for (int i = 23; i < 31; i++) {
            adafruit_columns[i] = reg1[i] | reg2[i] | reg3[i];
        }

        delay_ms(5000);

        for (int i = 0; i < 32; i++) {
            adafruit_columns[i] = 0b000000000000000000000000000000000;
        }

        play_again_flag = true;
    }
}

void get_score(uint32_t score)
{
    h = (int) (score / 100);
    t = (int) ((score % 100) / 10);
    u = (score % 100) - (t * 10);
    switch (h) {
        case 0:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001111100;
            reg1[25] = 0b00000000000000000000000001101100;
            reg1[26] = 0b00000000000000000000000001101100;
            reg1[27] = 0b00000000000000000000000001101100;
            reg1[28] = 0b00000000000000000000000001111100;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;
        case 1:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000000011000;
            reg1[25] = 0b00000000000000000000000000011110;
            reg1[26] = 0b00000000000000000000000000011000;
            reg1[27] = 0b00000000000000000000000000011000;
            reg1[28] = 0b00000000000000000000000001111110;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;
        case 2:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001111100;
            reg1[25] = 0b00000000000000000000000001100000;
            reg1[26] = 0b00000000000000000000000001111100;
            reg1[27] = 0b00000000000000000000000000001100;
            reg1[28] = 0b00000000000000000000000001111100;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;

        case 3:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001111100;
            reg1[25] = 0b00000000000000000000000001100000;
            reg1[26] = 0b0000000000000000000000000111100;
            reg1[27] = 0b00000000000000000000000001100000;
            reg1[28] = 0b00000000000000000000000001111100;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;

        case 4:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001000100;
            reg1[25] = 0b00000000000000000000000001000100;
            reg1[26] = 0b00000000000000000000000001111100;
            reg1[27] = 0b00000000000000000000000001100000;
            reg1[28] = 0b00000000000000000000000001100000;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;

        case 5:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001111100;
            reg1[25] = 0b00000000000000000000000000000100;
            reg1[26] = 0b00000000000000000000000001111100;
            reg1[27] = 0b00000000000000000000000001000000;
            reg1[28] = 0b00000000000000000000000001111100;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;

        case 6:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001111100;
            reg1[25] = 0b00000000000000000000000000000100;
            reg1[26] = 0b00000000000000000000000001111100;
            reg1[27] = 0b00000000000000000000000001000100;
            reg1[28] = 0b00000000000000000000000001111100;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;

        case 7:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000011111100;
            reg1[25] = 0b00000000000000000000000001000100;
            reg1[26] = 0b00000000000000000000000000100000;
            reg1[27] = 0b00000000000000000000000000010000;
            reg1[28] = 0b00000000000000000000000000001000;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;

        case 8:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001111100;
            reg1[25] = 0b00000000000000000000000001000100;
            reg1[26] = 0b00000000000000000000000001111100;
            reg1[27] = 0b00000000000000000000000001000100;
            reg1[28] = 0b00000000000000000000000001111100;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;

        case 9:
            reg1[23] = 0b00000000000000000000000000000000;
            reg1[24] = 0b00000000000000000000000001111100;
            reg1[25] = 0b00000000000000000000000001000100;
            reg1[26] = 0b00000000000000000000000001111100;
            reg1[27] = 0b00000000000000000000000001100000;
            reg1[28] = 0b00000000000000000000000001100000;
            reg1[29] = 0b00000000000000000000000000000000;
            reg1[30] = 0b00000000000000000000000000000000;
            break;
    }
    switch (t) {
        case 0:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000011111000000000;
            reg2[25] = 0b00000000000000000011011000000000;
            reg2[26] = 0b00000000000000000011011000000000;
            reg2[27] = 0b00000000000000000011011000000000;
            reg2[28] = 0b00000000000000000011111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 1:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000001100000000000;
            reg2[25] = 0b00000000000000000001111000000000;
            reg2[26] = 0b00000000000000000001100000000000;
            reg2[27] = 0b00000000000000000001100000000000;
            reg2[28] = 0b00000000000000000111111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 2:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000011111000000000;
            reg2[25] = 0b00000000000000000011000000000000;
            reg2[26] = 0b00000000000000000011111000000000;
            reg2[27] = 0b00000000000000000000011000000000;
            reg2[28] = 0b00000000000000000011111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 3:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000011111000000000;
            reg2[25] = 0b00000000000000000011000000000000;
            reg2[26] = 0b00000000000000000011111000000000;
            reg2[27] = 0b00000000000000000011000000000000;
            reg2[28] = 0b00000000000000000011111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 4:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000010001000000000;
            reg2[25] = 0b00000000000000000010001000000000;
            reg2[26] = 0b00000000000000000011111000000000;
            reg2[27] = 0b00000000000000000011000000000000;
            reg2[28] = 0b00000000000000000011000000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 5:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000011111000000000;
            reg2[25] = 0b00000000000000000000011000000000;
            reg2[26] = 0b00000000000000000011111000000000;
            reg2[27] = 0b00000000000000000011000000000000;
            reg2[28] = 0b00000000000000000011111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 6:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000111111000000000;
            reg2[25] = 0b00000000000000000000011000000000;
            reg2[26] = 0b00000000000000000111111000000000;
            reg2[27] = 0b00000000000000000110011000000000;
            reg2[28] = 0b00000000000000000111111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 7:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000111110000000000;
            reg2[25] = 0b00000000000000000010000000000000;
            reg2[26] = 0b00000000000000000001000000000000;
            reg2[27] = 0b00000000000000000000100000000000;
            reg2[28] = 0b00000000000000000000010000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 8:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000011111000000000;
            reg2[25] = 0b00000000000000000011011000000000;
            reg2[26] = 0b00000000000000000011111000000000;
            reg2[27] = 0b00000000000000000011011000000000;
            reg2[28] = 0b00000000000000000011111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
        case 9:
            reg2[23] = 0b00000000000000000000000000000000;
            reg2[24] = 0b00000000000000000011111000000000;
            reg2[25] = 0b00000000000000000010011000000000;
            reg2[26] = 0b00000000000000000011111000000000;
            reg2[27] = 0b00000000000000000010000000000000;
            reg2[28] = 0b00000000000000000011111000000000;
            reg2[29] = 0b00000000000000000000000000000000;
            reg2[30] = 0b00000000000000000000000000000000;
            break;
    }
    switch (u) {
        case 0:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000001111110000000000000000;
            reg3[25] = 0b00000000001100110000000000000000;
            reg3[26] = 0b00000000001100110000000000000000;
            reg3[27] = 0b00000000001100110000000000000000;
            reg3[28] = 0b00000000001111110000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 1:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000000011000000000000000000;
            reg3[25] = 0b00000000000011110000000000000000;
            reg3[26] = 0b00000000000011000000000000000000;
            reg3[27] = 0b00000000000011000000000000000000;
            reg3[28] = 0b00000000001111110000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 2:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000000111110000000000000000;
            reg3[25] = 0b00000000000110000000000000000000;
            reg3[26] = 0b00000000000111110000000000000000;
            reg3[27] = 0b00000000000000110000000000000000;
            reg3[28] = 0b00000000000111110000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 3:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000001111110000000000000000;
            reg3[25] = 0b00000000001100000000000000000000;
            reg3[26] = 0b00000000001111110000000000000000;
            reg3[27] = 0b00000000001100000000000000000000;
            reg3[28] = 0b00000000001111110000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 4:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000001100110000000000000000;
            reg3[25] = 0b00000000001100110000000000000000;
            reg3[26] = 0b00000000001111110000000000000000;
            reg3[27] = 0b00000000001100000000000000000000;
            reg3[28] = 0b00000000001100000000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 5:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000001111110000000000000000;
            reg3[25] = 0b00000000000000110000000000000000;
            reg3[26] = 0b00000000001111110000000000000000;
            reg3[27] = 0b00000000001100000000000000000000;
            reg3[28] = 0b00000000001111110000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 6:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000001111110000000000000000;
            reg3[25] = 0b00000000000000110000000000000000;
            reg3[26] = 0b00000000001111110000000000000000;
            reg3[27] = 0b00000000001100110000000000000000;
            reg3[28] = 0b00000000001111110000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 7:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000011111100000000000000000;
            reg3[25] = 0b00000000001000000000000000000000;
            reg3[26] = 0b00000000000100000000000000000000;
            reg3[27] = 0b00000000000010000000000000000000;
            reg3[28] = 0b00000000000001000000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 8:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000011111100000000000000000;
            reg3[25] = 0b00000000011001100000000000000000;
            reg3[26] = 0b00000000011111100000000000000000;
            reg3[27] = 0b00000000011001100000000000000000;
            reg3[28] = 0b00000000011111100000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
        case 9:
            reg3[23] = 0b00000000000000000000000000000000;
            reg3[24] = 0b00000000011111100000000000000000;
            reg3[25] = 0b00000000011001100000000000000000;
            reg3[26] = 0b00000000011111100000000000000000;
            reg3[27] = 0b00000000011000000000000000000000;
            reg3[28] = 0b00000000011111100000000000000000;
            reg3[29] = 0b00000000000000000000000000000000;
            reg3[30] = 0b00000000000000000000000000000000;
            break;
    }
}

// begin main
int main(void)
{
    AS.init();

    initialize_LPC_To_LED_Matrix_GPIO_Pins();

    M.Buf = 0; // initialize A B C D to 0

    xTaskCreate(draw_start_screen, (const char*) "draw_start_screen", 512, NULL, 1, &start_screen);
    xTaskCreate(draw_game_over_screen, (const char*) "draw_game_over_screen", 512, NULL, 1, &game_over_screen);

    xTaskCreate(iterate_through_led_matrix_rows_start_end, (const char *) "iterate through led matrix rows start end", 512, NULL, 1, &iterate_through_led_matrix_rows_start_end_handle);
    xTaskCreate(iterate_through_led_matrix_rows, (const char *) "iterate through led matrix rows", 512, NULL, 1, &iterate_through_led_matrix_rows_handle);

    xTaskCreate(draw_enemy_5, (const char*) "draw_enemy_5", 512, NULL, 1, &draw_enemy_5_handle);
    xTaskCreate(draw_enemy_4, (const char*) "draw_enemy_4", 512, NULL, 1, &draw_enemy_4_handle);
    xTaskCreate(draw_enemy_3, (const char*) "draw_enemy_3", 512, NULL, 1, &draw_enemy_3_handle);
    xTaskCreate(draw_enemy_2, (const char*) "draw_enemy_2", 512, NULL, 1, &draw_enemy_2_handle);
    xTaskCreate(draw_enemy_1, (const char*) "draw_enemy_1", 512, NULL, 1, &draw_enemy_1_handle);

    xTaskCreate(draw_player_sprite, (const char*) "draw_player_sprite", 512, NULL, 1, &player_sprite_handle);
    xTaskCreate(move_player_sprite, (const char*) "move_player_sprite", 512, NULL, 1, &player_sprite_move_handle);

    xTaskCreate(detect_a_hit, (const char*) "detect a hit", 512, NULL, 1, &detect_a_hit_handle);
    xTaskCreate(fire_a_shot, (const char*) "fire_a_shot", 512, NULL, 1, &fire_a_shot_handle);

    vTaskStartScheduler();

    return 0;
}
// end main
