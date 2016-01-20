#ifndef CHATROOM_DEFINITIONS_H
#define CHATROOM_DEFINITIONS_H


#pragma warning(disable : 4996)
#pragma comment(lib,"Ws2_32.lib")

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

#include <string.h>

#define EQUAL_STR(x,y) (!strcmp((x),(y)))
#define INIT_STR(x,y)  (strcpy((x),(y)))
#define MAX_CLIENT_NAME 20
#define MAX_OBJECT_NAME 20
#define MAX_BUFFER_SIZE 512
#define MAX_CLIENTS_NUMBER 4
#define ADDRESS "127.0.0.1"
#define SERVER_ID 0
#define CLIENT_ID 1
#define QUIT_REQUEST "quit\n"


typedef enum { _return_OK = 0, _return_Failed = 1 }return_Value;

typedef HANDLE SEMAPHORE;
typedef HANDLE MUTEX;
typedef HANDLE THREAD;
typedef struct _chatroom_Object* CHATROOM_OBJECT;

typedef struct _socket_object{
	SOCKET socket;
	struct sockaddr_in service;
	char Address[MAX_OBJECT_NAME];
	int Port;
}*SOCKET_OBJ;


typedef struct _clientProperties{
	char clientName[MAX_CLIENT_NAME];
	THREAD CommunicationThread;
	THREAD UI_Thread;
	SEMAPHORE UserEnterTextSemaphore;
	SEMAPHORE EngingDoneWithUserMassageSemaphore;
	SEMAPHORE IncomingMsgFromServerSemaphore;
}*ClientProperties;

typedef struct _serverProperties{
	int indexClient;//don't trust that when broardcast isnot lock
	SEMAPHORE NewClientSemaphore;
	MUTEX BoardcastMutex;
	SOCKET clientsChannel[MAX_CLIENTS_NUMBER + 1];
	THREAD CommunicationThread[MAX_CLIENTS_NUMBER];
	MUTEX mutexNumberOfConnectedClients;
	int* numOfConnectedClients;
}*ServerProperties;


typedef struct _chatroomObjectProperties{
	ClientProperties clientProperties;
	ServerProperties serverProperties;
}*chatroomObjectProperties;

struct _chatroom_Object{
	SOCKET_OBJ socket_obj;
	int chatroom_type;
	int exit;
	FILE* logFile;
	chatroomObjectProperties uniqeProperties;
	char UI_Buffer_input[MAX_BUFFER_SIZE];//to logFile and screen
	char communicatinBuffer_input[MAX_BUFFER_SIZE];//to send and recieve 
	char UI_Buffer_output[MAX_BUFFER_SIZE];//to logFile and screen
	char communicatinBuffer_output[MAX_BUFFER_SIZE];//to send and recieve 

};


#endif