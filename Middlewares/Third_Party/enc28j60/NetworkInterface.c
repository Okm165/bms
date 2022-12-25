#include "enc28j60.h"

typedef long BaseType_t;

#include "main.h"
#include "list.h"
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"

void vReleaseNetworkBufferAndDescriptor( xNetworkBufferDescriptor_t * const pxNetworkBuffer );

BaseType_t xNetworkInterfaceInitialise( void ) 
{
    if ( enc28j60_init(ucMACAddress) == 0 ) {
        return pdPASS;
    } else {
        return pdFAIL;
    }
}

BaseType_t xNetworkInterfaceOutput( xNetworkBufferDescriptor_t * const pxDescriptor,
                                    BaseType_t xReleaseAfterSend )
{
    enc28j60_send_packet(pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength );
    
    debug("FreeRTOS: Packet forwarded to driver for transmiting...\n");
    /* Call the standard trace macro to log the send event. */
    iptraceNETWORK_INTERFACE_TRANSMIT();

    if( xReleaseAfterSend != pdFALSE )
    {
        vReleaseNetworkBufferAndDescriptor( pxDescriptor );
    }

    return pdTRUE;
}

