//########################################################
//  initialiseer een node op het Ethernet zonder objecten.
//########################################################

#include "mbn.h"
#include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <strings.h>
// #include "common.h"

// Dit hier global zetten of niet ?
struct mbn_handler *mbn;
char error[MBN_ERRSIZE];
struct mbn_interface *itf = NULL;

// Informatie over de node
struct mbn_node_info node = 
{
0, 0,								// MambaNetAddr, Services
    "MambaNet Library Example Node", 	// Description
    "Example #1",						// Name
    0x0001, 0x0011, 0x0011,				// UniqueMediaAccessID
    0, 0,								// Hardware revision 
    0, 0,						 		//  Firmware revision 
    0, 0,						 		//  FPGAFirmware revision 
    0,							 		//  NumberOfObjects 
    0,							 		//  DefaultEngineAddr 
    {0x1,0x0444,0x00AA},	 		//  Hardwareparent
    0					  				// Service request 
};

int main(void)
{	
    fprintf(stdout, "%s\n",mbnVersion());

    // Open de ethernet interface
    //itf = mbnEthernetOpen("Intel(R) PRO/1000 MT Desktop Adapter", error);
	itf = mbnUDPOpen("192.168.1.76", "34848", NULL, error);  // Maak verbinding met ethernet
	    
    if (itf == NULL) 
    {
		printf("Error opening ethernet device: %s", error);
		return 0;
    }

    // Initialiseer de MambaNet node op het Ethernet zonder objecten
    mbn = mbnInit(&node, NULL, itf, error);

    if (mbn == NULL) 
    {
		printf("Error initializing MambaNet node: %s", error);
		return 0;
    }

    //printf("Example #1 Test Oke\n");

    //Start alle geopende interfacen
    mbnStartInterface(itf, error);    
    
    //mbnEthernetIFList("192.168.1.76");
    
    

    // [do some work here to keep the program alive]
    while(1)
    {
		printf("%s",mbnEthernetIFList("192.168.1.76"));
	    delay(1000);
    }

    mbnFree(mbn);	
    return 0;
}

