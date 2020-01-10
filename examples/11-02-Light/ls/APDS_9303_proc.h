//*****************************************************************************
// TMP102AIDRLT_proc.h - Prototypes for the APDS_9303 Peripheral
//
//  Created on: Dec 23, 2019
//      Author: Brandon Dixon
//
//*****************************************************************************

#ifndef APDS_9303_H_
#define APDS_9303_H_

//*****************************************************************************
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
// Functions
//*****************************************************************************

// I2C General Send and receive functions
void I2CHReceive(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
                    uint8_t *pReceiveData, uint8_t ui8NumBytes);
void I2CHSend(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
                    uint8_t *pTransmitData, uint8_t ui8NumBytes);


//*****************************************************************************
// Mark the end of the C bindings section for C++ compilers.
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* APDS_9303_H_ */
