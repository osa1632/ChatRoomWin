#include "UI_Thread.h"

return_Value convertStrToInt(char * str, int * parseStr){
	*parseStr = atoi(str);
	return _return_OK;
}

return_Value convertIntToStr(int  number,char convertNumber[MAX_BUFFER_SIZE] ){
	char temp_convertNumber[MAX_BUFFER_SIZE];
	itoa(number,temp_convertNumber,10);
	INIT_STR(convertNumber, temp_convertNumber);
	return _return_OK;
}

return_Value openFile(FILE** file, const char name[], const char mode[]){
	if ((*file = fopen(name, mode)) == NULL){
		return _return_Failed;
	}
	return _return_OK;
}


return_Value writeToLogfileAndScreen(CHATROOM_OBJECT chatroom_object, char prefix[], char msg[], char suffix[]){
	sprintf(chatroom_object->UI_Buffer_output, "%s%s%s", prefix, msg, suffix);
	fprintf(stdout, "%s", chatroom_object->UI_Buffer_output);
	fprintf(chatroom_object->logFile, "%s", chatroom_object->UI_Buffer_output);
	return _return_OK;
}


int UI_Thread(CHATROOM_OBJECT chatroom_object){
	int exitFlag = 0;
	while (!exitFlag){
		fgets(chatroom_object->UI_Buffer_input, MAX_BUFFER_SIZE, stdin);
		ReleaseSemaphore(chatroom_object->uniqeProperties->clientProperties->UserEnterTextSemaphore, 1, NULL);
		if (WaitForSingleObject(chatroom_object->uniqeProperties->clientProperties->EngingDoneWithUserMassageSemaphore, INFINITE) != WAIT_OBJECT_0){
			exitFlag = -1;
		}
		if (EQUAL_STR(chatroom_object->UI_Buffer_input, QUIT_REQUEST)){
			exitFlag = 1;
		}
	}
	return (exitFlag == 1) ? 0 : 1;
}