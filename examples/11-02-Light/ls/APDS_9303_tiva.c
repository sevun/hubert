//*****************************************************************************
// fxos8700cq_tiva.c - Prototypes for the APDS_9303 Peripheral
//
//  Created on: Dec 23, 2019
//      Author: Brandon Dixon
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"
#include "inc/hw_memmap.h"
#include "APDS_9303_proc.h"
#include "tm4c123gh6pm.h"

void I2CHReceive(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
             uint8_t *pReceiveData, uint8_t ui8NumBytes)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, false);

    //specify register to be read
    I2CMasterDataPut(I2C1_BASE, ui32SlaveRegister);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, true);

    //if there is only one argument, we only need to use the
    //single send I2C function
    if( 1 == ui8NumBytes )
    {
        //send control byte and read from the register we
        //specified
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

        //wait for MCU to finish transaction
        while(I2CMasterBusy(I2C1_BASE));

        //return data pulled from the specified register
        pReceiveData[0] = I2CMasterDataGet(I2C1_BASE);
    }
    else
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //return data pulled from the specified register
        pReceiveData[0] = I2CMasterDataGet(I2C1_BASE);

        uint8_t ui8Counter;
        for (ui8Counter = 1; ui8Counter < (ui8NumBytes - 1); ui8Counter++ )
        {
            //Initiate send of data from the MCU
            I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

            // Wait until MCU is done transferring.
            while(I2CMasterBusy(I2C1_BASE));

            //return data pulled from the specified register
            pReceiveData[ui8Counter] = I2CMasterDataGet(I2C1_BASE);
        }

        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //return data pulled from the specified register
        pReceiveData[ui8Counter] = I2CMasterDataGet(I2C1_BASE);
    }
}


void I2CGetRegister(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
             uint8_t *pReceiveData, uint8_t ui8NumBytes)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, false);
  
    //specify register to be read (COMMAND REGISTER)
    I2CMasterDataPut(I2C1_BASE, ui32SlaveRegister);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, true);

    //if there is only one argument, we only need to use the
    //single send I2C function
    if( 1 == ui8NumBytes )
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //return data pulled from the specified register
        pReceiveData[0] = I2CMasterDataGet(I2C1_BASE);
       
    }
    else
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //return data pulled from the specified register
        pReceiveData[0] = I2CMasterDataGet(I2C1_BASE);

        uint8_t ui8Counter = 1;
        for (ui8Counter = 1; ui8Counter < (ui8NumBytes - 1); ui8Counter++ )
        {
            //Initiate send of data from the MCU
            I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

            // Wait until MCU is done transferring.
            while(I2CMasterBusy(I2C1_BASE));

            //return data pulled from the specified register
            pReceiveData[ui8Counter] = I2CMasterDataGet(I2C1_BASE);
        }

        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //return data pulled from the specified register
        pReceiveData[ui8Counter] = I2CMasterDataGet(I2C1_BASE);
    }

    
}



//sends an I2C command to the specified slave
void I2CHSend(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
               uint8_t *pReceiveData, uint8_t ui8NumBytes)
{
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, false);

    //Put the register to be sent into FIFO (COMMAND REGISTER)
    I2CMasterDataPut(I2C1_BASE, ui32SlaveRegister);    


    if(ui8NumBytes == 1){
         //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

        //put next piece of data into I2C FIFO
        I2CMasterDataPut(I2C1_BASE, pReceiveData[0]);

    }
    else{  
        uint8_t ui8Counter;      

        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        for (ui8Counter = 0; ui8Counter < (ui8NumBytes - 1); ui8Counter++ )
        {
            //put next piece of data into I2C FIFO
            I2CMasterDataPut(I2C1_BASE, pReceiveData[ui8Counter]);

            //send next data that was just placed into FIFO
            I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

            // Wait until MCU is done transferring.
            while(I2CMasterBusy(I2C1_BASE));
        }

        //put next piece of data into I2C FIFO
        I2CMasterDataPut(I2C1_BASE, pReceiveData[ui8NumBytes - 1]);        

       
    }   

    //send next data that was just placed into FIFO
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

    // Wait until MCU is done transferring.
    while(I2CMasterBusy(I2C1_BASE));


    
}


//gets one data byte from slave register with register address set from sendCmdByte
void I2CGetDatabyte(uint32_t ui32SlaveAddress, uint8_t *pReceiveData)
{

    //specify that we are reading
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, true);

    //Put the register to be sent into FIFO
    I2CMasterDataPut(I2C1_BASE, 0x00);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE));

    //return data pulled from the specified register
    pReceiveData[0] = I2CMasterDataGet(I2C1_BASE);
}




//sets the command register to the address register to for read/write on subsequent operation
void I2CHSendCmdByte(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister)
{
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, false);

    //Put the register to be sent into FIFO
    I2CMasterDataPut(I2C1_BASE, ui32SlaveRegister);

    //Initiate send of data from the MCU
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    // Wait until MCU is done transferring.
    while(I2CMasterBusy(I2C1_BASE));    
}


//sends an I2C command to the specified slave
void I2CHSendDataByte(uint32_t ui32SlaveAddress, uint8_t dataByte)
{
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C1_BASE, ui32SlaveAddress, false);

    //Put the register to be sent into FIFO
    I2CMasterDataPut(I2C1_BASE, dataByte);

    //Initiate send of data from the MCU
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    // Wait until MCU is done transferring.
    while(I2CMasterBusy(I2C1_BASE));
    
}


