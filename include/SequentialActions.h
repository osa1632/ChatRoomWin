#ifndef SEQUENTIAL_ACTIONS_H
#define SEQUENTIAL_ACTIONS_H
#include "ChatRoom_Definitions.h"
#include "CommunicationThread.h"
#include "UI_Thread.h"

int UI_Thread(CHATROOM_OBJECT);

return_Value _tmain_SequentialActions(int argc, char *argv[]);

//##################	INTERNAL FUNCTION     ###############################//
#define CHECK_STATUS(x) if (status!=_return_Failed){{status=x;}}

return_Value initPart(int argc, char *argv[], CHATROOM_OBJECT chatroom_object);
return_Value checkArgs(int argc, char operationMode[], CHATROOM_OBJECT outputOperationMode);
return_Value initChatroomObj(char *argv[], CHATROOM_OBJECT chatroom_object);
return_Value initSocket(CHATROOM_OBJECT chatroom_object);
return_Value initSemaphores_MutexesAndHandels(CHATROOM_OBJECT chatroom_object);
return_Value engingePart(CHATROOM_OBJECT chatroom_object);
return_Value endPart(CHATROOM_OBJECT chatroom_object);
return_Value engingePartForServer(CHATROOM_OBJECT chatroom_object);
return_Value engingePartForClient(CHATROOM_OBJECT chatroom_object);


//###########################################################################//

#endif

