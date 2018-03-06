/*
 * Drone.c
 *
 * Created: 15.10.2017 14:14:19
 * Author : flola
 */ 


/*

// Enable IO
PIOB->PIO_PER = PIO_PB27;
// Set to output
PIOB->PIO_OER = PIO_PB27;
// Disable pull-up
PIOB->PIO_PUDR = PIO_PB27;
PIOB->PIO_CODR = PIO_PB27;

*/


#include "sam.h"
#include "BNO055.h"
#include "ESCControl.h"
#include "RCReader.h"
#include "PID.h"
#include "SerialCOM.h"
#include "FlashStorage.h"

#define BNO_MEASURE BNO_REG_GRV_DATA

BNO055_Data SensorValues;
//Init variables for the drone programm
RemoteControlValues RemoteValues;
int16_t Motor_speeds[4] = {0};
float ValueMapFactor = 0.3;
uint8_t maximumControlDegree = 10;

//PID Config:
float PID_PitchInput = 0,	PID_PitchOutput = 0,	PID_PitchSetPoint = 0;
float PID_RollInput = 0,	PID_RollOutput = 0,		PID_RollSetPoint = 0;
float PID_YawInput = 0,		PID_YawOutput = 0,		PID_YawSetPoint = 0;

float PitchKp = 0.1,  PitchKi = 0.05,    PitchKd = 0.05;
float RollKp = 0.1,  RollKi = 0.05,    RollKd = 0.05;
float YawKp = 0.1,    YawKi = 0.1,    YawKd = 0.1;
pidData PitchPid;
pidData RollPid;
pidData YawPid;

#define MAX_ERROR_COUNT 20
StatusCode errorStack[MAX_ERROR_COUNT] = {0};
uint8_t errorCount = 0;

//sonsor offsetValues:
float sensorOffsetY = 39;
float sensorOffsetX = -28;

//Status variables for whitch values are printing:
bool printSensorValues = 0;
bool printRCValues = 0;
bool printMotorValues = 0;
uint8_t sendCount = 0;

bool ArmMotors = false;

void error_handler_in(StatusCode errorIn)
{
	if(errorIn != SUCCESS && errorCount < MAX_ERROR_COUNT)
	{
		errorStack[errorCount++] = errorIn;
	}
}

void error_handler_print()
{
	if(errorCount != 0)
	{
		char ErrorToCharBuffer[10] = "";
		for (uint8_t i = 0; i < errorCount; i++)
		{
			itoa(errorStack[i], ErrorToCharBuffer, 16);
			ErrorToCharBuffer[2]='\0';
			SerialCOM_put_error(ErrorToCharBuffer);
		}
		errorCount = 0;
	}
}

void configure_wdt(void)
{
	WDT->WDT_MR = 0x00000000; // disable WDT
}

void BNO_Error(BNO_STATUS_BYTES Error, StatusCode Transmit_error_code)
{
	if(Error == BNO_STATUS_BUS_OVER_RUN_ERROR && Transmit_error_code == BNO055_ERROR)
		error_handler_in(BNO055_start_measurement(true,true,BNO_MEASURE));
	else 
	{
		_Delay(840000);
		error_handler_in(BNO055_ERROR);
		error_handler_in(Error);
	}
}

void SaveValuesToFlash(uint8_t type)
{
	if(type == 0x00 || type == 0x01)
		error_handler_in(FlashStorage_write_float(4, RollKp));
	if(type == 0x00 || type == 0x02)
		error_handler_in(FlashStorage_write_float(8, RollKi));
	if(type == 0x00 || type == 0x03)
		error_handler_in(FlashStorage_write_float(12, RollKd));
	if(type == 0x00 || type == 0x04)
		error_handler_in(FlashStorage_write_float(16, PitchKp));
	if(type == 0x00 || type == 0x05)
		error_handler_in(FlashStorage_write_float(20, PitchKi));
	if(type == 0x00 || type == 0x06)
		error_handler_in(FlashStorage_write_float(24, PitchKd));
	if(type == 0x00 || type == 0x07)
		error_handler_in(FlashStorage_write_float(28, YawKp));
	if(type == 0x00 || type == 0x08)
		error_handler_in(FlashStorage_write_float(32, YawKi));
	if(type == 0x00 || type == 0x09)
		error_handler_in(FlashStorage_write_float(36, YawKd));
	if(type == 0x00 || type == 0x0A)
		error_handler_in(FlashStorage_write_float(40, sensorOffsetX));
	if(type == 0x00 || type == 0x0B)
		error_handler_in(FlashStorage_write_float(44, sensorOffsetY));
}

void LoadValuesFromFlash()
{
	RollKp = FlashStorage_read_float(4);
	RollKi = FlashStorage_read_float(8);
	RollKd = FlashStorage_read_float(12);
	PitchKp = FlashStorage_read_float(16);
	PitchKi = FlashStorage_read_float(20);
	PitchKd = FlashStorage_read_float(24);
	YawKp = FlashStorage_read_float(28);
	YawKi = FlashStorage_read_float(32);
	YawKd = FlashStorage_read_float(36);
	sensorOffsetX = FlashStorage_read_float(40);
	sensorOffsetY = FlashStorage_read_float(44);
}

void DataReady()
{
	/*static uint16_t count1 = 0;
	if(count1++ >= 100)
	{
		error_handler_in(SerialCOM_put_debug("S"));
		count1 = 0;
	}*/
	static bool GearStateOld = false;
	bool needComputePitch = PID_need_compute(&PitchPid);
	bool needComputeRoll = PID_need_compute(&RollPid);
	if(needComputeRoll == true || needComputePitch == true)
	{
		
		//read Values from sensor and remote control:
		SensorValues = BNO055_get_measurement_data();

		RemoteValues = rc_read_values();
		if(GearStateOld == false && RemoteValues.Gear == true && RemoteValues.Throttle == 0)
		{
			GearStateOld = true;
			ArmMotors = true;
		} else if(RemoteValues.Gear == false)
		{
			ArmMotors = false;
			GearStateOld = false;
		}
		
		if(RemoteValues.error != true)
		{
			BNO055_Data CorrectedValues;
			CorrectedValues.X	= SensorValues.X - sensorOffsetX;
			CorrectedValues.Y	= SensorValues.Y - sensorOffsetY;
			CorrectedValues.Z	= SensorValues.Z;
			
			PID_PitchInput		= CorrectedValues.X;
			PID_RollInput		= CorrectedValues.Y;
			
			PID_PitchSetPoint	= map(RemoteValues.Pitch, 0, 2200, -50, 50);
			PID_RollSetPoint	= map(RemoteValues.Roll, 0, 2200, -50, 50);
			
			// Compute new PID output value
			if (needComputePitch)
				error_handler_in(PID_Compute(&PitchPid));
			if (needComputeRoll)
				error_handler_in(PID_Compute(&RollPid));
			//float factor = 0.0005;
			//int16_t PitchAdjust = PID_PitchInput*2;//*(factor*RemoteValues.Throttle); 
			//int16_t RollAdjust = PID_RollInput*2;//*(factor*RemoteValues.Throttle);
			
			Motor_speeds[0] = RemoteValues.Throttle - PID_PitchOutput - PID_RollOutput;// - MappedYaw;
			Motor_speeds[1] = RemoteValues.Throttle - PID_PitchOutput + PID_RollOutput;// + MappedYaw;
			Motor_speeds[2] = RemoteValues.Throttle + PID_PitchOutput - PID_RollOutput;// - MappedYaw;
			Motor_speeds[3] = RemoteValues.Throttle + PID_PitchOutput + PID_RollOutput;// + MappedYaw;
			
			/*
			Motor_speeds[0] = RemoteValues.Throttle;
			Motor_speeds[1] = RemoteValues.Throttle;
			Motor_speeds[2] = RemoteValues.Throttle;
			Motor_speeds[3] = RemoteValues.Throttle;
			*/			
			
			if(Motor_speeds[0] < 0)
				Motor_speeds[0] = 0;
			if(Motor_speeds[1] < 0)
				Motor_speeds[1] = 0;
			if(Motor_speeds[2] < 0)
				Motor_speeds[2] = 0;
			if(Motor_speeds[3] < 0)
				Motor_speeds[3] = 0;
			
			if(ArmMotors == true)
			{
				error_handler_in(esc_set(1, Motor_speeds[0]));
				error_handler_in(esc_set(2, Motor_speeds[1]));
				error_handler_in(esc_set(3, Motor_speeds[2]));
				error_handler_in(esc_set(4, Motor_speeds[3]));
			}
			else
			{
				error_handler_in(esc_set(1, 0));
				error_handler_in(esc_set(2, 0));
				error_handler_in(esc_set(3, 0));
				error_handler_in(esc_set(4, 0));
			}
			
			if(SerialCOM_get_free_space() >= printMotorValues + printRCValues + printSensorValues && sendCount++ >= 10)
			{
				
				sendCount = 0;
				if(printMotorValues == true)
				{
					uint8_t buffer[12] = {0};
					buffer[0] = '0';
					buffer[1] = (Motor_speeds[0] & 0xFF00) >> 8;
					buffer[2] = Motor_speeds[0] & 0x00FF;
					buffer[3] = '1';
					buffer[4] = (Motor_speeds[1] & 0xFF00) >> 8;
					buffer[5] = Motor_speeds[1] & 0x00FF;
					buffer[6] = '2';
					buffer[7] = (Motor_speeds[2] & 0xFF00) >> 8;
					buffer[8] = Motor_speeds[2] & 0x00FF;
					buffer[9] = '3';
					buffer[10] = (Motor_speeds[3] & 0xFF00) >> 8;
					buffer[11] = Motor_speeds[3] & 0x00FF;
					SerialCOM_put_message(buffer, 0x01, 12);
				}
				
				if(printRCValues == true)
				{
					uint8_t buffer[14] = {0};
					buffer[0] = 'T';
					buffer[1] = (RemoteValues.Throttle & 0xFF00) >> 8;
					buffer[2] = RemoteValues.Throttle & 0x00FF;
					buffer[3] = 'R';
					buffer[4] = (RemoteValues.Roll & 0xFF00) >> 8;
					buffer[5] = RemoteValues.Roll & 0x00FF;
					buffer[6] = 'P';
					buffer[7] = (RemoteValues.Pitch & 0xFF00) >> 8;
					buffer[8] = RemoteValues.Pitch & 0x00FF;
					buffer[9] = 'Y';
					buffer[10] = (RemoteValues.Yaw & 0xFF00) >> 8;
					buffer[11] = RemoteValues.Yaw & 0x00FF;
					buffer[12] = 'G';
					buffer[13] = RemoteValues.Gear;
					SerialCOM_put_message(buffer, 0x02, 14);
				}
				
				if(printSensorValues == true)
				{
					uint8_t buffer[12] = {0};
					bool XIsNegative = CorrectedValues.X < 0;
					bool YIsNegative = CorrectedValues.Y < 0;
					bool ZIsNegative = CorrectedValues.Z < 0;
					
					buffer[0] = 'X';
					buffer[1] = XIsNegative;
					if(XIsNegative)
					{
						buffer[2] = (CorrectedValues.X*-1 & 0xFF00) >> 8;
						buffer[3] = CorrectedValues.X*-1 & 0x00FF;
					}
					else
					{
						buffer[2] = (CorrectedValues.X & 0xFF00) >> 8;
						buffer[3] = CorrectedValues.X & 0x00FF;
					}
					
					buffer[4] = 'Y';
					buffer[5] = YIsNegative;
					if(YIsNegative)
					{
						buffer[6] = (CorrectedValues.Y*-1 & 0xFF00) >> 8;
						buffer[7] = CorrectedValues.Y*-1 & 0x00FF;
					}
					else
					{
						buffer[6] = (CorrectedValues.Y & 0xFF00) >> 8;
						buffer[7] = CorrectedValues.Y & 0x00FF;
					}
					
					buffer[8] = 'Z';
					buffer[9] = ZIsNegative;
					if(ZIsNegative)
					{
						buffer[10] = (CorrectedValues.Z*-1 & 0xFF00) >> 8;
						buffer[11] = CorrectedValues.Z*-1 & 0x00FF;
					}
					else
					{
						buffer[10] = (CorrectedValues.Z& 0xFF00) >> 8;
						buffer[11] = CorrectedValues.Z & 0x00FF;
					}
					
					SerialCOM_put_message(buffer, 0x03, 12);
				}
			}
		}
	}
}

void sendPIDValuesToPC(uint8_t PIDIdentifier, float kp, float ki, float kd)
{
	uint8_t buffer[16] = {0};
	uint32_t NumValue;
	
	buffer[0] = PIDIdentifier;
	buffer[1] = 'P';
	memcpy(&NumValue, &kp, 4);
	/*char temp[20] = "";
	sprintf(temp,"%.4f: %lu", kp, NumValue);
	SerialCOM_put_debug(temp);*/
	buffer[2] = (NumValue & 0xFF000000) >> 24;
	buffer[3] = (NumValue & 0x00FF0000) >> 16;
	buffer[4] = (NumValue & 0x0000FF00) >> 8;
	buffer[5] =  NumValue & 0x000000FF;
	
	buffer[6] = 'I';
	memcpy(&NumValue, &ki, 4);
	buffer[7] = (NumValue & 0xFF000000) >> 24;
	buffer[8] = (NumValue & 0x00FF0000) >> 16;
	buffer[9] = (NumValue & 0x0000FF00) >> 8;
	buffer[10]=  NumValue & 0x000000FF;
	
	buffer[11] = 'D';
	memcpy(&NumValue, &kd, 4);
	buffer[12] = (NumValue & 0xFF000000) >> 24;
	buffer[13] = (NumValue & 0x00FF0000) >> 16;
	buffer[14] = (NumValue & 0x0000FF00) >> 8;
	buffer[15] =  NumValue & 0x000000FF;
	SerialCOM_put_message(buffer, 0x04, 16);
}

//Receive:
//0x01: Motor
//0x02: RC
//0x03: Sensor
//0x04: Set current values as Sensor offset
//0x05: Send PID Values to PC
//0x06: Get Value from PC
//0x07: Save Values to Flash

//Send:
//0x00: Debug message
//0x01: Motor Values
//0x02: RC Values
//0x03: Sensor Values
//0x04: PID Values
//0x05: Reset GUI
//0x06: ACK
//0x64: Error Message
void message_from_PC(uint8_t* message, uint8_t Type)
{
	//error_handler_in(SerialCOM_put_debug("GR"));
	float kp, ki, kd;
	uint32_t Value;
	switch(Type)
	{
		case 0x01:
			if(message[0] == 'I')
				printMotorValues = true;
			else if(message[0] == 'O')
				printMotorValues = false;
		break;
		case 0x02:
			if(message[0] == 'I')
				printRCValues = true;
			else if(message[0] == 'O')
				printRCValues = false;
		break;
		case 0x03:
			if(message[0] == 'I')
				printSensorValues = true;
			else if(message[0] == 'O')
				printSensorValues = false;
		break;
		case 0x04:
			if(message[0] == 'S')
			{
				sensorOffsetX = SensorValues.X;
				sensorOffsetY = SensorValues.Y;
				//SaveValuesToFlash(0x0A);
				//SaveValuesToFlash(0x0B);
				PID_Reset(&RollPid);
				PID_Reset(&PitchPid);
			} 
			else
			{
				error_handler_in(SerialCOM_put_error("SetSensorOffsets command has errors"));
			}
		break;
		case 0x05:
			if(message[0] == 'R')
			{
				sendPIDValuesToPC('R', RollKp, RollKi, RollKd);
			} else if(message[0] == 'P')
			{
				sendPIDValuesToPC('P', PitchKp, PitchKi, PitchKd);
			} else if(message[0] == 'Y')
			{
				sendPIDValuesToPC('Y', YawKp, YawKi, YawKd);
			}
			else
			{
				error_handler_in(SerialCOM_put_error("SendPIDValues command has errors"));
			}
			//restart BNO measurement if necessary
			if(BNO055_is_busy() == false)
			{
				error_handler_in(BNO055_start_measurement(true, true, BNO_MEASURE));
				error_handler_in(SerialCOM_put_debug("Restart BNO"));
			}
		break;
		case 0x06:
			//check message integrity:
			if(message[1] == 'P' && message[6] == 'I' && message[11] == 'D')
			{
				Value  = ((message[2] << 24) | (message[3] << 16) | (message[4] << 8) | message[5]);
				memcpy(&kp, &Value, 4);
				Value  = ((message[7] << 24) | (message[8] << 16) | (message[9] << 8) | message[10]);
				memcpy(&ki, &Value, 4);
				Value  = ((message[12] << 24) | (message[13] << 16) | (message[14] << 8) | message[15]);
				memcpy(&kd, &Value, 4);
				
				switch(message[0])
				{
					case 'P':
						PitchKd = kd;
						PitchKi = ki;
						PitchKp = kp;
						PID_SetTunings(&PitchPid, PitchKd, PitchKi, PitchKp);
					break;
					case 'R':
						RollKd = kd;
						RollKi = ki;
						RollKp = kp;
						PID_SetTunings(&RollPid, RollKd, RollKi, RollKp);
					break;
					case 'Y':
						YawKd = kd;
						YawKi = ki;
						YawKp = kp;
						PID_SetTunings(&YawPid, YawKd, YawKi, YawKp);
					break;
					default:
						error_handler_in(SerialCOM_put_error("SetPIDValues command has a wrong PID identifier"));
					break;
				}
			} 
			else
			{
				error_handler_in(SerialCOM_put_error("SetPIDValues command has errors"));
			}
		break;
		case 0x07:
			if(message[0] == 'S')
			{
				//BNO055_stop_continous_measurement();	//stop BNO measurement
				//while(BNO055_is_busy() == true);		//wait for a bit to make sure that all measurements are finished
				SaveValuesToFlash(0x00);				//save Values
				//BNO055_start_measurement(true,true,BNO_MEASURE);	//resume Measurement
				error_handler_in(SerialCOM_put_debug("Saved values to flash"));
			}else
			{
				error_handler_in(SerialCOM_put_error("SaveToFlash command has errors"));
			}
		break;
		default:
			error_handler_in(SerialCOM_put_error("Unknown command!"));
		break;
	}
}



void config_BNO()
{
	error_handler_in(SerialCOM_put_debug("Start BNO Init"));
	error_handler_in(BNO055_init_fusion_mode(false));
	error_handler_in(SerialCOM_put_debug("Calib OK"));
	error_handler_in(BNO055_register_error_callback(BNO_Error));
	error_handler_in(BNO055_register_data_ready_callback(DataReady));
	//configure MAG
	/*error_handler_in(BNOCOM_write_and_wait_for_response_1byte(BNO_REG_PAGE_ID, 0, BNO_PAGE_ID1));
	uint8_t config = 0x07	//30Hz data rate
					|0x18	//High accuracy data
					|0x00;	//power mode normal
	error_handler_in(BNOCOM_write_and_wait_for_response_1byte(BNO_REG1_MAG_CONFIG, 1, config));
	error_handler_in(BNOCOM_write_and_wait_for_response_1byte(BNO_REG_PAGE_ID, 1, BNO_PAGE_ID0));*/
}

int main(void)
{
	SystemInit();
	configure_wdt();
	error_handler_in(SerialCOM_init());
	error_handler_in(SerialCOM_register_call_back(message_from_PC));
	error_handler_in(SerialCOM_put_Command('R', 0x05));			//Reset GUI
	error_handler_in(FlashStorage_Init());

	if(FlashStorage_read(0))
	{
		error_handler_in(SerialCOM_print_debug("Running for the first time, populating Flash with default values"));
		SaveValuesToFlash(0x00);
		//LoadValuesFromFlash();
		error_handler_in(FlashStorage_write_uint8_t(0,0));
	}else
	{
		error_handler_in(SerialCOM_print_debug("loading values from flash"));		
		LoadValuesFromFlash();
	}
	_Delay(8400000);
	config_BNO();
	
	//rc control and esc init:
	rc_init();
	esc_init();
	
	PID_Init();
	error_handler_in(PID_Initialize(&PitchPid, &PID_PitchInput, &PID_PitchOutput, &PID_PitchSetPoint, PitchKp, PitchKi, PitchKd,-250,250,10));
	error_handler_in(PID_Initialize(&RollPid, &PID_RollInput, &PID_RollOutput, &PID_RollSetPoint, RollKp, RollKi, RollKd,-250,250,10));
	error_handler_in(BNO055_start_measurement(true, true, BNO_MEASURE));
	error_handler_print();
	error_handler_in(SerialCOM_put_debug("Init Done!"));
	while(1)
	{
		error_handler_print();
		//if(UART0_has_space())
		{
			//error_handler_in(SerialCOM_put_debug("This message tests the correctness of the memmory management"));
			
		}
		//_Delay(1000000);
	}
}