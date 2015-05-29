/* Partner 1 Name & E-mail: Andy Polanco apola001@ucr.edu
 * Partner 2 Name & E-mail:
 * Lab Section: 022
 * Assignment: Lab 7 Excercise 4
 * 
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 */


#include "/includes/io.c"
#include "/includes/bit.h"
#include "/includes/keypad.c"
#include "/includes/timer.h"
#include <avr/io.h>

unsigned long int findGCD(unsigned long a,
                          unsigned long b)
{
    unsigned long int c;
    while(1){
        c = a%b;
        if(c == 0){
            return b;
        }
        a = b;
        b = c;
    }
    return 0;
}

void transmit_data_cols(unsigned char data) {
    int i;
    for (i = 0; i < 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTB = 0x08;
        // set SER = next bit of data to be sent.
        PORTB |= ((data >> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTB |= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTB |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTB = 0x00;
}

void transmit_data_rows(unsigned char data) {
    int i;
    for (i = 0; i < 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTC = 0x08;
        // set SER = next bit of data to be sent.
        PORTC |= ((data >> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTC |= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTC |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTC = 0x00;
}

typedef struct task {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct)(int);
} task;


//Button Variables
unsigned char player_pos_g = 1; //The player is locked to row 1, keep track of current col
unsigned char off_button_g = 0; //The power switch, pin A2
unsigned char start_button_g = 0; // The start button, pin A3
unsigned char pause_button_g = 0; // The pause button, pin A4
unsigned char left_button_g = 0; // The left movement button, pin A5
unsigned char right_button_g = 0; // The right movement button, pin A6

//Row/Col variables, track enemy, player, shots.
unsigned char enemy_rows_g = 0;
unsigned char enemy_cols_g = 0;
unsigned char col_g = 1;
unsigned char row_g = 1;

enum SM1_States { SM1_init, SM1_wait};

int SMTick1(int state){
    switch(state){
        case SM1_init:
            state = SM1_wait;
            break;
         case SM1_wait:
             state = SM1_wait;
            break;
        default:
            state = SM1_init;
            break;
    }
    switch(state){
        case SM1_init:
            break;
        case SM1_wait:

            break;
        default:
             break;
    }
    return state;
}

enum SM2_States { SM2_init, SM2_display };

int SMTick2(int state){
    switch(state){
        case SM2_init:
            state = SM2_display;
            state = SM2_display;
            break;
        default:
            state = SM2_init;
            break;
    }
    switch (state){
        case SM2_init:
            break;
        case SM2_display:
            row_g = 0x01 & enemy_rows_g;
            col_g = ~(player_pos_g & enemy_cols_g);
            transmit_data_rows(row_g);
            transmit_data_cols(row_g);
            break;
        default:
            break;
    }
    return state;
}

enum SM3_States{SM3_init, SM3_no_press, SM3_left, SM3_right};

int SMTick3(int state){
    switch(state){
        case SM3_init:
            state = SM3_no_press;
            break;
        case SM3_no_press:
            if(left_button_g){
                state = SM3_left;
                if(player_pos_g < 128){
                    player_pos_g = player_pos_g << 1;
                }
            }
            else if(right_button_g){
                state = SM3_right;
                if(player_pos_g > 1){
                    player_pos_g = player_pos_g >> 1;
                }
            }
            break;
        case SM3_left:
            if(!left_button_g && !right_button_g){
                state = SM3_no_press;
            }
            break;
        case SM3_right:
            if(!left_button_g && !right_button_g){
                state = SM3_no_press;
            }
            break;
        default:
            state = SM3_init;
            break;
    }// End state transtions.
    
    switch(state){ //Begin State actions, in this case empty.
        case SM3_init:
            break;
        case SM3_no_press:
            break;
        case SM3_left:
            break;
        case SM3_right:
            break;
        default:
            break;
    } // End state actions.
    return state;
}

enum SM4_States {SM4_init, SM4_get_inputs};

int SMTick4(int state){
    switch(state){ //Begin state transitions for SM4, grabbing inputs.
        case SM4_init:
            state = SM4_get_inputs;
            break;
        case SM4_get_inputs:
            state = SM4_get_inputs;
            break;
        default:
            state = SM4_init;
            break;
    } //End state transtions.
    
    switch(state){ //Begin state actions.
        case SM4_init:
            break;
        case SM4_get_inputs:
            off_button_g = ~PINA & 0x04;
            start_button_g = ~PINA & 0x08;
            pause_button_g = ~PINA & 0x10;
            left_button_g = ~PINA & 0x20;
            right_button_g = ~PINA & 0x40;
            //shoot_button_g = ~PINA & 0x80;
            break;
        default:
            break;
    } // End state actions
    return state;
}

enum SM5_States{SM5_init, SM5_LED_off, SM5_LED_on};

int main(void){
    
    DDRA = 0xFF; PORTA = 0x00; // LCD Control Lines
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x0F; PORTC = 0xF0; // Keypad Input
    DDRD = 0xFF; PORTD = 0x00; // LCD Data lines
    
    unsigned long int SMTick1_calc = 75;
    unsigned long int SMTick2_calc = 75;
    unsigned long int SMTick3_calc = 50;
    unsigned long int SMTick4_calc = 50;
    unsigned long int SMTick5_calc = 50;

    unsigned long int tmpGCD = 1;
    tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
    tmpGCD = findGCD(SMTick3_calc, tmpGCD);
    tmpGCD = findGCD(SMTick4_calc, tmpGCD);
    tmpGCD = findGCD(SMTick5_calc, tmpGCD);
    
    unsigned long int GCD = tmpGCD;
     
    unsigned long int SMTick1_period = SMTick1_calc/GCD;
    unsigned long int SMTick2_period = SMTick2_calc/GCD;
    unsigned long int SMTick3_period = SMTick3_calc/GCD;
    unsigned long int SMTick4_period = SMTick4_calc/GCD;
    
    //Array of task pointers...
    static task task1, task2, task3, task4;
    task *tasks[] = {&task1, &task2, &task3, &task4};
    const unsigned short numTasks =
    sizeof(tasks)/sizeof(task*);
    
    //Initializing the task components
    task1.state = -1;
    task1.period = SMTick1_period;
    task1.elapsedTime = SMTick1_period;
    task1.TickFct = &SMTick1;
    
    task2.state = -1;
    task2.period = SMTick2_period;
    task2.elapsedTime = SMTick2_period;
    task2.TickFct = &SMTick2;
    
    task3.state = -1;
    task3.period = SMTick3_period;
    task3.elapsedTime = SMTick3_period;
    task3.TickFct = &SMTick3;
    
    task4.state = -1;
    task4.period = SMTick4_period;
    task4.elapsedTime = SMTick4_period;
    task4.TickFct = &SMTick4;
    
    TimerSet(GCD);
    //5LCD_init();
    TimerOn();
    //transmit_data_cols(255);
    unsigned short i;
    
    while(1){
        for(i = 0; i < numTasks; ++i){
            if(tasks[i]->elapsedTime ==
               tasks[i]->period){
                   tasks[i]->state = 
                        tasks[i]->TickFct(tasks[i]->state);
                   tasks[i]->elapsedTime = 0;
              }
               tasks[i]->elapsedTime += 1;
        }
         while(!TimerFlag);
        TimerFlag = 0;
    }
    //Error
    return 0;
}