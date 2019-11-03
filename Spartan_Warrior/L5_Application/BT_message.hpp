/*
 * BT_message.hpp
 *
 *  Created on: May 5, 2018
 *      Author: Sucheta
 */

#ifndef BT_MESSAGE_HPP_
#define BT_MESSAGE_HPP_

#include<iostream>
#include <stdlib.h>
#include <string>

using namespace std;

/**
 *      Commands Received from BT :
 *      *0          ->STOP              Returns 0
 *      *1          ->START             Returns 1
 *      *2          ->PAUSE             Returns 2
 *      *3          ->Volume ++         Returns 3
 *      *4          ->Volume --         Returns 4
 *      *5\n        ->Fast Forward      Returns 5
 *      SONG_NAME   ->Play that song    Returns 9
 *      #song_name\n
 *
 *      ERROR                           Returns -1
 */
int validate_command(string temp);

/*
 *      We can store no more than 99 songs in the list
 */
int validate_BT_message(char msg[]);

/*
 * When the command to change song is received,
 * change the file name being fetched and reset offset to 0 so
 * that the new song plays from beginning
 */
void change_song();

#endif /* BT_MESSAGE_HPP_ */
