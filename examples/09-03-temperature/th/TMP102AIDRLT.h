//*****************************************************************************
// fxos8700cq.h - Prototypes for the TMP102 Peripheral
//
//  Created on: Nov 25, 2019
//      Author: Brandon Dixon
//
//*****************************************************************************

#ifndef TMP102AIDRLT_H_
#define TMP102AIDRLT_H_

//*****************************************************************************
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
// Register addreses
//*****************************************************************************
#define PR_TEMP_REG         0x00                //Pointer Register -> Points to
#define PR_CONFIG_REG       0x01
#define PR_TEMP_LOW         0x02
#define PR_TEMP_HIGH        0x03

    
//-----------------------------------------------------------
//---Masks used for flipping configuration bits
//-----------------------------------------------------------
//Byte 1 Register    
#define CONFIG_SD_BIT             0X01 
#define CONFIG_TM_BIT             0x02
#define CONFIG_POL_BIT            0X04
#define CONFIG_F0_BIT             0X08
#define CONFIG_F1_BIT             0X10
#define CONFIG_R0_BIT             0X28
#define CONFIG_R1_BIT             0X40
#define CONFIG_OS_BIT             0X80

//Byte 2 Register    
#define CONFIG_EM_BIT             0X10
#define CONFIG_AL_BIT             0X20
#define CONFIG_CONV_CR0_BIT       0X40  
#define CONFIG_CONV_CR1_BIT       0X80   


//*****************************************************************************
// Mark the end of the C bindings section for C++ compilers.
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* FXOS8700CQ_H_ */
