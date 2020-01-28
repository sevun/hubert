//*****************************************************************************
// fxos8700cq.c - Prototypes for the APDS_9303 Peripheral
//
//  Created on: Dec 22, 2019
//      Author: Brandon Dixon
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "APDS_9303.h"


//functions
bool APDS_9303_getID(uint32_t ui32SlaveAddress, tDataType tDT, t_RawData *tRD)
{
	
	if (LIGHT_DATA == tDT)
    {    	
        
	    uint8_t ui8Register[1];
	    
	    I2CHSendCmdByte(ui32SlaveAddress, SERIAL_ID);
        I2CGetDatabyte(ui32SlaveAddress, ui8Register);

		// copy the 8 bit ID byte data to pointer
        tRD->ui8ResultVal = ui8Register[0];

        return 1;
    }
    else
    {
    	return 0;
    }


}





uint8_t APDS_9303_getLuxData(uint32_t ui32SlaveAddress, tDataType tDT, t_IntegrationTimeSetting *tDTI, t_fRawData *tRD, tGainLevel tRDT)
{
	
	uint8_t ui8Register[1];   
	uint8_t ui8Register_1[1];  
	uint8_t ui8Register_2[1];
	float tempAnalogVal_0 = 0.0, tempAnalogVal_1 = 0.0, analogRatio = 0.0;
	uint16_t tempVal = 0, tempVal_1 = 0, tempVal_2 = 0;	
	bool err = 0;
	

    if (LUX_DATA == tDT)
    {
    	//read two registers so we need to send the command code with the register
        //uint8_t CMD_READREG = DATA_0_LOW | 0x20;
        I2CHSendCmdByte(ui32SlaveAddress, DATA_0_LOW);
        I2CGetDatabyte(ui32SlaveAddress, ui8Register);

        I2CHSendCmdByte(ui32SlaveAddress, DATA_0_HIGH);
        I2CGetDatabyte(ui32SlaveAddress, ui8Register_1);

        tRD->ui8Chan_0_0 = ui8Register[0];
        tRD->ui8Chan_0_1 = ui8Register_1[0];

        // copy the 16 bit channel 0 data to float variable
        tempVal = (uint16_t)( (ui8Register_1[0] << 8) | (ui8Register[0]) );
        tRD->f16Chan_0 = tempVal;
        tempAnalogVal_0 = (float)tempVal;

		ui8Register[0] = 0;
		ui8Register_1[0] = 0;

		//read two registers so we need to send the command code with the register
        //uint8_t CMD_READREG = DATA_0_HIGH | 0x20;
        I2CHSendCmdByte(ui32SlaveAddress, DATA_1_LOW);
        I2CGetDatabyte(ui32SlaveAddress, ui8Register);

        I2CHSendCmdByte(ui32SlaveAddress, DATA_1_HIGH);
        I2CGetDatabyte(ui32SlaveAddress, ui8Register_1);

        tRD->ui8Chan_1_0 = ui8Register[0];
        tRD->ui8Chan_1_1 = ui8Register_1[0];
       

        // copy the 16 bit channel 1 data to float variable
        tempVal_1 = (uint16_t)( (ui8Register_1[0] << 8) | (ui8Register[0]) );
        tRD->f16Chan_1 = tempVal_1;
        tempAnalogVal_1 = (float)tempVal_1;

        

        tRD->fChan_0 = (float)tempVal;
        tRD->fChan_1 = (float)tempVal_1;
        tRD->f16TempChan_0 = tempVal;
        tRD->f16TempChan_1 = tempVal_1;


        //check that count for the selected integration time is within specs
        if(tDTI->integTimeSetting == INTEG_TIME_13_7_MS){
        	if( (tempVal >= 5047) || (tempVal_1 >= 5047) ){ //maximum count for this integration setting
        		tRD->f16Chan_0 = 5047.0;
        		tRD->f16Chan_1 = 5047.0;
        		err = 1;
        		return 2;
        	}
        }
        else if(tDTI->integTimeSetting == INTEG_TIME_101_MS){
        	if( (tempVal >= 37177) || (tempVal_1 >= 37177) ){ //maximum count for this integration setting
        		tRD->f16Chan_0 = 37177.0;
        		tRD->f16Chan_1 = 37177.0;
        		err = 1;
        		return 3;
        	}
        }
        else{
        	if( (tempVal >= 65535) || (tempVal_1 >= 65535) ){ //maximum count for this integration setting
        		tRD->f16Chan_0 = 65535.0;
        		tRD->f16Chan_1 = 65535.0;
        		err = 1;
        		return 4;
        	}
        }


		tRD->fRatio = 0;


        if(!err){

        	//calculate the ration of the two analog values
	        analogRatio = tempAnalogVal_1 / tempAnalogVal_0;
	        tRD->fRatio = analogRatio;

	        //mutliply by the coefficient for the integration time
	        if(tDTI->integTimeSetting == INTEG_TIME_13_7_MS){
	        	tempAnalogVal_0 *= 1/0.034;
	        	tempAnalogVal_1 *= 1/0.034;
	        	tRD->fChan_0 = tempAnalogVal_0;
	        	tRD->fChan_1 = tempAnalogVal_1;
	        	//return 5;
	        }
	        else if(tDTI->integTimeSetting == INTEG_TIME_101_MS){
	        	tempAnalogVal_0 *= 1/0.252;
	        	tempAnalogVal_1 *= 1/0.252;
	        	tRD->fChan_0 = tempAnalogVal_0;
	        	tRD->fChan_1 = tempAnalogVal_1;
	        	//return 6;
	        }
	        else{
	        	tempAnalogVal_0 *= 1;
	        	tempAnalogVal_1 *= 1;
	        	tRD->fChan_0 = tempAnalogVal_0;
	        	tRD->fChan_1 = tempAnalogVal_1;
	        	//return 7;
	        }

	       			
		    I2CHSendCmdByte(ui32SlaveAddress, TIMING_REGISTER);
			I2CGetDatabyte(ui32SlaveAddress, ui8Register_2);

			tempVal_2 = ui8Register_2[0];
			tempVal_2 &=~ 0x10;

			if(!tempVal_2){ //gain is set to low

				tempAnalogVal_0 *= 16;
	        	tempAnalogVal_1 *= 16;
	        	tRD->fChan_0 = tempAnalogVal_0;
	        	tRD->fChan_1 = tempAnalogVal_1;				
			}
		    else{
		    	//gain is high already
		    }	     

	        //calculate lub levels according to formulas described on pg 4 of datasheet
	        if(analogRatio <= 0.52){
	        	tRD->luxVal = ( (0.0315 * tempAnalogVal_0) - (0.0593 * tempAnalogVal_0 * (pow(analogRatio, 1.4)) ) );
	        	return 9;
	        }
	        else if( (analogRatio > 0.52) && (analogRatio <= 0.65) ){
	        	tRD->luxVal = ( (0.0224 * tempAnalogVal_0) - (0.031 * tempAnalogVal_1) );
	        	return 10;
	        }
	        else if( (analogRatio > 0.65) && (analogRatio <= 0.80) ){
	        	tRD->luxVal = ( (0.0157 * tempAnalogVal_0) - (0.018 * tempAnalogVal_1) );
	        	return 11;
	        }
	        else if( (analogRatio > 0.8) && (analogRatio <= 1.30) ){
	        	tRD->luxVal = ( (0.00338 * tempAnalogVal_0) - (0.0026 * tempAnalogVal_1) );
	        	return 12;
	        }
	        else if(analogRatio > 1.30){
	        	tRD->luxVal = 0.0;
	        	return 13;
	        }
	        else{
	        	return 0;
	        }

        }
        else{
        	return 5;
        }

      
	    
    }
    else
    {
    	return 0;
    } 
}


bool APDS_9303_init(uint32_t ui32SlaveAddress, t_RawData *tRD){

	uint8_t ui8Register[1];
	uint8_t ui8Register_1[1];
	uint8_t tempVal;
	ui8Register[0] = SENSOR_POWER_UP;
    
    //set the light sensor to power up on POR
	//I2CHSend(ui32SlaveAddress, CONTROL_REGISTER, ui8Register, sizeof(ui8Register));	
    I2CHSendCmdByte(ui32SlaveAddress, CONTROL_REGISTER);
    I2CHSendDataByte(ui32SlaveAddress, SENSOR_POWER_UP);

	//SysCtlDelay(500); // writing time	

	//I2CGetRegister(ui32SlaveAddress, CONTROL_REGISTER, ui8Register_1, sizeof(ui8Register_1));
	I2CHSendCmdByte(ui32SlaveAddress, CONTROL_REGISTER);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register_1);

	tRD->ui8ResultVal = ui8Register_1[0];   

	return 1;
}



bool APDS_9303_PowerEnable(uint32_t ui32SlaveAddress, tPowerStatus tDT){	

	uint8_t ui8Register[1];
	uint8_t tempVal = 0;

	if(POWER_ON==tDT){
		tempVal = SENSOR_POWER_UP;
	}
	else if(POWER_OFF==tDT){
		tempVal = SENSOR_POWER_DOWN;
	}	
	else{
		return 0;
	}
    
    //set the light sensor to power up on POR
	I2CHSendCmdByte(ui32SlaveAddress, CONTROL_REGISTER);
    I2CHSendDataByte(ui32SlaveAddress, tempVal);

	return 1;

}


uint8_t APDS_9303_SetGain(uint32_t ui32SlaveAddress, tGainLevel tDT, t_GainSetting *tRD){

	uint8_t ui8Register[1];
	uint8_t tempVal = 0x00;
	//I2CGetRegister(ui32SlaveAddress, TIMING_REGISTER, ui8Register, sizeof(ui8Register));	
    I2CHSendCmdByte(ui32SlaveAddress, TIMING_REGISTER);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);

	tempVal = ui8Register[0];
    
	if(GAIN_LOW == tDT){
		tempVal &=~ 0x10;
		tRD->gainSetting = LOW_GAIN;	
	}
	else if(GAIN_HIGH == tDT){
		tempVal |= 0x10;
		tRD->gainSetting = HIGH_GAIN;		
	}
	else{}	

	ui8Register[0] = tempVal;

    //set the light sensor gain level	
    I2CHSendDataByte(ui32SlaveAddress, tempVal);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
			
	tRD->gainSetting = ui8Register[0];

	return 1;	

}



uint8_t APDS_9303_GetGain(uint32_t ui32SlaveAddress){

	uint8_t ui8Register[1];
	uint8_t tempVal = 0;
	//I2CGetRegister(ui32SlaveAddress, TIMING_REGISTER, ui8Register, sizeof(ui8Register));	
    I2CHSendCmdByte(ui32SlaveAddress, TIMING_REGISTER);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);

	tempVal = ui8Register[0];
	tempVal &=~ 0x10;

	if(!tempVal){
		return 0;
	}
    else{
    	return 1;
    }

}



bool APDS_9303_SetIntegrationTime(uint32_t ui32SlaveAddress, tIntegrationTime tDT, t_IntegrationTimeSetting *tDTI){

	uint8_t ui8Register[1];
	uint8_t tempVal;
	I2CHSendCmdByte(ui32SlaveAddress, TIMING_REGISTER);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tempVal = ui8Register[0];
	tempVal &= ~0x03;

    //tDTI->integTimeSetting = (uint16_t)((0x00 << 8) | ui8Register[0]);

	if(INTEGRATION_TIME_13_7_MS == tDT){
		tempVal |= 0x00;
		tDTI->integTimeSetting = INTEG_TIME_13_7_MS;
	}
	else if(INTEGRATION_TIME_101_MS == tDT){
		tempVal |= 0x01;
		tDTI->integTimeSetting = INTEG_TIME_101_MS;
	}
	else if(INTEGRATION_TIME_402_MS == tDT){
		tempVal |= 0x02;
		tDTI->integTimeSetting = INTEG_TIME_402_MS;
	}
	else{
		//return 0;
	}

	ui8Register[0] = tempVal;

	//set the light sensor gain level
	//I2CHSend( ui32SlaveAddress, TIMING_REGISTER, ui8Register, sizeof(ui8Register));	
    I2CHSendDataByte(ui32SlaveAddress, ui8Register);

	return 1;

}


bool APDS_9303_ClearInterruptFlag(uint32_t ui32SlaveAddress, tInterruptStatus tDT){

	//interrupt flag is in the command register so we just write 0xC0 for bit 6 & 7
	I2CHSendCmdByte(ui32SlaveAddress, CLEAR_INTERRUPT);

	return 1;

}


bool APDS_9303_EnableInterruptMode(uint32_t ui32SlaveAddress, tInterruptStatus tDT, t_RawData *tRD){

	uint8_t ui8Register[1];
	uint8_t tempVal;
	I2CHSendCmdByte(ui32SlaveAddress, INTERRUPT_CONTROL);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tempVal = ui8Register[0];

	if(INTERRUPT_ON == tDT){
		tempVal |= 0x10;
	}
	else if(INTERRUPT_OFF == tDT){
		tempVal &=~ 0x30;
	}
	else{
		return 0;
	}

	tRD->ui8ResultVal = 0;

	//set the light sensor interrupt enable	
	I2CHSendCmdByte(ui32SlaveAddress, INTERRUPT_CONTROL);
	I2CHSendDataByte(ui32SlaveAddress, tempVal);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tRD->ui8ResultVal = ui8Register[0];

	return 1;

}



bool APDS_9303_SetLowThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD){

	uint8_t ui8Register[1];
	uint8_t tempVal;

	//set the light sensor low threshold level byte 1 is LOW_LOW and byte 2 is LOW_HIGH
	//uint8_t CMD_REGWRITE = THRESHOLD_LOW_LOW | 0x20;  //send control to write WORD along with register address
	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_LOW_LOW);
	tempVal = tRD->ui8ArrVal[0];
    I2CHSendDataByte(ui32SlaveAddress, tempVal);

	//set command register for LOW_HIGH byte
	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_LOW_HIGH);
	tempVal = tRD->ui8ArrVal[1];
    I2CHSendDataByte(ui32SlaveAddress, tempVal);

    tRD->ui8ArrVal[0] = 0;
    tRD->ui8ArrVal[1] = 0;

    //read back registers to confirm settings are correct
    I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_LOW_LOW);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tRD->ui8ArrVal[0] = ui8Register[0];
    I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_LOW_HIGH);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tRD->ui8ArrVal[1] = ui8Register[0];

	return 1;

}



bool APDS_9303_SetHighThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD){

	uint8_t ui8Register[1];
	uint8_t tempVal;

	//set the light sensor low threshold level byte 1 is LOW_LOW and byte 2 is LOW_HIGH
	//uint8_t CMD_REGWRITE = THRESHOLD_LOW_LOW | 0x20;  //send control to write WORD along with register address
	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_HIGH_LOW);
	tempVal = tRD->ui8ArrVal[0];
    I2CHSendDataByte(ui32SlaveAddress, tempVal);

	//set command register for LOW_HIGH byte
	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_HIGH_HIGH);
	tempVal = tRD->ui8ArrVal[1];
    I2CHSendDataByte(ui32SlaveAddress, tempVal);

    tRD->ui8ArrVal[0] = 0;
    tRD->ui8ArrVal[1] = 0;

    //read back registers to confirm settings are correct
    I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_HIGH_LOW);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tRD->ui8ArrVal[0] = ui8Register[0];
    I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_HIGH_HIGH);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tRD->ui8ArrVal[1] = ui8Register[0];

	return 1;

}


bool APDS_9303_GetLowThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD){

	uint8_t ui8Register[1];
	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_LOW_LOW);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
    tRD->ui8ArrVal[0] = ui8Register[0];

	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_LOW_HIGH);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
    tRD->ui8ArrVal[1] = ui8Register[0];

    return 1;

}


bool APDS_9303_GetHighThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD){

	uint8_t ui8Register[1];
	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_HIGH_LOW);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
    tRD->ui8ArrVal[0] = ui8Register[0];

	I2CHSendCmdByte(ui32SlaveAddress, THRESHOLD_HIGH_HIGH);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
    tRD->ui8ArrVal[1] = ui8Register[0];

    return 1;

}



bool APDS_9303_SetInterruptCycles(uint32_t ui32SlaveAddress, uint8_t cycles, t_RawData *tRD){

	uint8_t ui8Register[1];
	uint8_t tempVal;
	I2CHSendCmdByte(ui32SlaveAddress, INTERRUPT_CONTROL);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tempVal = ui8Register[0];

	tempVal &=~ 0x0F;		//clear lower four bits
	cycles &=~ 0xF0;		//clear the upper four bits to 0
	tempVal |= cycles;		//make the changes to the register

	tRD->ui8ResultVal = 0;

	 //set the light sensor to power up on POR
	I2CHSendCmdByte(ui32SlaveAddress, INTERRUPT_CONTROL);
    I2CHSendDataByte(ui32SlaveAddress, tempVal);
	I2CGetDatabyte(ui32SlaveAddress, ui8Register);
	tRD->ui8ResultVal = ui8Register[0];

	return 1;


}
