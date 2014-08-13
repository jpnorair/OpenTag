/*******************************************************************************
 *
 * msp430f5_tlv.c
 * Provides Functions to Read the TLV Data Section of the MSP430 Devices
 *
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#include "msp430f5_lib.h"

void TLV_Get_Info(u8 tag, u8 instance, u8 *length, u16 **data_address)
{
    u8 *TLV_address = (u8 *)TLV_START;                           // TLV Structure Start Address

    while ((TLV_address < (u8 *)TLV_END)
           && ((*TLV_address != tag) || instance)                    // check for tag and instance
           && (*TLV_address != TLV_TAGEND))                          // do range check first
    {
        if (*TLV_address == tag) instance--;                         // repeat till requested
                                                                     // instance is reached
        TLV_address += *(TLV_address + 1) + 2;                       // add (Current TAG address +
                                                                     // LENGTH) + 2
    }

    if (*TLV_address == tag)                                         // Check if Tag match
                                                                     // happened..
    {
        *length = *(TLV_address + 1);                                // Return length = Address + 1
        *data_address = (u16 *)(TLV_address + 2);               // Return address of first
                                                                     // data/value info = Address +
                                                                     // 2
    }
    else                                                             // If there was no tag match
                                                                     // and the end of TLV structure
                                                                     // was reached..
    {
        *length = 0;                                                 // Return 0 for TAG not found
        *data_address = 0;                                           // Return 0 for TAG not found
    }
}

u16 TLV_Get_Device_Type(void)
{
    u16 *pDeviceType = (u16 *)DEVICE_ID_0;

    return pDeviceType[0];                                           // Return Value from TLV Table
}

u16 TLV_Get_Memory(u8 instance)
{
    u8 *pPDTAG;
    u8 bPDTAG_bytes;
    u16 count;

    instance *= 2;                                                   // set tag for word access
                                                                     // comparison

    // TLV access Function Call
    TLV_Get_Info(TLV_PDTAG, 0, &bPDTAG_bytes, (u16 **)&pPDTAG); // Get Peripheral data pointer

    for (count = 0; count <= instance; count += 2)
    {
        if (pPDTAG[count] == 0) return 0;                            // Return 0 if end reached
        if (count == instance) return (pPDTAG[count] | pPDTAG[count + 1] << 8);
    }

    return 0;                                                        // Return 0: not found
}

u16 TLV_Get_Peripheral(u8 tag, u8 instance)
{
    u8 *pPDTAG;
    u8 bPDTAG_bytes;
    u16 count = 0;
    u16 pcount = 0;

    TLV_Get_Info(TLV_PDTAG, 0, &bPDTAG_bytes, (u16 **)&pPDTAG); // Get Peripheral data pointer

    // read memory configuration from TLV to get offset for Peripherals
    while (TLV_Get_Memory(count)){
        count++;
    }

    pcount = pPDTAG[count * 2 + 1];                                  // get number of Peripheral
                                                                     // entries
    count++;                                                         // inc count to first Periperal
    pPDTAG += count * 2;                                             // adjust point to first
                                                                     // address of Peripheral
    count = 0;                                                       // set counter back to 0
    pcount *= 2;                                                     // align pcount for work
                                                                     // comparision

    // TLV access Function Call
    for (count = 0; count <= pcount; count += 2){
        if (pPDTAG[count + 1] == tag){                               // test if required Peripheral
                                                                     // is found
            if (instance > 0){                                       // test if required instance is
                                                                     // found
                instance--;
            }
            else {
                return (pPDTAG[count] | pPDTAG[count + 1] << 8);     // Return found data
            }
        }
    }

    return 0;                                                        // Return 0: not found
}

u8 TLV_Get_Interrupt(u8 tag)
{
    u8 *pPDTAG;
    u8 bPDTAG_bytes;
    u16 count = 0;
    u16 pcount = 0;

    TLV_Get_Info(TLV_PDTAG, 0, &bPDTAG_bytes, (u16 **)&pPDTAG); // Get Peripheral data pointer

    // read memory configuration from TLV to get offset for Peripherals
    while (TLV_Get_Memory(count))
    {
        count++;
    }

    pcount = pPDTAG[count * 2 + 1];
    count++;                                                         // inc count to first Periperal
    pPDTAG += (pcount + count) * 2;                                  // adjust point to first
                                                                     // address of Peripheral
    count = 0;                                                       // set counter back to 0

    // TLV access Function Call
    for (count = 0; count <= tag; count += 2)
    {
        if (pPDTAG[count] == 0) return 0;                            // Return 0: not found/end of
                                                                     // table
        if (count == tag) return (pPDTAG[count]);                    // Return found data
    }

    return 0;                                                        // Return 0: not found
}


