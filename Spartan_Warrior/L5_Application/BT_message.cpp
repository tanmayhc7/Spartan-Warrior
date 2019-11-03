/*
 * BT_message.c
 *
 *  Created on: May 5, 2018
 *      Author: Sucheta
 */

#include "printf_lib.h"
#include "BT_message.hpp"
#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include "tasks.hpp"
#include "VS1053B.hpp"
#include "printf_lib.h"

using namespace std;

extern char SONG_NAME[50];
static char new_song_name[50];


int validate_BT_message(char msg[])
{
    int i = 0;
    bool song_validated = false;
    int start_stop =0;
    string temp = "";

    //Play song command comes is the form
    //  #song_number\n
    if (msg[i] == '#') {
        i++;
        while (msg[i] != '\n') {
            temp += msg[i];
            strcpy(new_song_name,temp.c_str());
            u0_dbg_printf("%c ", msg[i]);
            i++;
        }

        temp = "";
        return 9; // received song number
        i++;
    }
    //start stop
    else if (msg[i] == '*') {

        i++;
        while (msg[i] != '\n') {
            temp += msg[i];
            i++;
        }

        start_stop = validate_command(temp);
        //u0_dbg_printf("command validated = %d",song_validated);
        temp = "";
        return start_stop; //start stop command
        i++;
    }
    if (song_validated) return true;
    else
        return false;
}


int validate_command(string temp)
{
    int command = atoi(temp.c_str());
    u0_dbg_printf("command = %d\n",command);
    int ret =0;
    if (command == 0) {
       ret = 10; //stop
    }
    else if (command == 1) {
        ret = 1; //play
    }
    else if (command == 2) {
        ret = 2; //pause
    }
    else if (command == 3) {
        ret = 3; //increase volume
    }
    else if (command == 4) {
        ret = 4; //decrease volume
    }
    else if (command == 5) {
        ret = 5; //fast forward
    }
    else if (command == 9) {
        ret = 9; //change song
    }
    return ret;
}


void change_song()
{
    vTaskSuspend(playSongTaskHandle);
    strcpy(SONG_NAME,new_song_name);
    song_offset = 0;
    vTaskResume(playSongTaskHandle);
}


