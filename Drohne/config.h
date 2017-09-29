/*
 * config.h
 *
 * Created: 29.09.2017 19:37:39
 *  Author: Markus Lorenz
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

/*
 *
 *	ESC Configuration
 *
 */

// M1 ESC Timer OCR Offset
#define ESC_Offset_M1 50
// M2 ESC Timer OCR Offset
#define ESC_Offset_M2 295
// M3 ESC Timer OCR Offset
#define ESC_Offset_M3 298
// M4 ESC Timer OCR Offset
#define ESC_Offset_M4 294

// M1 ESC Slope Compensation Factor
#define ESC_SlopeComp_M1 0.38
// M2 ESC Slope Compensation Factor
#define ESC_SlopeComp_M2 0
// M3 ESC Slope Compensation Factor
#define ESC_SlopeComp_M3 0
// M4 ESC Slope Compensation Factor
#define ESC_SlopeComp_M4 0

// Maximum ESC Speed
#define ESC_MaxLimit 960 // half of maximum motor power

/*
 *	
 *	RCReader Configuration
 *
 */

// Maximum Control Value Input (2000 = 1ms, 4000 = 2ms)
#define RC_ControlMax 3861
// Minimum Control Value Input (2000 = 1ms, 4000 = 2ms)
#define RC_ControlMin 2180
// Difference of the above defined values
#define RC_ControlDiff (RC_ControlMax - RC_ControlMin)


#endif /* CONFIG_H_ */