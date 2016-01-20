#ifndef UI_THREAD_H
#define UI_THREAD_H

#include <stdio.h>

#include "ChatRoom_Definitions.h"

int UI_Thread(CHATROOM_OBJECT chatroom_object);

return_Value convertStrToInt(char * str, int * parseStr);
return_Value convertIntToStr(int  number, char  convertNumber[MAX_BUFFER_SIZE]);
return_Value openFile(FILE** file, const char name[], const char mode[]);

return_Value writeToLogfileAndScreen(CHATROOM_OBJECT chatroom_object, char prefix[], char msg[], char suffix[]);
#endif