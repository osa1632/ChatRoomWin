#ifndef COMMUNICATION_THREAD_H
#define COMMUNICATION_THREAD_H

#include "ChatRoom_Definitions.h"
#include "UI_Thread.h"

/*
int CommunicationThread(CHATROOM_OBJECT);
int CommunicationThreadForServer(CHATROOM_OBJECT chatroom_object);
int CommunicationThreadForClient(CHATROOM_OBJECT chatroom_object);
return_Value createSocketByOperationMode(CHATROOM_OBJECT chatroom_object, const int opertationMode);
return_Value sendMsg(SOCKET recipient, char msg[]);
return_Value initSocket(CHATROOM_OBJECT chatroom_object);
//###################			Internal Fubction			###################
return_Value initService(struct sockaddr_in *service, char address[], int port);


int recvForHub(CHATROOM_OBJECT chatroom_object);
int premitaryPartOfCommunicationThreadForServer(CHATROOM_OBJECT chatroom_object);

int mainlyPartOfCommunicationThreadForServer(CHATROOM_OBJECT chatroom_object);

int endPartOfCommuticationThreadForServer(CHATROOM_OBJECT chatroom_object);
return_Value sendForAllClientsExcept_socketIndex(CHATROOM_OBJECT chatroom_object);
int recvUntilEndOfLine(SOCKET s, char buffer[]);
//############################################################################
*/



return_Value initService(struct sockaddr_in* service, char address[], int port);

return_Value createSocketByOperationMode(CHATROOM_OBJECT chatroom_object, const int opertationMode);

int CommunicationThread(LPVOID arg);

int CommunicationThreadForServer(CHATROOM_OBJECT chatroom_object);

int recvForHub(CHATROOM_OBJECT chatroom_object, int clientIndex);
int premitaryPartOfCommunicationThreadForServer(CHATROOM_OBJECT chatroom_object, int clientIndex, char clientName[MAX_CLIENT_NAME]);

int mainlyPartOfCommunicationThreadForServer(CHATROOM_OBJECT chatroom_object, int clientIndex, char clientName[MAX_CLIENT_NAME]);

int endPartOfCommuticationThreadForServer(CHATROOM_OBJECT chatroom_object, int clientIndex);
return_Value sendForAllClientsExcept_socketIndex(CHATROOM_OBJECT chatroom_object, int clientIndex);

int CommunicationThreadForClient(CHATROOM_OBJECT chatroom_object);

return_Value sendMsg(SOCKET recipient, char msg[]);


return_Value initSocket(CHATROOM_OBJECT chatroom_object);

int recvUntilEndOfLine(SOCKET s, char buffer[]);


#endif