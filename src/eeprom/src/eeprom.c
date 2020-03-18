#include "eeprom.h"
#include "eeprom_prv.h"
#include "eeprom_cfg.h"
#include "bits.h"
#include "types.h"
#include <avr/io.h>

#if EEPROM_ENABLE_SERIAL_DUMP == ON
#include <stdio.h>
#include <string.h>
#include "serial.h"
#endif

static EepromState      eepromState = EEPROM_Ready;
static EepromDataBuffer writeBuffer;
static EepromDataBuffer readBuffer;

bool EEPROM_IsReady(void)
{
    bool retval = FALSE;

    if (eepromState == EEPROM_Ready)
    {
        retval = TRUE;
    }

    return retval;
}

Std_ReturnType EEPROM_BackgroundTask()
{
    Std_ReturnType retval = Status_OK;

    if (eepromState == EEPROM_Writing)
    {
        if (!IS_SET_BIT(EECR, EEPE))
        {
            if (writeBuffer.length > 0)
            {
                EEAR = writeBuffer.romAddr++;
                EEDR = READ_PU8(writeBuffer.ramAddr++);
                writeBuffer.length--;

                /* Set Master Program Enable first, then Program Enable */
                SET_BIT(EECR, EEMPE);
                SET_BIT(EECR, EEPE);
            }
            else
            {
                eepromState = EEPROM_Ready;
            }
        }

        // More processing needed
        retval = Status_Pending;
    }
    else if (eepromState == EEPROM_Reading)
    {
        if (!IS_SET_BIT(EECR, EEPE))
        {
            if (readBuffer.length > 0)
            {
                EEAR = readBuffer.romAddr++;
                SET_BIT(EECR, EERE);
                WRITE_PU8(readBuffer.ramAddr++, EEDR);
                readBuffer.length--;
            }
            else
            {
                eepromState = EEPROM_Ready;
            }
        }

        // More processing needed
        retval = Status_Pending;
    }

    return retval;
}

Std_ReturnType EEPROM_AsyncWrite(word ucAddress, void *ucData, int dataLength)
{
    Std_ReturnType retval = Status_Not_OK;

    if (eepromState == EEPROM_Ready && dataLength > 0)
    {
        /* Set Programming mode */
        RESET_BIT(EECR, EEPM0);
        RESET_BIT(EECR, EEPM1);

        writeBuffer.romAddr = ucAddress;
        writeBuffer.ramAddr = ucData;
        writeBuffer.length  = dataLength;

        eepromState = EEPROM_Writing;

        retval = Status_Pending;
    }

    return retval;
}

Std_ReturnType EEPROM_AsyncRead(word ucAddress, void *ucData, int dataLength)
{
    Std_ReturnType retval = Status_Not_OK;

    if (eepromState == EEPROM_Ready && dataLength > 0)
    {
        readBuffer.romAddr = ucAddress;
        readBuffer.ramAddr = ucData;
        readBuffer.length  = dataLength;

        eepromState = EEPROM_Reading;

        retval = Status_Pending;
    }

    return retval;
}


Std_ReturnType EEPROM_MemSet(word ucAddress, byte val, int length)
{
    Std_ReturnType retval = Status_Not_OK;

    if (eepromState == EEPROM_Ready)
    {
        /* Wait for completion of previous write */
        while (IS_SET_BIT(EECR, EEPE));

        /* Set Programming mode : erase and write in atomic operation */
        RESET_BIT(EECR, EEPM0);
        RESET_BIT(EECR, EEPM1);

        while (length-- > 0)
        {
            EEAR = ucAddress++;
            EEDR = val;

            /* Set Master Program Enable first, then Program Enable */
            SET_BIT(EECR, EEMPE);
            SET_BIT(EECR, EEPE);

            while (IS_SET_BIT(EECR, EEPE));
        }

        retval = Status_OK;
    }

    return retval;
}

Std_ReturnType EEPROM_SyncWrite(word ucAddress, void *ucData, int length)
{
    Std_ReturnType retval = Status_Not_OK;

    if (eepromState == EEPROM_Ready)
    {
        /* Wait for completion of previous write */
        while (IS_SET_BIT(EECR, EEPE));

        /* Set Programming mode : erase and write in atomic operation */
        RESET_BIT(EECR, EEPM0);
        RESET_BIT(EECR, EEPM1);

        while (length-- > 0)
        {
            EEAR = ucAddress++;
            EEDR = READ_PU8(ucData++);

            /* Set Master Program Enable first, then Program Enable */
            SET_BIT(EECR, EEMPE);
            SET_BIT(EECR, EEPE);

            while (IS_SET_BIT(EECR, EEPE));
        }

        retval = Status_OK;
    }

    return retval;
}

Std_ReturnType EEPROM_SyncRead(word ucAddress, void *ucData, int length)
{
    Std_ReturnType retval = Status_Not_OK;

    if (eepromState == EEPROM_Ready)
    {
        /* Wait for completion of previous write */
        while (IS_SET_BIT(EECR, EEPE));

        while(length-- > 0)
        {
            EEAR = ucAddress++;
            SET_BIT(EECR, EERE);
            WRITE_PU8(ucData++, EEDR);
        }

        retval = Status_OK;
    }

    return retval;
}


#if EEPROM_ENABLE_BLOCK_API == ON

Std_ReturnType EEPROM_IterateBlocks(byte type, void *buffer, Callback cbk)
{
    word addr = 0;
    Header hdr = {0};

    while ( addr < EEPROM_SIZE - sizeof(Header) )
    {
        EEPROM_SyncRead(addr, &hdr, sizeof(Header));
        if (hdr.id != 0xFFFF)
        {
            if (hdr.type == type)
            {
                EEPROM_SyncRead(addr, buffer, hdr.size);
                Std_ReturnType retval = cbk(buffer);
                if(retval != Status_Pending) return retval;
            }

            addr += hdr.size;
        }
        else return Status_OK;
    }

    return Status_OK;
}

#if EEPROM_ENABLE_SERIAL_DUMP == ON
Std_ReturnType EEPROM_DumpEEPROM(word from, word to, unsigned int len)
{
    byte b = 0;
    char strBuffer[16];
    for ( word addr = from ; addr < to ; addr++ )
    {
        EEPROM_SyncRead(addr, &b, 1);

        if (addr % len == 0)
        {
            sprintf(strBuffer, "0x%04X | %02X ", addr, b);
        }
        else if (addr % len == len-1)
        {
            sprintf(strBuffer, "%02X\r\n", b);
        }
        else
        {
            sprintf(strBuffer, "%02X ", b);
        }

        Serial_Print(strBuffer);
    }
    return Status_OK;
}
#endif

Std_ReturnType EEPROM_GetLastBlock(void* blk, byte type)
{
    Header hdr = {0};

    word lastAddr = EEPROM_SIZE + 1;
    word lastId = 0;
    word lastSz = 0;
    word addr = 0;

    // Find block with the highest id
    while ( addr < EEPROM_SIZE - sizeof(Header) )
    {
        EEPROM_SyncRead(addr, &hdr, sizeof(Header));
        if ( hdr.id != 0xFFFF )
        {
            if ( hdr.id >= lastId && hdr.type == type )
            {
                lastAddr = addr;
                lastId = hdr.id;
                lastSz = hdr.size;
            }

            addr += hdr.size;
        }
        else break;
    }

    // Read the block
    if ( lastAddr < EEPROM_SIZE - lastSz)
    {
        return EEPROM_SyncRead(lastAddr, blk, lastSz);
    }

    return Status_Not_OK;
}

Std_ReturnType EEPROM_WriteBlock(void* blk)
{
    word addr = 0;
    Header hdr = {0};

    Header *blockHeader = TYPECAST(blk, Header*);

    // Look for last write address
    while (addr < EEPROM_SIZE - sizeof(Header))
    {
        EEPROM_SyncRead(addr, &hdr, sizeof(Header));
        if ( hdr.id != 0xFFFF ) addr += hdr.size;
        else return EEPROM_SyncWrite(addr, blk, blockHeader->size);
    }

    return Status_Not_OK;
}

#endif
