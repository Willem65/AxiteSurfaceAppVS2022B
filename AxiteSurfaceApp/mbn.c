/****************************************************************************
	**
	** Copyright (C) 2009 D&R Electronica Weesp B.V. All rights reserved.
	**
	** This file is part of the Axum/MambaNet digital mixing system.
	**
	** This file may be used under the terms of the GNU General Public
	** License version 2.0 as published by the Free Software Foundation
	** and appearing in the file LICENSE.GPL included in the packaging of
	** this file.
	**
	** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
	** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
	**
****************************************************************************/

/* General project-wide TODO list (in addition to `grep TODO *.c`)
	*  - Add more H/W interfaces:
	*    > Serial line?
	*  - Test/port to OS X?
	*  - Test suite?
	*  - Buffering of outgoing packets (to make all mbn* calls non-blocking)
*/

#define MBN_VARARG
#define _XOPEN_SOURCE 500

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include "mbn.h"
#include "address.h"
#include "codec.h"
#include "object.h"

/* sleep() */
#ifdef MBNP_mingw
	# include <windows.h>
	# define sleep(x) Sleep(x*1000)
	#else
	# include <unistd.h>
#endif

#define MMTYPE_SIZE(t, s)\
(t == MBN_DATATYPE_OCTETS || t == MBN_DATATYPE_BITS ? MBN_DATATYPE_UINT : t),\
(t == MBN_DATATYPE_OCTETS || t == MBN_DATATYPE_BITS ? 1 : s)

#define MMTYPE(t)\
(t == MBN_DATATYPE_OCTETS || t == MBN_DATATYPE_BITS ? MBN_DATATYPE_UINT : t)



int mbnhandlers = 0;

char versionString[256];

/* thread that keeps track of messages requiring an acknowledge reply,
* and retries the message after a timeout (of one second, currently) */
void *msgqueue_thread(void *arg) {
	struct mbn_handler *mbn = (struct mbn_handler *) arg;
	struct mbn_msgqueue *q, *last, *tmp;
	#ifndef MBNP_mingw
		struct timeval delay;
		int RetVal;
	#endif
	
	mbn->msgqueue_run = 1;
	
	/* this is the only thread that can free() items from the msgqueue list,
	* so we don't have to lock while reading from it */
	while(1) {
		for(q=last=mbn->queue; q!=NULL; ) {
			/* Remove item from the list */
			if(q->retries == -1 || q->retries++ >= MBN_ACKNOWLEDGE_RETRIES) {
				/* send callback if the message timed out */
				if(q->retries >= 0 && mbn->cb_AcknowledgeTimeout != NULL)
				mbn->cb_AcknowledgeTimeout(mbn, &(q->msg));
				/* remove item from the queue */
				LCK();
				if(last == mbn->queue) {
					mbn->queue = last = q->next;
				} else
				last->next = q->next;
				tmp = q;
				q = q->next;
				free_message(&(tmp->msg));
				free(tmp);
				ULCK();
				continue;
			}
			/* Wait a sec if < 1. */
			if(q->retries > 1) {
				/* No reply yet, let's try again */
				mbnSendMessage(mbn, &(q->msg), MBN_SEND_NOCREATE | MBN_SEND_FORCEID);
			}
			last = q;
			q = q->next;
		}
		
		pthread_testcancel();
		#ifdef MBNP_mingw
			sleep(1);
			#else
			delay.tv_sec = 1;
			delay.tv_usec = 0;
			RetVal =  1;
			while (RetVal) {
				RetVal = select(0, NULL, NULL, NULL, &delay);
			}
		#endif
	}
}


struct mbn_handler * MBN_EXPORT mbnInit(struct mbn_node_info *node, struct mbn_object *objects, struct mbn_interface *itf, char *err) {
	struct mbn_handler *mbn;
	struct mbn_object *obj;
	int i, l;
	
	// #ifdef PTW32_STATIC_LIB
	// if(!mbnhandlers++)
    // pthread_win32_process_attach_np();
	// #endif
	
	if(itf == NULL) {
		sprintf(err, "No interface specified");
		return NULL;
	}
	
	#ifdef MBN_MANUFACTURERID
		if(node->ManufacturerID != 0xFFFF && node->ManufacturerID != MBN_MANUFACTURERID) {
			sprintf(err, "This library has been built to only allow ManufacturerID %d", MBN_MANUFACTURERID);
			return NULL;
		}
	#endif
	
	mbn = (struct mbn_handler *) calloc(1, sizeof(struct mbn_handler));
	memcpy((void *)&(mbn->node), (void *)node, sizeof(struct mbn_node_info));
	mbn->node.Services &= 0x7F; /* turn off validated bit */
	mbn->itf = itf;
	itf->mbn = mbn;
	
	/* pad descriptions and name with zero and clear some other things */
	l = strlen(mbn->node.Description);
	if(l < 64)
    memset((void *)&(mbn->node.Description[l]), 0, 64-l);
	l = strlen(mbn->node.Name);
	if(l < 32)
    memset((void *)&(mbn->node.Name[l]), 0, 32-l);
	
	/* create a copy of the objects, and make some small changes for later use */
	if(objects) {
		mbn->objects = (struct mbn_object *) malloc(mbn->node.NumberOfObjects*sizeof(struct mbn_object));
		memcpy((void *)mbn->objects, (void *)objects, mbn->node.NumberOfObjects*sizeof(struct mbn_object));
		for(i=0;i<mbn->node.NumberOfObjects;i++) {
			obj = &(mbn->objects[i]);
			if(objects[i].SensorSize > 0) {
				copy_datatype(MMTYPE_SIZE(objects[i].SensorType, objects[i].SensorSize), &(objects[i].SensorMin), &(mbn->objects[i].SensorMin));
				copy_datatype(MMTYPE_SIZE(objects[i].SensorType, objects[i].SensorSize), &(objects[i].SensorMax), &(mbn->objects[i].SensorMax));
				copy_datatype(objects[i].SensorType, objects[i].SensorSize, &(objects[i].SensorData), &(mbn->objects[i].SensorData));
			}
			if(objects[i].ActuatorSize > 0) {
				copy_datatype(MMTYPE_SIZE(objects[i].ActuatorType, objects[i].ActuatorSize), &(objects[i].ActuatorMin), &(mbn->objects[i].ActuatorMin));
				copy_datatype(MMTYPE_SIZE(objects[i].ActuatorType, objects[i].ActuatorSize), &(objects[i].ActuatorMax), &(mbn->objects[i].ActuatorMax));
				copy_datatype(MMTYPE_SIZE(objects[i].ActuatorType, objects[i].ActuatorSize), &(objects[i].ActuatorDefault), &(mbn->objects[i].ActuatorDefault));
				copy_datatype(objects[i].ActuatorType, objects[i].ActuatorSize, &(objects[i].ActuatorData), &(mbn->objects[i].ActuatorData));
			}
			mbn->objects[i].changed = mbn->objects[i].timeout = 0;
			l = strlen(mbn->objects[i].Description);
			if(l < 32)
			memset((void *)&(mbn->objects[i].Description[l]), 0, 32-l);
			mbn->objects[i].Services = mbn->objects[i].SensorType != MBN_DATATYPE_NODATA ? 0x03 : 0x00;
		}
	} else
    mbn->node.NumberOfObjects = 0;
	
	/* init and allocate some pthread objects */
	mbn->mbn_mutex = malloc(sizeof(pthread_mutex_t));
	mbn->timeout_thread = malloc(sizeof(pthread_t));
	mbn->throttle_thread = malloc(sizeof(pthread_t));
	mbn->msgqueue_thread = malloc(sizeof(pthread_t));
	pthread_mutex_init((pthread_mutex_t *) mbn->mbn_mutex, NULL);
	
	/* initialize address list */
	init_addresses(mbn);
	
	/* create threads to keep track of timeouts */
	if(    (i = pthread_create((pthread_t *)mbn->timeout_thread,  NULL, node_timeout_thread, (void *) mbn)) != 0
		|| (i = pthread_create((pthread_t *)mbn->throttle_thread, NULL, throttle_thread,     (void *) mbn)) != 0
		|| (i = pthread_create((pthread_t *)mbn->msgqueue_thread, NULL, msgqueue_thread,     (void *) mbn)) != 0) {
		sprintf("Can't create thread: %s (%d)", strerror(i), i);
		free(mbn);
		return NULL;
	}
	
	return mbn;
}

void MBN_EXPORT mbnStartInterface(struct mbn_interface *itf, char *err) {
	/* init interface */
	if(itf->cb_init != NULL)
	{
		itf->cb_init(itf, err);
	}
}

/* IMPORTANT: must not be called in a thread which has a lock on mbn_mutex */
void MBN_EXPORT mbnFree(struct mbn_handler *mbn) {
	int i;
	
	/* disable all callbacks so the application won't see all kinds of activities
	* while we're freeing everything */
	mbn->cb_ReceiveMessage = NULL;
	mbn->cb_AddressTableChange = NULL;
	mbn->cb_WriteLogMessage = NULL;
	mbn->cb_OnlineStatus = NULL;
	mbn->cb_NameChange = NULL;
	mbn->cb_DefaultEngineAddrChange = NULL;
	mbn->cb_SetActuatorData = NULL;
	mbn->cb_GetSensorData = NULL;
	mbn->cb_ObjectFrequencyChange = NULL;
	mbn->cb_ObjectInformationResponse = NULL;
	mbn->cb_ObjectFrequencyResponse = NULL;
	mbn->cb_SensorDataResponse = NULL;
	mbn->cb_SensorDataChanged = NULL;
	mbn->cb_ActuatorDataResponse = NULL;
	mbn->cb_ObjectError = NULL;
	mbn->cb_Error = NULL;
	mbn->cb_AcknowledgeTimeout = NULL;
	mbn->cb_AcknowledgeReply = NULL;
	
	/* wait for the threads to be running
		* (normally they should be running right after mbnInit(),
	*  but there can be some slight lag on pthread-win32) */
	for(i=0; !mbn->msgqueue_run || !mbn->timeout_run || !mbn->throttle_run; i++) {
		if(i > 10)
		break; /* shouldn't happen, but silently ignore if it somehow does. */
		sleep(1);
	}
	
	/* Stop the interface receiving */
	if(mbn->itf->cb_stop != NULL)
    mbn->itf->cb_stop(mbn->itf);
	
	/* request cancellation for the threads */
	pthread_cancel(*((pthread_t *)mbn->timeout_thread));
	pthread_cancel(*((pthread_t *)mbn->throttle_thread));
	pthread_cancel(*((pthread_t *)mbn->msgqueue_thread));
	
	/* wait for the threads
	* (make sure no locks on mbn->mbn_mutex are present here) */
	pthread_join(*((pthread_t *)mbn->timeout_thread), NULL);
	pthread_join(*((pthread_t *)mbn->throttle_thread), NULL);
	pthread_join(*((pthread_t *)mbn->msgqueue_thread), NULL);
	free(mbn->timeout_thread);
	free(mbn->throttle_thread);
	free(mbn->msgqueue_thread);
	
	/* free address list */
	free_addresses(mbn);
	
	/* free interface */
	if(mbn->itf->cb_free != NULL)
    mbn->itf->cb_free(mbn->itf);
	
	/* free objects */
	for(i=0; i<mbn->node.NumberOfObjects; i++) {
		if(mbn->objects[i].SensorSize > 0) {
			free_datatype(MMTYPE(mbn->objects[i].SensorType), &(mbn->objects[i].SensorMin));
			free_datatype(MMTYPE(mbn->objects[i].SensorType), &(mbn->objects[i].SensorMax));
			free_datatype(mbn->objects[i].SensorType, &(mbn->objects[i].SensorData));
		}
		if(mbn->objects[i].ActuatorSize > 0) {
			free_datatype(MMTYPE(mbn->objects[i].ActuatorType), &(mbn->objects[i].ActuatorMin));
			free_datatype(MMTYPE(mbn->objects[i].ActuatorType), &(mbn->objects[i].ActuatorMax));
			free_datatype(MMTYPE(mbn->objects[i].ActuatorType), &(mbn->objects[i].ActuatorDefault));
			free_datatype(mbn->objects[i].ActuatorType, &(mbn->objects[i].ActuatorData));
		}
	}
	free(mbn->objects);
	
	/* and get rid of our mutex */
	pthread_mutex_destroy((pthread_mutex_t *)mbn->mbn_mutex);
	free(mbn->mbn_mutex);
	free(mbn);
	
	#ifdef PTW32_STATIC_LIB
		/* attach_np() doesn't seem to work again after we've
			* detached, so let's not call this function...
			if(--mbnhandlers == 0)
		pthread_win32_process_detach_np();*/
	#endif
}


int process_acknowledge_reply(struct mbn_handler *mbn, struct mbn_message *msg) {
	struct mbn_msgqueue *q;
	struct mbn_message orig;
	int ret = 1, tries = -1;
	
	if(!msg->AcknowledgeReply || msg->MessageID == 0)
    return 0;
	
	LCK();
	/* search for the message ID in our queue */
	for(q=mbn->queue; q!=NULL; q=q->next) {
		if(q->id == msg->MessageID)
		break;
	}
	
	/* found! */
	if(q != NULL && q->id == msg->MessageID) {
		/* make a copy for the callback */
		copy_message(&(q->msg), &orig);
		tries = q->retries-1;
		/* determine whether we need to process this message further,
		* If the original message is a GET action, then we should continue processing */
		if(q->msg.MessageType == MBN_MSGTYPE_OBJECT) {
			switch(q->msg.Message.Object.Action) {
				case MBN_OBJ_ACTION_GET_INFO:
				case MBN_OBJ_ACTION_GET_ENGINE:
				case MBN_OBJ_ACTION_GET_FREQUENCY:
				case MBN_OBJ_ACTION_GET_SENSOR:
				case MBN_OBJ_ACTION_GET_ACTUATOR:
				ret = 0;
				break;
				default:
				ret = 1;
			}
		}
		/* ...and signal the msgqueue thread to free() the message */
		q->retries = -1;
	}
	ULCK();
	
	/* send callback (if any) */
	if(tries >= 0 && mbn->cb_AcknowledgeReply != NULL) {
		mbn->cb_AcknowledgeReply(mbn, &orig, msg, tries);
		free_message(&orig);
	}
	return ret;
}


/* Entry point for all incoming MambaNet messages */
void MBN_EXPORT mbnProcessRawMessage(struct mbn_interface *itf, unsigned char *buffer, int length, void *ifaddr) {
	struct mbn_handler *mbn = itf->mbn;
	int r, processed = 0;
	struct mbn_message msg;
	char err[MBN_ERRSIZE];
	
	memset((void *)&msg, 0, sizeof(struct mbn_message));
	msg.raw = buffer;
	msg.rawlength = length;
	
	/* parse message */
	if((r = parse_message(&msg)) != 0) {
		if(mbn->cb_Error) {
			sprintf(err, "Couldn't parse incoming message (%d)", r);
			mbn->cb_Error(mbn, MBN_ERROR_PARSE_MESSAGE, err);
		}
		return;
	}
	
	/* Oh my, the interface is echoing back packets, let's ignore them */
	if((mbn->node.Services & MBN_ADDR_SERVICES_VALID) && msg.AddressFrom == mbn->node.MambaNetAddr)
    processed++;
	
	/* send ReceiveMessage() callback, and stop processing if it returned non-zero */
	if(!processed && mbn->cb_ReceiveMessage != NULL && mbn->cb_ReceiveMessage(mbn, &msg) != 0)
    processed++;
	
	/* handle address reservation messages */
	if(!processed && process_address_message(mbn, &msg, ifaddr) != 0)
    processed++;
	
	/* we can't handle any other messages if we don't have a validated address */
	if(!(mbn->node.Services & MBN_ADDR_SERVICES_VALID))
    processed++;
	/* ...or if it's not targeted at us */
	if(msg.AddressTo != MBN_BROADCAST_ADDRESS && msg.AddressTo != mbn->node.MambaNetAddr)
    processed++;
	
	/* acknowledge reply, yay! */
	if(!processed && process_acknowledge_reply(mbn, &msg) != 0)
    processed++;
	
	/* object messages */
	if(!processed && process_object_message(mbn, &msg) != 0)
    processed++;
	
	free_message(&msg);
}


void MBN_EXPORT mbnSendMessage(struct mbn_handler *mbn, struct mbn_message *msg, int flags) {
	unsigned char raw[MBN_MAX_MESSAGE_SIZE];
	char err[MBN_ERRSIZE];
	struct mbn_address_node *dest;
	struct mbn_msgqueue *q, *n, *prev_q;
	void *ifaddr;
	int r;
	
	if(mbn->itf->cb_transmit == NULL) {
		if(mbn->cb_Error) {
			sprintf(err, "Registered interface can't send messages");
			mbn->cb_Error(mbn, MBN_ERROR_NO_INTERFACE, err);
		}
		return;
	}
	
	if(!(flags & MBN_SEND_IGNOREVALID) && !(mbn->node.Services & MBN_ADDR_SERVICES_VALID)) {
		if(mbn->cb_Error) {
			sprintf(err, "Can't send message: we don't have a validated MambaNet address");
			mbn->cb_Error(mbn, MBN_ERROR_INVALID_ADDR, err);
		}
		return;
	}
	
	/* just forward the raw data to the interface, if we don't need to do any processing */
	if(flags & MBN_SEND_RAWDATA) {
		if(mbn->itf->cb_transmit(mbn->itf, raw, msg->rawlength, NULL, err) != 0) {
			if(mbn->cb_Error)
			mbn->cb_Error(mbn, MBN_ERROR_ITF_WRITE, err);
		}
		return;
	}
	
	if(!(flags & MBN_SEND_FORCEADDR))
    msg->AddressFrom = mbn->node.MambaNetAddr;
	
	/* lock, to make sure we have a unique message ID */
	LCK();
	
	if(!(flags & MBN_SEND_FORCEID) && !msg->AcknowledgeReply) {
		msg->MessageID = 0;
		if(flags & MBN_SEND_ACKNOWLEDGE) {
			/* get a new message ID */
			msg->MessageID = 1;
			if(mbn->queue != NULL) {
				q = mbn->queue;
				do {
					if(q->id >= msg->MessageID)
					msg->MessageID = q->id+1;
				} while((q = q->next) != NULL);
			}
		}
	}
	
	msg->raw = raw;
	msg->rawlength = 0;
	
	/* create the message */
	if((r = create_message(msg, (flags & MBN_SEND_NOCREATE)?1:0)) != 0) {
		ULCK();
		if(mbn->cb_Error) {
			sprintf(err, "Couldn't create message (%d)", r);
			mbn->cb_Error(mbn, MBN_ERROR_CREATE_MESSAGE, err);
		}
		return;
	}
	
	/* save the message to the queue if we need to check for acknowledge replies */
	if(flags & MBN_SEND_ACKNOWLEDGE) {
		/* create struct */
		n = malloc(sizeof(struct mbn_msgqueue));
		n->id = msg->MessageID;
		copy_message(msg, &(n->msg));
		n->retries = 0;
		n->next = NULL;
		/* add to the list */
		if(mbn->queue == NULL)
		mbn->queue = n;
		else {
			/* mbn->queue is not NULL so prev_q will always set in the while */
			prev_q = NULL;
			q = mbn->queue;
			while(q != NULL) {
				/* check for duplicate retry message, if found replace object-data*/
				if ((q->msg.AddressTo == n->msg.AddressTo) &&
					(q->msg.MessageType == MBN_MSGTYPE_OBJECT) &&
					(n->msg.MessageType == MBN_MSGTYPE_OBJECT) &&
					(q->msg.Message.Object.Action == n->msg.Message.Object.Action) &&
					(q->msg.Message.Object.Number == n->msg.Message.Object.Number)) {
					q->retries = -1;
				}
				prev_q = q;
				q = q->next;
			}
			prev_q->next = n;
		}
	}
	ULCK();
	
	/* determine interface address */
	if(msg->AddressTo == MBN_BROADCAST_ADDRESS)
    ifaddr = NULL;
	else {
		if((dest = mbnNodeStatus(mbn, msg->AddressTo)) == NULL)
		ifaddr = NULL;
		else
		ifaddr = dest->ifaddr;
	}
	
	/* send the data to the interface transmit callback */
	if(mbn->itf->cb_transmit(mbn->itf, raw, msg->rawlength, ifaddr, err) != 0) {
		if(mbn->cb_Error)
		mbn->cb_Error(mbn, MBN_ERROR_ITF_WRITE, err);
	}
}


void MBN_EXPORT mbnUpdateNodeName(struct mbn_handler *mbn, char *name) {
	memset((void *)mbn->node.Name, 0, 32);
	memcpy((void *)mbn->node.Name, (void *)name, strlen(name));
}
void MBN_EXPORT mbnUpdateEngineAddr(struct mbn_handler *mbn, unsigned long addr) {
	mbn->node.DefaultEngineAddr = addr;
}
void MBN_EXPORT mbnUpdateServiceRequest(struct mbn_handler *mbn, char srv) {
	mbn->node.ServiceRequest = srv;
}

void MBN_EXPORT mbnWriteLogMessage(struct mbn_interface *itf, const char *fmt, ...) {
	struct mbn_handler *mbn = itf->mbn;
	if(mbn->cb_WriteLogMessage != NULL) {
		va_list ap;
		char buf[500];
		va_start(ap, fmt);
		vsnprintf(buf, 500, fmt, ap);
		va_end(ap);
		
		mbn->cb_WriteLogMessage(mbn, buf);
	}
}



//--------------------------------------------------------------------------------------------------------------------
// DIT WERKT WEL !!!!
//MBN_EXPORT char mbnVersion(char * versionString) 
//{
	// // //sprintf(versionString, "MambaNet Library V1.10 - %s (%s) - %s", __DATE__, __TIME__, MBN_MANUFACTURER_NAME);
	// // sprintf(versionString, "MambaNet Library V1.10");
	// // return versionString;
//	return 0;
//}
//---------------------------------------------------------------------------------------------------------------------

// DIT HIERONDER WERKT NIET !!!!!
// const char *MBN_EXPORT mbnVersion() {
// sprintf(versionString, "MambaNet Library V1.10 - %s (%s) - %s", __DATE__, __TIME__, MBN_MANUFACTURER_NAME);
// return versionString;
// }

// __declspec(dllexport) const char __cdecl mbnVersion() {
// sprintf(versionString, "MambaNet Library V1.10 - %s (%s) - %s", __DATE__, __TIME__, MBN_MANUFACTURER_NAME);
// return versionString;
// }


// int deSom(int a, int b) {
// return a + b;
// }

/*
	//Informatie over de node
	struct mbn_node_info node = 
	{
	0, 0,								// MambaNetAddr, Services
    "MambaNet Library Example Node", 	// Description
    "Example-Test-3",						// Name
    0x0001, 0x0022, 0x0244,				// UniqueMediaAccessID
    0, 0,								// Hardware revision 
    1, 3,						 		//  Firmware revision 
    0, 0,						 		//  FPGAFirmware revision 
    1,							 		//  NumberOfObjects 
    0,							 		//  DefaultEngineAddr 
    {0x0,0x0,0x0},	 		//  Hardwareparent
    0					  				// Service request 
	};
	
	struct mbn_object objects[1];	// Declareer  objecten
	
	MBN_EXPORT void start()
	{
	//int itf = Connect();
	//int mbn = mambaInit(itf);
	//int mbn = 0;
	struct mbn_handler *mbn;
	struct mbn_interface *itf;
	char error[MBN_ERRSIZE];
	
	
	itf = mbnUDPOpen("192.168.1.76", "34848", NULL, error);  // Maak verbinding met ethernet
	
	//########################################################
	//  initialiseer een node op het Ethernet met 3 objecten.
	//########################################################
	
	// sensor 						 sensor type, freq, size (bytes), min, max, current value,  actuator (none)
	objects[0] = MBN_OBJ("Object #1", MBN_DATATYPE_UINT, 0, 2 , 0, 512, 256, MBN_DATATYPE_NODATA);
	
	
	// Initialiseer de MambaNet node op het Ethernet
	mbn = mbnInit(&node, objects, itf, error);  // Een handle wordt verkregen door mbnInit aan te roepen
	printf("Interface is misschien gestart\n\n itf = %d\n mbn = %d\n\n", itf, mbn);
	
	//Start alle geopende interfacen
	mbnStartInterface(itf, error);
    
    printf("Example #1 2  Oke\n");
	
	// mbnFree(mbn);
	// return 0;
	}
	
	
	// MBN_EXPORT int Connect()
	// {
	// int itff = mbnUDPOpen("192.168.1.76", "34848", 0, 0);  // Maak verbinding met ethernet	return itf;
	// return itff;
	// }
	
	// MBN_EXPORT int mambaInit(int itf)
	// {
	// int mbn = mbnInit(&node, 0, itf, 0);
	// return mbn;
	// }
	
*/

// --- OPNIEUW------------------------------------------------------------------------------

#define FIRMWARE_MAJOR_VERSION   	0
#define FIRMWARE_MINOR_VERSION   	0
#define FIRMWARE_PATCH_VERSION      1

#define  OBJVALBUF 16

struct mbn_interface *itf = NULL;
struct mbn_handler *mbn;
char error[MBN_ERRSIZE];
char online;
struct mbn_node_info thisnode;
int cntSwitch;
struct mbn_object objects[350];
int cntObject = 0;
char obj_desc[32];

unsigned int SwitchState[380];  
unsigned int OnColor[380];
unsigned int OffColor[380];


char ObjectArr[24];

unsigned int FaderLevel[6];  // 6 Faders
unsigned int PPMlevel[12], oldPPMlevel[12];;
unsigned char DisplayLine12[20];

//unsigned char buffer[24];

unsigned char buffer2[24];

int num, oldNum=0, tel;

int ui_6fbp_mambanet_init();


unsigned char valStr[80];
int objct;

char dumpnew[2180];
char dump[2180];



concatenateStr(int upddump)
{
	strcat(dump,upddump);			
	strcpy(dumpnew,dump);
	
	// int buffer[10];
	// sprintf(buffer, "#%04d_%04d", 1084, 1023);
	// strcpy(dumpnew,buffer);
}

// Nu komt het in C# terecht
MBN_EXPORT char retString(char *stringOut) 
{	
	sprintf(stringOut, "%s\n", dumpnew);
	dump[0]='\0';
}

// Callback functie
MBN_EXPORT void mSetActuatorData(struct mbn_handler *mbn, unsigned short object, union mbn_data data)
{
	
	int mod;
	int ch;	
	int sw, t;	
	
	
	if( (object >= 1024) && (object <=1035) )   //Display text
	{
		int val = data.Octets;
		objct = (int)object;
		char buffer[OBJVALBUF];
		sprintf(buffer, "#%04d_%08s", objct, val); 
		concatenateStr(buffer);	
	}
	
	// labels
	if (object >= 1138 && object <= 1191)
	{
		unsigned int statt;
		statt = data.State;
		
	    char buffer[OBJVALBUF];
		sprintf(buffer, "#%04d_%04d", object, statt);
		concatenateStr(buffer);
		
	}
	
	//PotentialMeters
	else if ((object >= 1339) && (object <=1342))
	{
		unsigned int statt;
		statt = data.State;
		
	    char buffer[OBJVALBUF];
		sprintf(buffer, "#%04d_%04d", object, statt);
		concatenateStr(buffer);
		
	}

	//else if (object >= 1114)
	else if ( (object >= 1114) && (object <=1137) || (object >= 1290) && (object <=1338))  
		// <-----------------------OFF COLOR-----------
	{	
		if ( (object >= 1114) && (object <=1137) )
			sw = object-1114;
		else if ( (object >= 1290) && (object <=1338) )
			sw = object-1290;
		OffColor[sw] = data.UInt;
	}
	
	
	// knopjes
	//else if (object >= 1090) 
	else if ( (object >= 1090) && (object <=1113) || (object >= 1241) && (object <=1289) )		
		// <-------------------------ON COLOR---------------------
	{	
		if ( (object >= 1090) && (object <=1113) )
			sw = object-1090;
		else if ( (object >= 1241) && (object <=1289) )
			sw = object-1241;
		OnColor[sw] = data.UInt;
	}	
	
	//else if (object >= 1060)
	else if ( (object >= 1060) && (object <=1083) || (object >= 1192) && (object <=1240) ) 
	{	
		// Faderpanel switches 1.1 - 6.4  <---------------------------------STATE-------------------------------
		
		if ( (object >= 1060) && (object <=1083) )
			sw = object-1060;
		else if ( (object >= 1192) && (object <=1240) ) 
			sw = object-1192;
		SwitchState[sw] = data.State;
		
		if( OnColor[sw] == 0 ) OnColor[sw] = 2;
		
		int colorState;
		
		if( SwitchState[sw] )
			colorState=OnColor[sw];
		else
			colorState=OffColor[sw];
		
		char buffer[OBJVALBUF];
		sprintf(buffer, "#%04d_%04d", object, colorState); 
		concatenateStr(buffer);						
	}	
	
	
	else if( (object >= 1084) && (object <=1089) )     // Motor Faders
	{	
		// Fader level 
		mod = object-1084;
		FaderLevel[mod] = data.UInt;

		int buffer[OBJVALBUF];
		sprintf(buffer, "#%04d_%04d", object, FaderLevel[mod]);
		concatenateStr(buffer);
		//strcpy(dumpnew,buffer);	
	}
	
	
	
	else if( object >= 1054 )
	{
		// Encoder switches 
		// NEVER COME HERE.. NO ACTUATORS... 
	}
	else if( object >= 1048 )
	{
		// Encoder 
		// NEVER COME HERE.. NO ACTUATORS... 
	}
	
		
	
	else if( object >= 1036 && object <= 1047) // VU Left
	{
		unsigned int Value = 148+(int)data.Float;
		char dumpp[OBJVALBUF];
		sprintf(dumpp, "#%04d_%04d", object,  Value);						
		concatenateStr(dumpp);
	}
	

}




MBN_EXPORT  void mOnlineStatus(struct mbn_handler *mbn, unsigned long addr, char valid)
{
	online = valid; 
	// printf("OnlineStatus: %08lX %s\n", addr, valid ? "validated" : "invalid");        
}

int ui_6fbp_mambanet_init()
{
	
	int cnt;
	
	thisnode.MambaNetAddr = 0;
	thisnode.Services = 0;	
	sprintf(thisnode.Description, "Axite Control Surface 6 Faders + CRM");  	
	sprintf(thisnode.Name, "Axite-UI-6FBP-CRM");	
	thisnode.ManufacturerID = 0x0001;
	thisnode.ProductID = 0x0022;
	thisnode.UniqueIDPerProduct = 0x505;   //UniqueIDPerProduct
	thisnode.HardwareMajorRevision = 0;
	thisnode.HardwareMinorRevision = 0;
	thisnode.FirmwareMajorRevision = 1;
	thisnode.FirmwareMinorRevision = 3;
	thisnode.FPGAFirmwareMajorRevision = 0;
	thisnode.FPGAFirmwareMinorRevision = 0;
	thisnode.NumberOfObjects = 0;
	thisnode.DefaultEngineAddr = 0;	
	thisnode.HardwareParent[0] = 0;
	thisnode.HardwareParent[1] = 0;
	thisnode.HardwareParent[2] = 0;
	thisnode.ServiceRequest = 0;		
	
	/* 1024 - 1035: OLED Display lines */
	for (cnt=0; cnt<6; cnt++) 
	{
		sprintf(obj_desc, "Display %d Line 1", cnt+1);
		
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");
		
		//objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_UINT, 0, 2 , 0, 512, 256, MBN_DATATYPE_NODATA);
	}
	for (cnt=0; cnt<6; cnt++) 
	{
		sprintf(obj_desc, "Display %d Line 2", cnt+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");
	}
	
	/* 1036 - 1047: Stereo Audio level metering */
	for (cnt=0; cnt<6; cnt++) 
	{
		sprintf(obj_desc, "Display Meter %d Left dB", cnt+1);
		objects[cntObject++] = MBN_OBJ( obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_FLOAT, 2, -50.0, 5.0, -50.0, -50.0);
		sprintf(obj_desc, "Display Meter %d Right dB", cnt+1);
		objects[cntObject++] = MBN_OBJ( obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_FLOAT, 2, -50.0, 5.0, -50.0, -50.0);
	}
	
	/* 1048 - 1053: Encoder rotation */
	for (cnt=0; cnt<6; cnt++) 
	{
		sprintf(obj_desc, "Encoder %d", cnt+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_SINT, 1, 1, -128, 127, 0, MBN_DATATYPE_NODATA);
	}
	
	/* 1054 - 1059: Encoder switch */
	for (cnt=0; cnt<6; cnt++) 
	{
		sprintf(obj_desc, "Encoder %d Switch", cnt+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_STATE, 1, 1, 0, 1, 0, MBN_DATATYPE_NODATA);
	}
	
	
	
	/* 1060 - 1083: Module switches (TB, INP B, CUE/PFL, START/STOP) */
	for (cntSwitch=0; cntSwitch<4; cntSwitch++) 
	{
		for (cnt=0; cnt<6; cnt++) 
		{
			sprintf(obj_desc, "Switch %d.%d", cnt+1, cntSwitch+1);
			objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_STATE, 1, 1, 0, 1, 0, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
		}	
	}
	
	/* 1084 - 1089: Fader (also actuator[A] to store actual value set by other node) */
	for (cnt=0; cnt<6; cnt++) 
	{
		sprintf(obj_desc, "Fader %d", cnt+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0, MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);
	}
	
	/* 1090 - 1113: Module switches ON color */
	for (cntSwitch=0; cntSwitch<4; cntSwitch++) 
	{
		for (cnt=0; cnt<6; cnt++) 
		{
			sprintf(obj_desc, "Switch %d.%d on color", cnt+1, cntSwitch+1);
			objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 3, 2, 2);
		}
	}
	
	/* 1114 - 1137: Module switches OFF color */
	for (cntSwitch=0; cntSwitch<4; cntSwitch++) 
	{ /*cntSwitch == switchID */
		for (cnt=0; cnt<6; cnt++)  /* cnt == module */
		{
			sprintf(obj_desc, "Switch %d.%d off color", cnt+1, cntSwitch+1);
			objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 3, 0, 0);
		}
	}
	
	/* 1138 - 1143: PGM on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) 
	{
		sprintf(obj_desc, "PGM %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1144 - 1149: SUB on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "SUB %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1150 - 1155: EQ on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "EQ %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1156 - 1161: DYN on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "Dynamics %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1162 - 1167: AUX1 on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "Aux1 %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1168 - 1173: AUX2 on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "Aux2 %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1174 - 1179: AUX3 on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "Aux3 %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1180 - 1185: AUX4 on/off */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "Aux4 %d on/off", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	
	/* 1186 - 1191: Peak Indicator (ppm) */
	for (cntSwitch=0; cntSwitch<6; cntSwitch++) {
		sprintf(obj_desc, "Peak %d", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}  
	
	/* 1192 - 1240: CRM Switches */
	for (cntSwitch=0; cntSwitch<49; cntSwitch++) 
	{
		sprintf(obj_desc, "Switch %d", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_STATE, 1, 1, 0, 1, 0, MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
	}
	/* 1241 - 1289: CRM switches ON color */
	for (cntSwitch=0; cntSwitch<49; cntSwitch++) 
	{
		sprintf(obj_desc, "Switch %d on color", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 3, 2, 2);
	}
	/* 1290 - 1338: CRM switches OFF color */
	for (cntSwitch=0; cntSwitch<49; cntSwitch++) 
	{
		sprintf(obj_desc, "Switch %d off color", cntSwitch+1); 
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_NODATA, MBN_DATATYPE_STATE, 1, 0, 3, 0, 0);
	}
	/* 1339 - 1342: Potmeters, also actuator to internally save current value set by other nodes (not in sync status) */
	for (cntSwitch=0; cntSwitch<4; cntSwitch++) 
	{
		sprintf(obj_desc, "Potential Meter %d", cntSwitch+1);
		objects[cntObject++] = MBN_OBJ(obj_desc, MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0, MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);
	}	
	
	
	
	// // //------------------------------------------------------------------------------------------------	
	
	thisnode.NumberOfObjects = cntObject;		
	//fprintf(stdout, "%s\n",mbnVersion());
	
	if( (mbn = mbnInit(&thisnode, objects, itf, error)) == NULL) // Verkrijgen van een Handle door mbnInit
	{
		//printf("Error initializing mbn: %s\n", error);
		return 1;
	}
	
	//mbnSetErrorCallback(mbn, Error);	
	mbnSetSetActuatorDataCallback(mbn, mSetActuatorData);
	//mbnSetWriteLogMessageCallback(mbn, mWriteLogMessage);
	//mbnSetAddressTableChangeCallback(mbn, mAddressTableChange);
	//mbnStartInterface(itf, err);
	mbnSetOnlineStatusCallback(mbn, mOnlineStatus);
	
	return mbn;
	
}	






MBN_EXPORT void connectToObjects(int ipA, int ipB, int ipC, int ipD)
{	
	//itf = mbnUDPOpen("192.168.1.76", "34848", NULL, error);  // Maak verbinding met ethernet
	//itf = mbnUDPOpen("192.168.0.222", "34848", NULL, error);  // Maak verbinding met ethernet
	
	char buffer[20];
	sprintf(buffer, "%d.%d.%d.%d", ipA, ipB, ipC, ipD); 

	
	
	itf = mbnUDPOpen(buffer, "34848", NULL, error);  // Maak verbinding met ethernet
	
	if (itf == NULL) 
	{
		//printf("Error opening ethernet device: %s", error);
		return 0;
	}
	
	
	//  initialiseer alle nodes op het Ethernet met de objecten.
	ui_6fbp_mambanet_init();
	

	
	//Sleep(300);
	
	
	
	//Start alle geopende interfacen
	mbnStartInterface(itf, error);	
	

		
}

//MBN_EXPORT void UpdateActuator(unsigned short obj, int num)
//{
// union mbn_data d;

// d.SInt = num;

// if(online)
// {	
// mbnUpdateActuatorData(mbn, obj, d);
// }

// return 0;	
//}	

MBN_EXPORT void sensor(unsigned short obj, int num)
{
	union mbn_data d;
	
	
	d.SInt = num;
	
	//mbnUpdateSensorData((struct)18423336, 1, 1072);
	
	if(online)
	{	
		mbnUpdateSensorData(mbn, obj, d);
		//Sleep(20);
	}
	


	//mbnFree(mbn);
	//return 0;

}	





























