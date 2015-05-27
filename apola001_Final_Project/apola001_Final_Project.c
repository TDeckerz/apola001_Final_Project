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


unsigned char string[] = "Congratulations!";
unsigned char keypadVal_g;
unsigned char isHeld_g = 0; //checks whether the button is being hel
unsigned char col_g = 1;
unsigned char row_g = 1;


enum SM1_States { SM1_init, SM1_wait};

//gets the value of the keypad...
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
            transmit_data_rows(row_g);
            transmit_data_cols(row_g);
            break;
        default:
            break;
    }
    return state;
}

int main(void){
    
    DDRA = 0xFF; PORTA = 0x00; // LCD Control Lines
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x0F; PORTC = 0xF0; // Keypad Input
    DDRD = 0xFF; PORTD = 0x00; // LCD Data lines
    
    unsigned long int SMTick1_calc = 100;
    unsigned long int SMTick2_calc = 100;

    unsigned long int tmpGCD = 1;
    tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
    
    unsigned long int GCD = tmpGCD;
     
    unsigned long int SMTick1_period = SMTick1_calc/GCD;
    unsigned long int SMTick2_period = SMTick2_calc/GCD;
    
    //Array of task pointers...
    static task task1, task2;
    task *tasks[] = {&task1, &task2};
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