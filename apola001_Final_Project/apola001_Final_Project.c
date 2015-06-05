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
#include <avr/eeprom.h>

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

unsigned char* char_to_string(unsigned char val, unsigned char string[])
{
    char const nums[] = "0123456789";
    unsigned char* num = string;
    int divisor = 100;
    int i;
    for(i = 0; i < 3; ++i){
        num[i] = nums[val / divisor];
        val = val%divisor;
        divisor = divisor/10;
    }
    num[i] = '\0';
    return num;
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
unsigned char off_button_g = 0; //The power switch, pin A2
unsigned char start_button_g = 0; // The start button, pin A3
unsigned char reset_button_g = 0; // The pause button, pin A4
unsigned char left_button_g = 0; // The left movement button, pin A5
unsigned char right_button_g = 0; // The right movement button, pin A6

//Row/Col variables, track enemy, player, shots.
unsigned char col_g = 1;
unsigned char row_g = 1;

//Player and enemy positions
unsigned char player_pos_g = 0x08; //The player is locked to row 1, keep track of current col

unsigned char bullet_x_g = 0; //There is a maximum of 5 bullets for the player at any time
unsigned char bullet_y_g = 0; // If I shift a char it becomes 0
unsigned char shoot_button_g = 0;
unsigned char bullet_live = 0; //If the bullet is on the board.

unsigned char enemy_col[3] = {0}; //The enemy rows
unsigned char enemy_row[3] = {0}; // The enemies columns
unsigned char move_left = 0;
unsigned char move_right = 0;
unsigned char enemy_or_cols_g = 0;

//game state, score, displays
unsigned char paused_g = 0;
unsigned char to_reset_g = 0;
unsigned char game_over_g= 0;
unsigned char to_display_g;

//Score stuff. The values, the 'strings' to display and to output the value.
unsigned char score = 0;
unsigned char string_score[5] = {0};
unsigned char to_display_g;
unsigned char score_line[] = "Score: ";

unsigned char game_over_low[] = "GAME OVER";
unsigned char game_over_high[] = "New Hi-Score!";
unsigned char hi_score_string[4] = {0};
unsigned char display_low_or_high = 0; //choose the appropriate string in a game over.

//Pause menu strings
unsigned char current_high[] = "Cur Hi-Score:";
unsigned char more_string[] = "More";
unsigned char second_pause[] = "Speed Enemies?";
unsigned char second_pause2[] = "Fire to toggle between 3";
unsigned char scroll_down = 25; // Arrow pointing downwards;

//EEPROM values.
uint8_t EEMEM ScoreChar = 1;
uint8_t high_score;


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
            high_score = eeprom_read_byte(&ScoreChar);
            break;
        case SM1_wait:
            if(game_over_g){
                if(score > high_score){
                    display_low_or_high = 1;
                    high_score = (uint8_t) score;
                    eeprom_update_byte(&ScoreChar, high_score);
                }
                else{
                    display_low_or_high = 0;
                }
            }

            if((paused_g == 1) && shoot_button_g){
                high_score = 0;
                eeprom_update_byte(&ScoreChar, high_score);
            }
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
            row_g = 1;
            col_g = ~(player_pos_g);
            transmit_data_rows(row_g);
            transmit_data_cols(col_g);
            delay();
            transmit_data_rows(bullet_y_g);
            transmit_data_cols(~bullet_x_g);
            delay();
            transmit_data_rows(enemy_row[0]);
            transmit_data_cols(~enemy_col[0]);
            delay();
            transmit_data_rows(enemy_row[1]);
            transmit_data_cols(~enemy_col[1]);
            delay();
            transmit_data_rows(enemy_row[2]);
            transmit_data_cols(~enemy_col[2]);
            delay();
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
            if(paused_g){
                break;
            }
            if(bullet_live){
                break;
            }
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
            if(paused_g){
                state = SM3_no_press;
                break;
            }
            if(!left_button_g && !right_button_g){
                state = SM3_no_press;
            }
            break;
        case SM3_right:
            if(paused_g){
                state = SM3_no_press;
                break;
            }
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
            reset_button_g = ~PINA & 0x10;
            left_button_g = ~PINA & 0x20;
            right_button_g = ~PINA & 0x40;
            shoot_button_g = ~PINA & 0x80;
            
            if(to_reset_g){
                to_reset_g = 0;
            }
            // If the game is not paused, reset is pressed and it was not already being reset
            if(reset_button_g && !to_reset_g){ 
                game_over_g = 0; // set game over to 0
                to_reset_g = 1; // enable the reset variable.
            }

            // If the game is not over, and start is pressed, pause
            if(start_button_g && !game_over_g){
                if(!paused_g){
                    paused_g = 1;
                }
            }
            
            //If the game is paused and the player moves, unpause.
            if((paused_g) && (left_button_g || right_button_g)){
                paused_g = 0;
            }
            if((paused_g == 1) && start_button_g){
                paused_g = 2;
            }
            break;
        default:
            break;
    } // End state actions
    return state;
}

//Updates the bullets movement.
enum SM5_States{SM5_init, SM5_track_shot, SM5_paused};

int SMTick5(int state){
    switch(state){
        case SM5_init:
            state = SM5_track_shot;
            break;
        case SM5_track_shot:
            if(paused_g){
                state = SM5_paused;
            }
            if(reset_button_g){
                state = SM5_init;
            }
            state = SM5_track_shot;
            break;
        case SM5_paused:
            if(!paused_g){
                state = SM5_track_shot;
            }
            if(!game_over_g){
                state = SM5_init;
            }
            break;
        default:
            state = SM5_init;
            break;
    }
    switch(state){
        case SM5_init:
            break;
        case SM5_track_shot:
            if(paused_g || game_over_g){
                break;
            }
            if(!bullet_live && shoot_button_g){ //If there is no bu
                bullet_live = 1;
                bullet_x_g = player_pos_g;
                bullet_y_g = 1;
            }
            if(bullet_x_g && bullet_y_g){ // If the bullet has a valid position.
                bullet_y_g = bullet_y_g << 1;
            }
            if(bullet_y_g == 0){
                bullet_x_g = 0;
                bullet_y_g = 0;
                bullet_live = 0;
            }
            //Since the bullet is faster than the enemies, collision is checked here.
            to_display_g = 0;
            if(bullet_y_g == enemy_row[0]){
                if((bullet_x_g & enemy_col[0]) != 0){
                    enemy_col[0] = enemy_col[0] ^ (char) bullet_x_g;
                    bullet_live = 0;
                    bullet_x_g = 0;
                    bullet_y_g = 0;
                    to_display_g = 1;
                    score += 1;
                }
            }
            if(bullet_y_g == enemy_row[1]){
                if((bullet_x_g & enemy_col[1]) != 0){
                    enemy_col[1] = enemy_col[1] ^ (char) bullet_x_g;
                    bullet_live = 0;
                    bullet_x_g = 0;
                    bullet_y_g = 0;
                    to_display_g = 1;
                    score += 5;
                }
            }
            if(bullet_y_g == enemy_row[2]){
                if((bullet_x_g & enemy_col[2]) != 0){
                    enemy_col[2] = enemy_col[2] ^ (char) bullet_x_g;
                    bullet_live = 0;
                    bullet_x_g = 0;
                    bullet_y_g = 0;
                    to_display_g = 1;
                    score += 10;
                }
            }
            if(reset_button_g){
                bullet_live = 0;
                bullet_x_g = 0;
                bullet_y_g = 0;
            }
            break;
        case SM5_paused:
            break;
        default:
            break;
    }
    return state;
};

enum SM6_States {SM6_init, SM6_spawn, SM6_move, SM6_pause, SM6_game_over};

int SMTick6(int state){
    switch(state){
        case SM6_init:
            state = SM6_spawn;
            break;
        case SM6_spawn:
            enemy_row[0] = 32; enemy_row[1] = 64; enemy_row[2] = 128;
            enemy_col[0] = 252; enemy_col[1] = 252; enemy_col[2] = 252;
            state = SM6_move;
            if(paused_g){
                state = SM6_pause;
            }
            break;
        case SM6_move:
            state = SM6_move;
            if(reset_button_g){
                state = SM6_spawn;
            }
            if(paused_g){
                state = SM6_pause;
            }
            if(game_over_g){
                state = SM6_game_over;
            }
            break;
        case SM6_pause:
            state = SM6_move;
            if(paused_g){
                state = SM6_pause;
            }
            if(to_reset_g){
                state = SM6_spawn;
            }
            break;
        case SM6_game_over:
            if(to_reset_g){
                state = SM6_spawn;
            }
            break;
        default:
            state = SM6_init;
            break;
    }

    switch (state){
        case SM6_init:
            break;
        case SM6_spawn:
            score = 0;
            move_left = 0; // Begin shifting left.
            move_right = 1;
            break;
        case SM6_move:
            enemy_or_cols_g = enemy_col[0] | enemy_col[1] | enemy_col[2];
            if(!enemy_or_cols_g){ //If there are no more enemies on the field.
                game_over_g = 1;
                break;
            }
            //alien movement;
            if(move_right){
                if(!(enemy_or_cols_g & 0x01)){ // If there are no aliens on the rightmost column...
                    enemy_col[0] = enemy_col[0] >> 1;
                    enemy_col[1] = enemy_col[1] >> 1;
                    enemy_col[2] = enemy_col[2] >> 1;
                }
                else{ // If there are aliens on the rightmost column
                    move_right = 0;
                    move_left = 1; // begin moving left
                    enemy_row[0] = enemy_row[0] >> 1;
                    enemy_row[1] = enemy_row[1] >> 1;
                    enemy_row[2] = enemy_row[2] >> 1; // and move the aliens up a row.
                }
            }
            else if(move_left){
                if(!(enemy_or_cols_g & 0x80)){ // If there are no enemies on the leftmost column
                    enemy_col[0] = enemy_col[0] << 1;
                    enemy_col[1] = enemy_col[1] << 1;
                    enemy_col[2] = enemy_col[2] << 1;
                }
                else{ // Otherwise move forward.
                    move_right = 1;
                    move_left = 0; // begin moving left
                    enemy_row[0] = enemy_row[0] >> 1;
                    enemy_row[1] = enemy_row[1] >> 1;
                    enemy_row[2] = enemy_row[2] >> 1; // and move the aliens up a row.
                }
            }
            if(enemy_row[0] == 1){
                if(enemy_col[0] != 0){
                    game_over_g= 1;
                }
            }
            if(enemy_row[1] == 1){
                if(enemy_col[1] != 0){
                    game_over_g= 1;
                }
            }
            if(enemy_row[2] == 1){
                if(enemy_col[2] != 0){
                    game_over_g= 1;
                }
            }
            break;
        case SM6_pause:
            break;
        case SM6_game_over:
            break;
        default:
            break;
    }
    return state;
}

enum SM7_States{SM7_init, SM7_wait, SM7_update, SM7_display_off, SM7_game_over, SM7_paused1,SM7_paused2 };

int SMTick7(int state){
    switch(state){
        case SM7_init:
            state = SM7_wait;
            break;
        case SM7_wait:
            if(to_reset_g){
                state = SM7_init;
                LCD_ClearScreen();
            }
            if(to_display_g){
                state = SM7_update;    
            }
            if(paused_g == 1){
                char_to_string((char)high_score, hi_score_string);
                LCD_DisplayString(1, current_high);
                LCD_DisplayString_NoClear(14, hi_score_string);
                LCD_DisplayString_NoClear(17, more_string);
                LCD_Cursor(25);
                LCD_WriteData(scroll_down);
                state = SM7_paused1;
            }            
            
            if(game_over_g){
                 if(!display_low_or_high){
                    LCD_DisplayString(1, game_over_low);
                }
                else{
                    LCD_DisplayString(1, game_over_high);
                    char_to_string(score, string_score);
                    LCD_DisplayString_NoClear(17, string_score);
                }
                state = SM7_game_over;
            }
            break;
        case SM7_update:
            state = SM7_wait;
            break;
        case SM7_display_off:
            state = SM7_display_off;
            break;
        case SM7_game_over:
            if(to_reset_g){
                state = SM7_init;
                LCD_ClearScreen();
            }
            break;
        case SM7_paused1:
            if(!paused_g){
                state = SM7_init;
            }
            if(paused_g == 2){
                state = SM7_paused2;
                
            }
            break;
        case SM7_paused2:
            if(!paused_g){
                state = SM7_init;
            }
            break;
        default:
            state = SM7_init;
            break;
    }
    switch(state){
        case SM7_init:
            char_to_string(score, string_score);
            LCD_DisplayString(1, score_line);
            LCD_DisplayString_NoClear(17, string_score);
            break;
        case SM7_wait:
            break;
        case SM7_update:
            char_to_string(score, string_score);
            LCD_DisplayString_NoClear(17, string_score);
            to_display_g = 0;
            break;
        case SM7_display_off:
            break;
        case SM7_game_over:
            break;
        case SM7_paused1:
            break;
        case SM7_paused2:
            break;
        default:
            break;
    }
    return state;
}

int main(void){

    DDRA = 0x03; PORTA = 0xFC; // LCD Control Lines
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x0F; PORTC = 0xF0; // Keypad Input
    DDRD = 0xFF; PORTD = 0x00; // LCD Data lines
    
    unsigned long int SMTick1_calc = 10;
    unsigned long int SMTick2_calc = 1;
    unsigned long int SMTick3_calc = 50;
    unsigned long int SMTick4_calc = 50;
    unsigned long int SMTick5_calc = 50;
    unsigned long int SMTick6_calc = 500;
    unsigned long int SMTick7_calc = 10;

    unsigned long int tmpGCD = 1;
    tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
    tmpGCD = findGCD(SMTick3_calc, tmpGCD);
    tmpGCD = findGCD(SMTick4_calc, tmpGCD);
    tmpGCD = findGCD(SMTick5_calc, tmpGCD);
    tmpGCD = findGCD(SMTick6_calc, tmpGCD);
    tmpGCD = findGCD(SMTick7_calc, tmpGCD);
    
    unsigned long int GCD = tmpGCD;
     
    unsigned long int SMTick1_period = SMTick1_calc/GCD;
    unsigned long int SMTick2_period = SMTick2_calc/GCD;
    unsigned long int SMTick3_period = SMTick3_calc/GCD;
    unsigned long int SMTick4_period = SMTick4_calc/GCD;
    unsigned long int SMTick5_period = SMTick5_calc/GCD;
    unsigned long int SMTick6_period = SMTick6_calc/GCD;
    unsigned long int SMTick7_period = SMTick7_calc/GCD;

    //Array of task pointers...
    static task task1, task2, task3, task4, task5, task6, task7;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6, &task7};
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
    
    task5.state = -1;
    task5.period = SMTick5_period;
    task5.elapsedTime = SMTick5_period;
    task5.TickFct = &SMTick5;

    task6.state = -1;
    task6.period = SMTick6_period;
    task6.elapsedTime = SMTick6_period;
    task6.TickFct = &SMTick6;

    task7.state = -1;
    task7.period = SMTick7_period;
    task7.elapsedTime = SMTick7_period;
    task7.TickFct = &SMTick7;

    TimerSet(GCD);
    LCD_init();
    TimerOn();
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