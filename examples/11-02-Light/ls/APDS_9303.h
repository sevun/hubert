//*****************************************************************************
// APDS_9303.h - Prototypes for the TMP102 Peripheral
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
// Register addreses
//*****************************************************************************
#define CONTROL_REGISTER			   		    0x80
#define TIMING_REGISTER				       		0x81
#define THRESHOLD_LOW_LOW 		           		0x82
#define THRESHOLD_LOW_HIGH      		   		0x83
#define THRESHOLD_HIGH_LOW                		0x84
#define THRESHOLD_HIGH_HIGH						0x85
#define INTERRUPT_CONTROL						0x86
#define SERIAL_ID								0x8A
#define DATA_0_LOW								0x8C
#define DATA_0_HIGH								0x8D
#define DATA_1_LOW								0x8E
#define DATA_1_HIGH								0x8F

#define SENSOR_POWER_UP							0X03
#define SENSOR_POWER_DOWN						0X00

#define LOW_GAIN								0X00
#define HIGH_GAIN								0X01

#define CLEAR_INTERRUPT							0XC0
#define ADC_CHAN_0								0X00   
#define ADC_CHAN_1								0X01

#define INTEG_TIME_13_7_MS				        137
#define INTEG_TIME_101_MS						101
#define INTEG_TIME_402_MS						402


    
//-----------------------------------------------------------
//---Functions
//-----------------------------------------------------------

typedef enum
{
    LIGHT_DATA,
    LUX_DATA
} tDataType;

typedef enum 
{
	POWER_ON,
	POWER_OFF
} tPowerStatus;

typedef enum name
{
	INTERRUPT_ON,
	INTERRUPT_OFF,
	INTERRUPT_CLEAR
} tInterruptStatus;

typedef enum 
{
	INTEGRATION_TIME_13_7_MS,
	INTEGRATION_TIME_101_MS, 
	INTEGRATION_TIME_402_MS
} tIntegrationTime;

typedef enum 
{
	GAIN_LOW,
	GAIN_HIGH	
} tGainLevel;

typedef struct 
{
	uint8_t ui8Chan_0_0;
	uint8_t ui8Chan_0_1;
	uint8_t ui8Chan_1_0;
	uint8_t ui8Chan_1_1;
	uint16_t f16Chan_0;
	uint16_t f16Chan_1;
	uint16_t f16TempChan_0;
	uint16_t f16TempChan_1;
	float fChan_0;
	float fChan_1;
	float fRatio;
	float luxVal;
} t_fRawData;


typedef struct
{
	uint8_t ui8ResultVal;
    int16_t i16ResultVal;
} t_RawData;

typedef struct
{
    uint8_t ui8ArrVal[2];
} t_aRawData;

typedef struct 
{
	uint16_t integTimeSetting;
} t_IntegrationTimeSetting;

typedef struct 
{
	uint8_t gainSetting;
} t_GainSetting;



// I2C General Send and receive functions
extern void I2CHReceive(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
                    uint8_t *pReceiveData, uint8_t ui8NumBytes);
extern void I2CGetRegister(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
                    uint8_t *pReceiveData, uint8_t ui8NumBytes);
extern void I2CGetDatabyte(uint32_t ui32SlaveAddress, uint8_t *pReceiveData);
extern void I2CHSend(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister,
                    uint8_t *pTransmitData, uint8_t ui8NumBytes);
extern void I2CHSendCmdByte(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister);
extern void I2CHSendDataByte(uint32_t ui32SlaveAddress, uint8_t ui32SlaveRegister);

bool APDS_9303_getID(uint32_t ui32SlaveAddress, tDataType tDT, t_RawData *tRD);
uint8_t APDS_9303_getLuxData(uint32_t ui32SlaveAddress, tDataType tDT, t_IntegrationTimeSetting *tDTI, t_fRawData *tRD, tGainLevel tRDT);
bool APDS_9303_init(uint32_t ui32SlaveAddress, t_RawData *tRD);
bool APDS_9303_PowerEnable(uint32_t ui32SlaveAddress, tPowerStatus tDT);
uint8_t APDS_9303_SetGain(uint32_t ui32SlaveAddress, tGainLevel tDT, t_GainSetting *tRD);
uint8_t APDS_9303_GetGain(uint32_t ui32SlaveAddress);
bool APDS_9303_SetIntegrationTime(uint32_t ui32SlaveAddress, tIntegrationTime tDT, t_IntegrationTimeSetting *tDTI);
bool APDS_9303_ClearInterruptFlag(uint32_t ui32SlaveAddress, tInterruptStatus tDT);
bool APDS_9303_EnableInterruptMode(uint32_t ui32SlaveAddress, tInterruptStatus tDT, t_RawData *tRD);
bool APDS_9303_SetLowThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD);
bool APDS_9303_SetHighThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD);
bool APDS_9303_GetLowThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD);
bool APDS_9303_GetHighThreshold(uint32_t ui32SlaveAddress, t_aRawData *tRD);
bool APDS_9303_SetInterruptCycles(uint32_t ui32SlaveAddress, uint8_t cycles, t_RawData *tRD);



//*****************************************************************************
// Mark the end of the C bindings section for C++ compilers.
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* APDS_9303_H_ */
