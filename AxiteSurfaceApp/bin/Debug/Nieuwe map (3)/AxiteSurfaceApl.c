#define MBN_VARARG
//#include "common.h"
#include "mbn.h"

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>



#define FIRMWARE_MAJOR_VERSION   	0
#define FIRMWARE_MINOR_VERSION   	0
#define FIRMWARE_PATCH_VERSION      1

struct mbn_interface *itf = NULL;
struct mbn_handler *mbn;
char error[MBN_ERRSIZE];
char online;
struct mbn_node_info thisnode;
int cntSwitch;
struct mbn_object objects[350];
int cntObject = 0;
char obj_desc[32];

int num, oldNum=0;

int ui_6fbp_mambanet_init();
//void mambanet_send_switch_change(switches_t sw, int state);

FILE* fp2;

// const char* filePathSensor = "C:/mambanet/Willems-Test/Sensorfile.txt";
// const char* filePathActuator = "C:/mambanet/Willems-Test/Actuatorfile.txt";

const char* filePathSensor = "Sensorfile.txt";
const char* filePathActuator = "Actuatorfile.txt";
const char* fileIpAddress = "IpAddress.txt";

//#####################################################################


/* I/O callbacks */

// Callback functie
int mSetActuatorData(struct mbn_handler *mbn, unsigned short object, union mbn_data data)
{
	//oldNum=0;
	//union mbn_data out;
	FILE* fp;
	
	// int val = (int)data.Bits;	
	// printf("We zijn in de callback functie %d  %s \n", object, val );		
	
	switch(object)
	{
		case 1024: // Display Line 1 
		{

			printf("We zijn in de callback functie Display 1024 Line 1 %d  %s \n", object, data.Octets );			
		

			fp = fopen(filePathActuator,"w");	 
			fprintf(fp, "%04d%s%s", object, "_string", data.Octets);
			fclose(fp);
			Sleep(800);
			
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%s", "0000_0000");
			fclose(fp);
			Sleep(30);
			break;
		}		
		case 1030: // Display Line 2 
		{

			printf("We zijn in de callback functie Display 1030 Line 2 %d  %s \n", object, data.Octets );			
		

			fp = fopen(filePathActuator,"w");	 	
			fprintf(fp, "%04d%s%s", object, "_string", data.Octets);
			fclose(fp);
			Sleep(800);
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%s", "0000_0000");			
			fclose(fp);
			Sleep(30);
			break;
		}	
		//--------------------------------------------- VU METER ---------------------------------------------
		case 1036: // VU
		{
			float val = data.Float;
			val=148+val;
			int val2 = (int)val;
			fp = fopen(filePathActuator,"w");		
			fprintf(fp, "%04d%s%04d", object, "_", val2);
			//putc( val+0x30,fp);					
			fclose(fp);	
			//Sleep(50);
			Sleep(1);
			break;
		}
		case 1037: // VU
		{
			float val = data.Float;
			val=148+val;
			int val2 = (int)val;
			//printf("%d",val2);
			fp = fopen(filePathActuator,"w");		
			fprintf(fp, "%04d%s%04d", object, "_", val2);
			//putc( val+0x30,fp);					
			fclose(fp);	
			//Sleep(50);
			Sleep(1);
			break;
		}		
		//--------------------------------------------- MODULE 1 SWITCHES ----------------------------------------------	
		case 1060: // Switch SWITCH_
		{
			int val = data.State;			
			fp = fopen(filePathActuator,"w");		
			fprintf(fp, "%04d%s%04d", object, "_", val);				
			fclose(fp);	
			Sleep(50);
			break;
		}
		case 1066: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%04d%s%04d", object, "_", val);					
			fclose(fp);
			Sleep(50);
			break;
		}
		case 1072: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%s%s%04d", "1072", "_", val);				
			fclose(fp);
			Sleep(50);
			break;
		}
		case 1078: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%s%s%04d", "1078", "_", val);
			fclose(fp);
			Sleep(50);
			break;
		}
		
		
		//--------------------------------------------- MODULE 2 SWITCHES ----------------------------------------------	
		case 1061: // Switch SWITCH_
		{
			int val = data.State;			
			fp = fopen(filePathActuator,"w");		
			fprintf(fp, "%04d%s%04d", object, "_", val);					
			fclose(fp);	
			Sleep(50);
			break;
		}
		case 1067: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%04d%s%04d", object, "_", val);					
			fclose(fp);	
			Sleep(50);
			break;
		}
		case 1073: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%04d%s%04d", object, "_", val);				
			fclose(fp);	
			Sleep(50);
			break;
		}
		case 1079: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			Sleep(50);
			break;
		}	
		
		
		//--------------------------------------------- MODULE 3 SWITCHES ----------------------------------------------	
		case 1062: // Switch SWITCH_
		{
			int val = data.State;			
			fp = fopen(filePathActuator,"w");		
			fprintf(fp, "%04d%s%04d", object, "_", val);					
			fclose(fp);	
			break;
		}
		case 1068: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%04d%s%04d", object, "_", val);					
			fclose(fp);	
			break;
		}
		case 1074: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");
			fprintf(fp, "%04d%s%04d", object, "_", val);				
			fclose(fp);	
			break;
		}
		case 1080: // Switch SWITCH_
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			break;
		}	
		
		
		
		//------------------------------------ FADERS -------------------------------------------------------		
		case 1084: // Fader 1
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);
			Sleep(5);
			break;
		}
		case 1085: // Fader 2
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");		
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			break;
		}
		case 1086: // Fader 3
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			break;
		}
		case 1087: // Fader 4
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			break;
		}
		case 1088: // Fader 5
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			break;
		}	
		case 1089: // Fader 6
		{
			int val = data.State;
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			break;
		}	
		
//----------------------------------------------------------------------
		case 1195: // Switch Source
		{
			int val = data.State;
			printf("We zijn in de callback functie %d  %d \n", object, val);
			fp = fopen(filePathActuator,"w");			
			fprintf(fp, "%04d%s%04d", object, "_", val);
			fclose(fp);	
			break;
		}
		//default:
			//object=0;
		
	}
	object=0;
	return 0;
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
	thisnode.UniqueIDPerProduct = 0x212;   //UniqueIDPerProduct
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
	
//------------------------------------------------------------------------------------------------	
	
    thisnode.NumberOfObjects = cntObject;		
	//fprintf(stdout, "%s\n",mbnVersion());
	
	if( (mbn = mbnInit(&thisnode, objects, itf, error)) == NULL) // Verkrijgen van een Handle door mbnInit
	{
		printf("Error initializing mbn: %s\n", error);
		return 1;
	}
		
    //mbnSetErrorCallback(mbn, Error);	
	mbnSetSetActuatorDataCallback(mbn, mSetActuatorData);
    //mbnSetWriteLogMessageCallback(mbn, mWriteLogMessage);
    //mbnSetAddressTableChangeCallback(mbn, mAddressTableChange);
	//mbnStartInterface(itf, err);
    
}



void OnlineStatus(struct mbn_handler *mbn, unsigned long addr, char valid) 
{
	printf("OnlineStatus: %08lX %s\n", addr, valid ? "validated" : "invalid");
	online = valid;
}



int i, oldstate, switchState;

int main(int argc, int *argv[]) 
{		
	int a =1;
	union mbn_data d, un;
	
	printf("\nType the ip address of the engine\n Example: 192.168.0.200 \n Or simply press enter for your default IP\n");
	
	char ipaddress[30];
	fgets(ipaddress, 100, stdin);
	FILE* fpa;

	if ( (strcmp(ipaddress, "\n")) == 0 )
	{
		//char ipaddress[30]="192.168.1.70";
		char ipaddressBuf[30];
		
		fpa = fopen(fileIpAddress,"r");				
		fgets (ipaddress, 15, fpa); 					
		fclose(fpa);
		
		printf("Default IP will be used : %s\n",ipaddress);
		
		if( (itf = mbnUDPOpen( ipaddress, "34848", NULL, error)) == NULL) 
		{
			printf("UDP Error: %s\n", error);
			//return 1;
		}
	}				
	else
	{
		printf("\n\nIP address entered = %s\n\n",ipaddress);
		ipaddress[strlen(ipaddress) - 1] = '\0';
		
		if( (itf = mbnUDPOpen( ipaddress, "34848", NULL, error)) == NULL) 
		{
			printf("UDP Error: %s\n", error);
			return 1;
		}			
		
	}		
	
	
	//########################################################
	//  initialiseer een node op het Ethernet met objecten.
	//########################################################	
	ui_6fbp_mambanet_init();
	//########################################################
	
	
	mbnSetOnlineStatusCallback(mbn, OnlineStatus);
	
	//Start alle geopende interfacen
	mbnStartInterface(itf, error);
	
	
	FILE* fp;
	//char button1_4Start, oldButton1_4Start, oldc;	
	//union mbn_data d, e, I;
	
	char c;		
	char Str1[15];
	char oldStr1[15];
	int ret;
	char buf[15];
	char left[15], desc[15];	
	int  obj; 
	
	
	//-----------------------------------------------------------------------------------------------------------------
	// 1. Sensors inlezen	
	while(1)
	{			
		fp = fopen(filePathSensor,"r");				
		fgets (buf, 15, fp);  		
		strcpy(Str1, buf);					
		fclose(fp);
		
		Sleep(1);
		
		sprintf(desc, "%s", Str1+5);			
		sscanf(desc, "%d", &num);
		   
		
		if( oldNum != num )
		{	
			oldNum=num;
			strncpy(left, Str1, 4);			
			sscanf(left, "%d", &obj);								
			d.SInt = num;
			
			if ( online )
			{
				//mbnUpdateSensorData((struct)18423336, 1, 1072);
				mbnUpdateSensorData(mbn, obj, d);
				printf("We zijn in de Main     : %d %d  %d\n", mbn, num, obj);			
			}
		}
	}		
	mbnFree(mbn);
	return 0;
}										