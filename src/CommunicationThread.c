#include "CommunicationThread.h"

 return_Value initService(struct sockaddr_in* service,char address[],int port){
	
	service->sin_family = AF_INET;
	service->sin_addr.s_addr = inet_addr(address);
	service->sin_port = htons((u_short)port);
	return _return_OK;
}
 
 return_Value createSocketByOperationMode(CHATROOM_OBJECT chatroom_object, const int opertationMode){
	int resSocketOp;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		writeToLogfileAndScreen(chatroom_object, "Error:", "wsa function failed", "\n");
		return _return_Failed;
	}	
	chatroom_object->socket_obj->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (chatroom_object->socket_obj->socket == INVALID_SOCKET) {
		//printf("socket function failed with error: %ld\n", WSAGetLastError());
		writeToLogfileAndScreen(chatroom_object, "Error:", "socket function failed", "\n");
		WSACleanup();
		return _return_Failed;
	}
	if (initService(&(chatroom_object->socket_obj->service), chatroom_object->socket_obj->Address, chatroom_object->socket_obj->Port) == _return_Failed){
		return _return_Failed;
	}
	if (opertationMode == SERVER_ID){
		resSocketOp = bind(chatroom_object->socket_obj->socket, (SOCKADDR*)&(chatroom_object->socket_obj->service), sizeof(chatroom_object->socket_obj->service));
		if (resSocketOp == SOCKET_ERROR){
			writeToLogfileAndScreen(chatroom_object,"Error:","Unable to bind to server!","\n");
			WSACleanup();
			return _return_Failed;
		}
		resSocketOp = listen(chatroom_object->socket_obj->socket, 10);
		if (resSocketOp == SOCKET_ERROR){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Unable to listen to server!", "\n");
			WSACleanup();
			return _return_Failed;
		}
		return _return_OK;
	}
	if (opertationMode == CLIENT_ID) {
		resSocketOp = connect(chatroom_object->socket_obj->socket, (SOCKADDR*)&(chatroom_object->socket_obj->service), sizeof(chatroom_object->socket_obj->service));
		if (resSocketOp == SOCKET_ERROR){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Unable to connect server", "\n");
			WSACleanup();
			return _return_Failed;
		}
		if (sendMsg(chatroom_object->socket_obj->socket, chatroom_object->communicatinBuffer_output) == _return_Failed){
			writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in send function", "\n");
		}
		return _return_OK;
	}
	return _return_Failed;
}

 int CommunicationThread(LPVOID arg){
		 CHATROOM_OBJECT chatroom_object = (CHATROOM_OBJECT)arg;
	  return (chatroom_object->exit= (chatroom_object->chatroom_type == CLIENT_ID) ? 
		 CommunicationThreadForClient(chatroom_object) : 
		 CommunicationThreadForServer(chatroom_object));
 }

 int CommunicationThreadForServer(CHATROOM_OBJECT chatroom_object){
	 int clientIndex = chatroom_object->uniqeProperties->serverProperties->indexClient;
	 char clientName[MAX_CLIENT_NAME] = "";

	 if (!premitaryPartOfCommunicationThreadForServer(chatroom_object,clientIndex,clientName)||
		 !mainlyPartOfCommunicationThreadForServer(chatroom_object,clientIndex,clientName)||
		 !endPartOfCommuticationThreadForServer(chatroom_object,clientIndex)){
		 return 1;
	 }
	 return 0 ;
 }

 int recvForHub(CHATROOM_OBJECT chatroom_object,int clientIndex){
	 switch (recvUntilEndOfLine(chatroom_object->uniqeProperties->serverProperties->clientsChannel[clientIndex],
		 chatroom_object->communicatinBuffer_input)){
	 case 0:
		 break;
	 case 1: //for both, recv null msg or error
	 case -1:
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in recv function", "\n");
		 return FALSE;
	 default:
		 writeToLogfileAndScreen(chatroom_object, "Error:", "UNknown error in internal function (recvUntilEndOfLine,CommunicationThread)", "\n");
		 return FALSE;
	 }
	 return TRUE;
 }
 int premitaryPartOfCommunicationThreadForServer(CHATROOM_OBJECT chatroom_object,int clientIndex,char clientName[MAX_CLIENT_NAME]){
	 INIT_STR(chatroom_object->communicatinBuffer_input, "");
	 INIT_STR(chatroom_object->communicatinBuffer_output, "");
	 if (!recvForHub(chatroom_object,clientIndex)){
		 return FALSE;
	 }
	 if (WaitForSingleObject(chatroom_object->uniqeProperties->serverProperties->BoardcastMutex, INFINITE) != WAIT_OBJECT_0){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in WaitForSingleObject function", "\n");
		 return FALSE;
	 }
	 if (sscanf(chatroom_object->communicatinBuffer_input, "username=%s\n",clientName) < 1){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect first massge from client", "\n");
		 return FALSE;
	 }
	 sprintf(chatroom_object->communicatinBuffer_output, "New client entered the chat room:%s\n",clientName);
	 if (_return_OK != sendForAllClientsExcept_socketIndex(chatroom_object,clientIndex)){
		 return FALSE;
	 }
	 
	 if (!(ReleaseMutex(chatroom_object->uniqeProperties->serverProperties->BoardcastMutex))){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in ReleaseMutex function", "\n");
		 return FALSE;
	 }
	 
	 return TRUE;
 }

 int mainlyPartOfCommunicationThreadForServer(CHATROOM_OBJECT chatroom_object,int clientIndex,char clientName[MAX_CLIENT_NAME]){
	 int exitFlag = 0;
	 while (!exitFlag){
		 INIT_STR(chatroom_object->communicatinBuffer_input, "");
		 INIT_STR(chatroom_object->communicatinBuffer_output, "");
		 if (!recvForHub(chatroom_object,clientIndex)){
			exitFlag=-1;
		 }
		 if (EQUAL_STR(chatroom_object->communicatinBuffer_input, QUIT_REQUEST)){
			 sprintf(chatroom_object->communicatinBuffer_output, "%s left the chat room\n", clientName);
			 exitFlag = 1;
		 }
		 else{
			 sprintf(chatroom_object->communicatinBuffer_output, "%s:%s\n",clientName,
				 chatroom_object->communicatinBuffer_input);
		 }
		 if (WaitForSingleObject(chatroom_object->uniqeProperties->serverProperties->BoardcastMutex, INFINITE) != WAIT_OBJECT_0){
			 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in WaitForSingleObject function", "\n");
			 return FALSE;
		 }
		 if (_return_OK != sendForAllClientsExcept_socketIndex(chatroom_object, clientIndex)){
			 exitFlag = -1;
		 }
		
		 
		 if (!exitFlag && !(ReleaseMutex(chatroom_object->uniqeProperties->serverProperties->BoardcastMutex))){
			 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in ReleaseMutex function", "\n");
			 exitFlag = -1;
		 }
	 }
	 return (exitFlag == 1) ? TRUE : FALSE;
 }

 int endPartOfCommuticationThreadForServer(CHATROOM_OBJECT chatroom_object,int clientIndex){
	 int status=TRUE;
	 
	 if (closesocket(chatroom_object->uniqeProperties->serverProperties->clientsChannel[clientIndex])){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Cannot close socket", "\n");
		 status=FALSE;
	 }
	 chatroom_object->uniqeProperties->serverProperties->clientsChannel[clientIndex] = INVALID_SOCKET;
	 if (WaitForSingleObject(
		 chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients,INFINITE) 
		 != WAIT_OBJECT_0){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Wait Mutex Failed", "\n");
		 status= FALSE;
	 }
	 *(chatroom_object->uniqeProperties->serverProperties->numOfConnectedClients)-=1;
	 if (*(chatroom_object->uniqeProperties->serverProperties->numOfConnectedClients) == 0 &&
		 (closesocket(chatroom_object->socket_obj->socket))	&&
		 ((chatroom_object->exit = 1)==1)){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Cannot Close Socket", "\n");
		 status= FALSE;
	 }
	 if (!ReleaseMutex(
		 chatroom_object->uniqeProperties->serverProperties->mutexNumberOfConnectedClients)){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Release Mutex Failed", "\n");
		 status= FALSE;
	 }

	 if (!ReleaseSemaphore(
		 chatroom_object->uniqeProperties->serverProperties->NewClientSemaphore, 1, NULL)){
			
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Cannot ReleaseSemaphore socket", "\n");
		 status= FALSE;
	 }
	 if (*(chatroom_object->uniqeProperties->serverProperties->numOfConnectedClients) == 0){
		 writeToLogfileAndScreen(chatroom_object, "", "No Clients Connected-exiting", "\n");
		 status= FALSE;
	 }

	 if (!ReleaseMutex(chatroom_object->uniqeProperties->serverProperties->BoardcastMutex)){
		 writeToLogfileAndScreen(chatroom_object, "Error:", "Cannot ReleaseMutex socket", "\n");
		 status= FALSE;
	 }
	 
	 return status;
 }
 return_Value sendForAllClientsExcept_socketIndex(CHATROOM_OBJECT chatroom_object,int clientIndex){
	 int i = 0;
	 for (i = 0; i < MAX_CLIENTS_NUMBER; i++){
		 if (i != clientIndex && 
			 chatroom_object->uniqeProperties->serverProperties->clientsChannel[i]!=INVALID_SOCKET&&
			 sendMsg(chatroom_object->uniqeProperties->serverProperties->clientsChannel[i],
			 chatroom_object->communicatinBuffer_output) != _return_OK){
			 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in send function", "\n");
			 return _return_Failed;
		 }
	 }

	 return _return_OK;
 }

 int CommunicationThreadForClient(CHATROOM_OBJECT chatroom_object){
	 int exitFlag = 0;
	 while (!exitFlag){
		 if (WaitForSingleObject(chatroom_object->uniqeProperties->clientProperties->EngingDoneWithUserMassageSemaphore, INFINITE) != WAIT_OBJECT_0){
			 exitFlag = -1;
			 break;
		 }
		 switch (recvUntilEndOfLine(chatroom_object->socket_obj->socket, chatroom_object->communicatinBuffer_input)){
		 case 0:
			 break;
		 case 1:
			 writeToLogfileAndScreen(chatroom_object, "", "close socket. will be quit", "\n");
			 break;
		 case -1:
			 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in recv function", "\n");
			 break;
		 default:
			 writeToLogfileAndScreen(chatroom_object, "Error:", "UNknown error in internal function (recvUntilEndOfLine,CommunicationThread)", "\n");
			 break;
		 }

		 if (!ReleaseSemaphore(chatroom_object->uniqeProperties->clientProperties->IncomingMsgFromServerSemaphore, 1, NULL)){
			 writeToLogfileAndScreen(chatroom_object, "Error:", "Unexpect error in ReleaseMutex function", "\n");
			 exitFlag= -1;
		 }	
	 }
	 chatroom_object->exit = 1;
	 return (exitFlag == 1) ? 0:1;
 }
 
 return_Value sendMsg(SOCKET recipient, char msg[]){
	 if (send(recipient, msg, strlen(msg), 0) <= 0){
		 return _return_Failed;
	 }
	 return _return_OK;
 }


 return_Value initSocket(CHATROOM_OBJECT chatroom_object){
	 return_Value status = _return_OK;
	 if (chatroom_object->chatroom_type == CLIENT_ID){
		 writeToLogfileAndScreen(chatroom_object, "username=", chatroom_object->uniqeProperties->clientProperties->clientName, "\n");
		 INIT_STR(chatroom_object->communicatinBuffer_output, chatroom_object->UI_Buffer_output);
	 }
	 status =  createSocketByOperationMode(chatroom_object, chatroom_object->chatroom_type);
	 if (chatroom_object->chatroom_type == CLIENT_ID){
		 char portStr[MAX_BUFFER_SIZE] = "";
		 convertIntToStr(chatroom_object->socket_obj->Port, portStr);
		 if (status == _return_Failed){
			 writeToLogfileAndScreen(chatroom_object, "Failed Connecting to server on port ", portStr, "\n");
		 }
		 else{
			 writeToLogfileAndScreen(chatroom_object, "Connected to server on port ", portStr, "\n");
		 }
	 }
	 
	 INIT_STR(chatroom_object->communicatinBuffer_input, "");
	 INIT_STR(chatroom_object->communicatinBuffer_output, "");
	 INIT_STR(chatroom_object->UI_Buffer_input, "");
	 INIT_STR(chatroom_object->UI_Buffer_output, "");

	 return status;
 }

 int recvUntilEndOfLine(SOCKET s, char buffer[]){
	 char *pointerToEndOfInput = NULL,temp_buffer[MAX_BUFFER_SIZE]; 
	 int sizeOfRecv = -1;
	 while (pointerToEndOfInput == NULL){
		  sizeOfRecv = recv(s, temp_buffer, MAX_BUFFER_SIZE, 0);
		 if (sizeOfRecv == 0){
			 return 1;
		 }
		 if (sizeOfRecv < 0){
			 printf("may  %d\n",WSAGetLastError());
			 return -1;
		 }
		 pointerToEndOfInput = strchr(temp_buffer, '\n');
		 if (pointerToEndOfInput != NULL){
			 *(pointerToEndOfInput + 1) = '\0';
		 }
		 sprintf(buffer, "%s%s", buffer, temp_buffer);
	 }
	 return 0;
 }