//#define MBN_VARARG
#include "mbn.h"
#include <stdio.h>
#include <errno.h>
 #include <pthread.h>

// Dit hier global zetten of niet ?
struct mbn_handler *mbn;
char error[MBN_ERRSIZE];
struct mbn_interface *itf = NULL; 

//Informatie over de node
struct mbn_node_info node = 
{
	// 0, 0,								// MambaNetAddr, Services
    // "MambaNet Library Example Node", 	// Description
    // "Example-Test-2",						// Name
    0,0,"0","0",0x0001, 0x0022, 0x0233,				// UniqueMediaAccessID
    // 0, 0,								// Hardware revision 
    // 1, 3,						 		//  Firmware revision 
    // 0, 0,						 		//  FPGAFirmware revision 
    // 0,							 		//  NumberOfObjects 
    // 0,							 		//  DefaultEngineAddr 
    // {0x0,0x0,0x0},	 		//  Hardwareparent
    // 0					  				// Service request 
};

int main(void)
{
	// Open de ethernet interface
    itf = mbnUDPOpen("192.168.1.76", "34848", 0, 0);  // Maak verbinding met ethernet

  
 	 // Initialiseer de MambaNet node op het Ethernet
	//mbn = 
	//mbnInit(&node, 0, itf, 0);  // Een handle wordt verkregen door mbnInit aan te roepen
	
	mbnInit(0, 0, 0, 0);

	// //Start alle geopende interfacen
	// mbnStartInterface(itf, 0);
     
    printf("Interface is misschien gestart\n\n itf = %d\n mbn = %d\n\n", itf, mbn);
	

    //[do some work here to keep the program alive]
	while(1)
	{
        ;
    }
	//Sleep(100000);
    
	mbnFree(mbn);
	return 0;
}

