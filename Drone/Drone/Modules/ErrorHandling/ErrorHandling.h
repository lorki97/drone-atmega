/*
 * ErrorHandling.h
 *
 * Created: 07.03.2018 16:24:07
 *  Author: flola
 */ 


#ifndef ERRORHANDLING_H_
#define ERRORHANDLING_H_

#include "sam.h"
#include "../../config.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


typedef enum {
										//0xFF0000
									//0xFF00000000
	MODULE_SUCCESS						= 0x000000,
	MODULE_GENERIC						= 0x010000,
	MODULE_MAIN							= 0x020000,
	MODULE_BNO055						= 0x030000,
	MODULE_BNOCOM						= 0x040000,
	MODULE_ESCCONTROL					= 0x050000,
	MODULE_FLASHSTORAGE					= 0x060000,
	MODULE_HELPERFUNCTIONS				= 0x070000,
	MODULE_PID							= 0x080000,
	MODULE_RCREADER						= 0x090000,
	MODULE_SERIALCOM					= 0x0A0000,
	MODULE_UART0						= 0x0B0000,
	MODULE_USART0						= 0x0C0000					//<--
}Modules;

typedef enum {
	FUNCTION_SUCCESS								= 0x0000,
	FUNCTION_GENERIC								= 0x0100,
	FUNCTION_init_fusion_mode						= 0x0200,
	FUNCTION_init_non_fusion_mode					= 0x0300,
	FUNCTION_start_measurement						= 0x0400,
	FUNCTION_calculate_calibration					= 0x0500,
	FUNCTION_register_data_ready_callback			= 0x0600,
	FUNCTION_register_error_callback				= 0x0700,
	FUNCTION_success								= 0x0800,
	FUNCTION_error									= 0x0900,
	FUNCTION_write_and_wait_for_response_1byte		= 0x0A00,
	FUNCTION_read_and_wait_for_response_1byte		= 0x0B00,
	FUNCTION_write_and_wait_for_response			= 0x0C00,
	FUNCTION_read_and_wait_for_response				= 0x0D00,
	FUNCTION_register_write_1byte_by_table			= 0x0E00,
	FUNCTION_register_write_by_table				= 0x0F00,
	FUNCTION_register_read_by_table					= 0x1000,
	FUNCTION_register_read_1byte_by_table			= 0x1100,
	FUNCTION_init									= 0x1200,
	FUNCTION_register_success_callback				= 0x1300,
	FUNCTION_register_read							= 0x1400,
	FUNCTION_register_write							= 0x1500,
	FUNCTION_response_received						= 0x1600,
	FUNCTION_runtime_success						= 0x1700,
	FUNCTION_set									= 0x1800,
	FUNCTION_lock									= 0x1900,
	FUNCTION_unlock									= 0x1A00,
	FUNCTION_write									= 0x1B00,
	FUNCTION_write_unlocked							= 0x1C00,
	FUNCTION_queue_delete							= 0x1D00,
	FUNCTION_queue_write							= 0x1E00,
	FUNCTION_median_filter_new						= 0x1F00,
	FUNCTION_median_filter_add						= 0x2000,
	FUNCTION_Initialize								= 0x2100,
	FUNCTION_Compute								= 0x2200,
	FUNCTION_SetTunings								= 0x2300,
	FUNCTION_SetSampleTime							= 0x2400,
	FUNCTION_SetOutputLimits						= 0x2500,
	FUNCTION_SetControllerDirection					= 0x2600,
	FUNCTION_register_call_back						= 0x2700,
	FUNCTION_put_message							= 0x2800,
	FUNCTION_force_put_message						= 0x2900,
	FUNCTION_put_data								= 0x2A00,
	FUNCTION_puts_blocking							= 0x2B00,
	FUNCTION_set_receiver_length					= 0x2C00,
	FUNCTION_register_received_callback				= 0x2D00,
	FUNCTION_calibrate								= 0x2E00,
	FUNCTION_get_calibration						= 0x2F00,
	FUNCTION_register_write_1byte					= 0x3000,
	FUNCTION_register_read_1byte					= 0x3100,
	FUNCTION_write_uint8_t							= 0x3200,
	FUNCTION_write_float							= 0x3300,
	FUCNTION_put_Command							= 0x3400,
	FUCNTION_put_debug								= 0x3500,
	FUCNTION_put_debug_n							= 0x3600,
	FUCNTION_put_error								= 0x3700,
	FUCNTION_force_put_error						= 0x3800,
	FUNCTION_print_debug							= 0x3900,
	FUNCTION_print_error							= 0x3A00,
	FUNCTION_puts									= 0x3B00,
	FUNCTION_put_float								= 0x3C00,
	FUNCTION_put_int								= 0x3D00,
	FUNCTION_put_int_blocking						= 0x3E00,
	FUNCTION_write_blocking							= 0x3F00,
	FUNCTION_read									= 0x4000,
	FUNCTION_read_blocking							= 0x4100,		//<--
	FUNCTION_data_received_callback					= 0x4200
}Functions;

typedef enum {
	SUCCESS								= 0x00,
	ERROR_GENERIC						= 0x01,
	ERROR_ARGUMENT_OUT_OF_RANGE			= 0x02,
	ERROR_GOT_NULL_POINTER				= 0x03,
	ERROR_MALLOC_RETURNED_NULL			= 0x04,
	ERROR_NOT_READY_FOR_OPERATION		= 0x05,
	ERROR_INVALID_ARGUMENT				= 0x06,
	ERROR_WRONG_DEVICE_ID				= 0x07,
	ERROR_LENGTH_MISSMATCH				= 0x08,
	ERROR_QUEUE_WAS_EMPTY				= 0x09,
	ERROR_WRITE_FAILED					= 0x0A,
	ERROR_ADDRESS_TOO_LOW				= 0x0B,
	ERROR_ADDRESS_TOO_HIGH				= 0x0C,
	ERROR_FAILED_TO_LOCK_FLASH			= 0x0D,
	ERROR_FAILED_TO_UNLOCK_FLASH		= 0x0E,
	ERROR_ADDRESS_NOT_4_BYTE_BOUDARY	= 0x0F,
	ERROR_TRANSMISSION_ERROR			= 0x10,
	ERROR_READ_FAIL						= 0x11,
	ERROR_WRITE_FAIL					= 0x12,
	ERROR_REGMAP_INVALID_ADDRESS		= 0x13,
	ERROR_WRONG_START_BYTE				= 0x14,
	ERROR_BUS_OVER_RUN					= 0x15,
	ERROR_MAX_LENGTH					= 0x16,
	ERROR_MIN_LENGTH					= 0x17,
	ERROR_RECEIVE_CHARACTER_TIMEOUT		= 0x18,
	ERROR_STATUS_BYTE_UNKNOWN			= 0x19,
	ERROR_REGMAP_WRITE_DISABLED			= 0x1A,
	ERROR_BNO_READ_TIMEOUT				= 0x1B,		//<--
	ERROR_BNO_WRITE_TIMEOUT				= 0x1C
}Errors;

typedef uint64_t ErrorCode;

ErrorCode ErrorHandling_tempError;


#define DEFAULT_ERROR_HANDLER(error, module, function)		ErrorHandling_tempError = error;if(ErrorHandling_tempError != SUCCESS){return ErrorHandling_set_top_level(ErrorHandling_tempError, module, function);}


/*
	This function adds a error to the error queue to get processed when there is time for it.
	Parameters:
		- Error:	This is the error code witch gets added to the queue.
*/
void ErrorHandling_throw(ErrorCode Error);

/*
	This function adds a error to the error queue to get processed when there is time for it.
	Parameters:
		- Module:	Module where the error occurred.
		- Function:	Function where the error occurred.
		- Error:	Type of error that occurred.
*/
void ErrorHandling_throw_b(Modules Module, Functions Function, Errors Error);

/*
	This function gets the top error from the error queue and returns it as a pointer.
	Parameters:
		- Error:	Pointer to the error variable.
	Returns:
		true if there was an error, false if the queue is empty.
*/
bool ErrorHandling_catch(ErrorCode* Error);

/*
	This function Sets the topmost level of the error stack to the given module and function.
	This should be used when calling functions which return errors inside a function which is not at the topmost error stack level.
	Parameters:
		- Error:		Error from a child function.
		- LastModule:	Identifier of this Module.
		- LastFunction:	Identifier of this function.
	Returns:
		the modified error code.
*/
ErrorCode ErrorHandling_set_top_level(ErrorCode Error, Modules LastModule, Functions LastFunction);

/*
	This function translates an module identifier from an error code to a string description.
	strcat is used to append the description to the given char array.
	ATTENTION: the Array must be at least 33 values long ('\0' included)
	Parameters:
		- Error:			Error code where the module identifier should get translated
		- errorDescription:	Pointer to a predefined char array.
*/
void ErrorHandling_get_module_description(ErrorCode Error, char errorDescription[]);

/*
	This function translates the top stack module identifier from an error code to a string description.
	strcat is used to append the description to the given char array.
	ATTENTION: the Array must be at least 33 values long ('\0' included)
	Parameters:
		- Error:			Error code where the top stack module identifier should get translated
		- errorDescription:	Pointer to a predefined char array.
*/
void ErrorHandling_get_top_module_description(ErrorCode Error, char errorDescription[]);

/*
	This function translates the function identifier from an error code to a string description.
	strcat is used to append the description to the given char array.
	ATTENTION: the Array must be at least 33 values long ('\0' included)
	Parameters:
		- Error:			Error code where the function identifier should get translated
		- errorDescription:	Pointer to a predefined char array.
*/
void ErrorHandling_get_function_description(ErrorCode Error, char errorDescription[]);

/*
	This function translates the top stack function identifier from an error code to a string description.
	strcat is used to append the description to the given char array.
	ATTENTION: the Array must be at least 33 values long ('\0' included)
	Parameters:
		- Error:			Error code where the top stack function identifier should get translated
		- errorDescription:	Pointer to a predefined char array.
*/
void ErrorHandling_get_top_function_description(ErrorCode Error, char errorDescription[]);

/*
	This function translates an error code from its identifier to a string description.
	strcat is used to append the description to the given char array.
	ATTENTION: the Array must be at least 33 values long ('\0' included)
	Parameters:
		- Error:			Error that should get translated
		- errorDescription:	Pointer to a predefined cahr array.
*/
void ErrorHandling_get_error_description(ErrorCode Error, char errorDescription[]);

#endif /* ERRORHANDLING_H_ */