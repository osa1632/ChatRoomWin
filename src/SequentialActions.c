#include "SequentialActions.h"


return_Value _tmain_SequentialActions(int argc, char *argv[]){
	return_Value status = _return_OK;
	struct _chatroom_Object _chatroom_obj;	
	CHATROOM_OBJECT chatroom_object = &_chatroom_obj;
	struct _serverProperties _server_properties;
	struct _clientProperties  _client_properties;
	struct _socket_object _socket_obj;
	struct _chatroomObjectProperties _chatroom_object_properties;
	_chatroom_object_properties.clientProperties = &_client_properties;
	_chatroom_object_properties.serverProperties = &_server_properties;
	_chatroom_obj.uniqeProperties = &_chatroom_object_properties;
	_chatroom_obj.socket_obj = &_socket_obj;
	chatroom_object->chatroom_type=-1;
	CHECK_STATUS(initPart(argc, argv, chatroom_object));
	CHECK_STATUS(engingePart(chatroom_object));
	return (status==_return_OK && chatroom_object->exit==0)?endPart(chatroom_object):_return_Failed;
}

return_Value initPart(int argc, char *argv[], CHATROOM_OBJECT chatroom_object ){
	return_Value status = _return_OK;
	CHECK_STATUS( checkArgs(argc, argv[1],chatroom_object));
	CHECK_STATUS(initChatroomObj(argv, chatroom_object));
	return status;
}

return_Value checkArgs(int argc, char operationMode[], CHATROOM_OBJECT chatroom_object){
	if (!(!(EQUAL_STR(operationMode, "server") && argc >= 4 && (chatroom_object->chatroom_type = SERVER_ID) == SERVER_ID) ^
		!(EQUAL_STR(operationMode, "client") && argc >=5 && (chatroom_object->chatroom_type = CLIENT_ID) == CLIENT_ID)))
	{
		return _return_Failed;
	}	
	return _return_OK;
}

return_Value initChatroomObj(char *argv[], CHATROOM_OBJECT chatroom_object){
	return_Value status = _return_OK;
	CHECK_STATUS(openFile(&(chatroom_object->logFile), argv[2], "wt"));
	INIT_STR(chatroom_object->socket_obj->Address, ADDRESS);
	CHECK_STATUS(convertStrToInt(argv[3], &(chatroom_object->socket_obj->Port)));

	CHECK_STATUS(initSemaphores_MutexesAndHandels(chatroom_object));


	if (chatroom_object->chatroom_type == CLIENT_ID){
		INIT_STR(chatroom_object->uniqeProperties->clientProperties->clientName, argv[4]);
	}

	CHECK_STATUS(initSocket(chatroom_object));


	return status;
}


return_Value initSemaphores_MutexesAndHandels(CHATROOM_OBJECT chatroom_object){
	if (chatroom_object->chatroom_type == CLIENT_ID){
		chatroom_object->uniqeProperties->clientProperties->EngingDoneWithUserMassageSemaphore=CreateSemaphore(NULL,1,1,NULL);
		chatroom_object->uniqeProperties->clientProperties->UserEnterTextSemaphore= CreateSemaphore(NULL, 0, 1, NULL);
		chatroom_object->uniqeProperties->clientProperties->IncomingMsgFromServerSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
		chatroom_object->uniqeProperties->clientProperties->CommunicationThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CommunicationThread, (LPVOID)chatroom_object, 1, NULL);
		chatroom_object->uniqeProperties->clientProperties->UI_Thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UI_Thread, (LPVOID)chatroom_object, 1, NULL);
		if (chatroom_object->uniqeProperties->clientProperties->UserEnterTextSemaphore == NULL ||
			chatroom_object->uniqeProperties->clientProperties->IncomingMsgFromServerSemaphore == NULL ||
			chatroom_object->uniqeProperties->clientProperties->CommunicationThread == NULL ||
			chatroom_object->uniqeProperties->clientProperties->UI_Thread == NULL){
			writeToLogfileAndScreen(chatroom_object,"Error:", "Problem in Handle Allocation!","\n");
			return _return_Failed;
		}
	}
	if (chatroom_object->chatroom_type == SERVER_ID){
		chatroom_object->uniqeProperties->serverProperties->NewClientSemaphore = CreateSemaphore(NULL, MAX_CLIENTS_NUMBER,MAX_CLIENTS_NUMBER, NULL);
		chatroom_object->uniqeProperties->serverProperties->BoardcastMutex = CreateMutex(NULL,FALSE,NULL);
		chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients = CreateMutex(NULL, FALSE, NULL);
		
		if (chatroom_object->uniqeProperties->serverProperties->BoardcastMutex==NULL||
			chatroom_object->uniqeProperties->serverProperties->NewClientSemaphore == NULL||
			chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients==NULL){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Problem in Handle Allocation!", "\n");
			return _return_Failed;
		}

		
	}
	return _return_OK;
}



return_Value engingePart(CHATROOM_OBJECT chatroom_object){
	return_Value status=_return_OK;
	chatroom_object->exit = 0;
	CHECK_STATUS((chatroom_object->chatroom_type == SERVER_ID) ? engingePartForServer(chatroom_object) : engingePartForClient(chatroom_object));
	
	return status;
}

return_Value engingePartForServer(CHATROOM_OBJECT chatroom_object){
	int newSocketIndex = -1, exitFromThisPart = 0;
	int numOfConnectedClients = 0;
	int i = 0;
	int isFirstRun = 1;


	if (WaitForSingleObject(
		chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients,
		INFINITE) != WAIT_OBJECT_0){
		writeToLogfileAndScreen(chatroom_object, "Error:", "Wait Mutex Failed", "\n");
		return _return_Failed;
	}
	chatroom_object->uniqeProperties->serverProperties->numOfConnectedClients = &numOfConnectedClients;
	for (i = 0; i < MAX_CLIENTS_NUMBER; i++){
		chatroom_object->uniqeProperties->serverProperties->clientsChannel[i] = INVALID_SOCKET;
	}

	if (!ReleaseMutex(chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients)){
		writeToLogfileAndScreen(chatroom_object, "Error:", "Release Mutex Failed", "\n");
		return _return_Failed;
	}
	while (!exitFromThisPart){
		if (WaitForSingleObject(chatroom_object->uniqeProperties->serverProperties->NewClientSemaphore, INFINITE) != WAIT_OBJECT_0){
			writeToLogfileAndScreen(chatroom_object, "Error: ", "Problem with wait function", "\n");
			exitFromThisPart = -1;
			break;
		}
		if (*(chatroom_object->uniqeProperties->serverProperties->numOfConnectedClients) == 0
			&&! isFirstRun){
			exitFromThisPart = 1;
			break;
		}
		isFirstRun = 0;
		for (i = 0; i < MAX_CLIENTS_NUMBER; i++){
			if (chatroom_object->uniqeProperties->serverProperties->clientsChannel[i] == INVALID_SOCKET){
				if (INVALID_SOCKET == (chatroom_object->uniqeProperties->serverProperties->clientsChannel[i] =
					accept(chatroom_object->socket_obj->socket, NULL, NULL))){
					if (WSAGetLastError() == 10004 ){
						writeToLogfileAndScreen(chatroom_object, "", "No Clients Connected-exiting", "\n");
						return _return_OK;
					}
					writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in accept function", "\n");
					exitFromThisPart = -1;
				}
				else{
					break;
				}
			}
		}
		if (exitFromThisPart == -1){
			break;
		}

		if (WaitForSingleObject(
			chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients
			, INFINITE) != WAIT_OBJECT_0){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Wait Mutex Failed", "\n");
			exitFromThisPart = -1;
			break;
		}

		*(chatroom_object->uniqeProperties->serverProperties->numOfConnectedClients) += 1;
		if (!ReleaseMutex(chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients)){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Release Mutex Failed", "\n");
			exitFromThisPart = -1;
			break;
		}
		chatroom_object->uniqeProperties->serverProperties->indexClient = i;

		if (NULL == (chatroom_object->uniqeProperties->serverProperties->CommunicationThread[newSocketIndex] =
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CommunicationThread, (LPVOID)chatroom_object, 1, NULL))){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in CreateThread function", "\n");
			exitFromThisPart = -1;
			break;
		}
		

		exitFromThisPart |= chatroom_object->exit;
	}
	return (exitFromThisPart == 1) ? _return_OK : _return_Failed;
}

return_Value engingePartForClient(CHATROOM_OBJECT chatroom_object){
	int operationToDo = -1, exitFromThisPart =0;
	SEMAPHORE chatroom_object_semaphres[2]; 
	chatroom_object_semaphres[0] = chatroom_object->uniqeProperties->clientProperties->UserEnterTextSemaphore;
	chatroom_object_semaphres[1] = chatroom_object->uniqeProperties->clientProperties->IncomingMsgFromServerSemaphore;

	while (!exitFromThisPart){

		
		operationToDo = WaitForMultipleObjects(2, chatroom_object_semaphres, FALSE, INFINITE) - WAIT_OBJECT_0;
		if (operationToDo == 0){
			if (EQUAL_STR(chatroom_object->UI_Buffer_input, QUIT_REQUEST)){
				exitFromThisPart = 1;
			}
			INIT_STR(chatroom_object->communicatinBuffer_output, chatroom_object->UI_Buffer_input);

			if (sendMsg(chatroom_object->socket_obj->socket, chatroom_object->communicatinBuffer_output) == _return_Failed){
				writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in send function", "\n");
				exitFromThisPart = -1;
			}
			if (writeToLogfileAndScreen(chatroom_object, "send: ", chatroom_object->communicatinBuffer_output, "") != _return_OK){
				exitFromThisPart = -1;
			}
			INIT_STR(chatroom_object->communicatinBuffer_output, "");

		}
		if (operationToDo == 1){
			if (writeToLogfileAndScreen(chatroom_object, "", chatroom_object->communicatinBuffer_input, "") != _return_OK){
				exitFromThisPart = -1;
			}
			INIT_STR(chatroom_object->communicatinBuffer_input, "");
		}

		if (!ReleaseSemaphore(chatroom_object->uniqeProperties->clientProperties->EngingDoneWithUserMassageSemaphore,
			1, NULL) ){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in ReleaseSemaphore function", "\n");
			exitFromThisPart = -1;
		}

		
		exitFromThisPart |= chatroom_object->exit;
	}

	return (exitFromThisPart==1)?_return_OK:_return_Failed;
}

return_Value endPart(CHATROOM_OBJECT chatroom_object){
	return_Value status = _return_OK;
	if (chatroom_object->chatroom_type == SERVER_ID){
		if (!CloseHandle( chatroom_object->uniqeProperties->serverProperties->NewClientSemaphore)||
			!CloseHandle(chatroom_object->uniqeProperties->serverProperties->BoardcastMutex)){
			status = _return_Failed;
			writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in CloseHandle function", "\n");
		}
	}
	if (chatroom_object->chatroom_type == CLIENT_ID){
		if (!CloseHandle(chatroom_object->uniqeProperties->clientProperties->UserEnterTextSemaphore) ||
			!CloseHandle(chatroom_object->uniqeProperties->clientProperties->EngingDoneWithUserMassageSemaphore) ||
			!CloseHandle(chatroom_object->uniqeProperties->clientProperties->IncomingMsgFromServerSemaphore)){
			status = _return_Failed;
			writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in CloseHandle function", "\n");
		}

	}
	if (!fclose(chatroom_object->logFile)){
		status= _return_Failed;
	}
	
	return status;
}


