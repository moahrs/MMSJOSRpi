/******************************************************************
 *****                                                        *****
 *****  Name: Touch.h                                         *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 04/01/2013                                      *****
 *****  Auth: Frank Vannieuwkerke                             *****
 *****        Erik Olieman                                    *****
 *****  Func: Touch driver for use with ADS7843               *****
 *****                                                        *****
 ******************************************************************/

#ifndef MBED_Touch_H
#define MBED_Touch_H

#define TFT_IRQ_TCH RPI_V2_GPIO_P1_32

typedef struct
{
  int x;
  int y;
} Coordinate;

typedef struct
{
int         An,
           Bn,
           Cn,
           Dn,
           En,
           Fn,
           Divider ;
} Matrix;

/*
* Create a Touchscreen object connected to SPI and two pins.
*
* @param mosi,miso,sclk SPI
* @param cs pin connected to CS of ADS7843
* @param irq pin connected to IRQ of ADS7843
* @param pointer to SPI_TFT constructor
*
*/
//TouchScreenADS7843(PinName tp_mosi, PinName tp_miso, PinName tp_sclk, PinName tp_cs, PinName tp_irq, SPI_TFT *_LCD);

void Touch_Init(void);

/*
* Draw a 2 by 2 dot on the LCD screen.
*
* @param x (horizontal position)
* @param y (vertical position)
* @param color (16 bit pixel color)
*
*/
void TP_DrawPoint(unsigned int Xpos,unsigned int Ypos,unsigned int color);

/*
* Obtain averaged data from ADS7846.
* does 9 consecutive reads and only stores averaged data
* when the 9 points are within the treshold limits.
*
* @param screenPTR (pointer to store data)
* @returns 1 on success
* @returns 0 on failure
*
* If called with screenPTR = NULL - 'screen' variable is used, otherwise (parameter) is used.
*
*/
unsigned char Read_Ads7843(Coordinate * screenPtr);

/*
* Calibrate the touch panel.
* Three crosshairs are drawn and need to be touched in sequence.  
* A calibration matrix is set accordingly.
*
*/
void TouchPanel_Calibrate(void);

/*
* Obtain real x,y coordinates.
* The x,y coordinates are calculated using the calibration matrix.
*
*/
unsigned int getDisplayPoint(void);

/*
* Read touchpanel screensample and matrix values.
* 
* In your code, create 2 structures using Matrix and screenPtr
* and call this function with these structures.
* the calibration values are returned into these structures.
* Example:
* Matrix matrix;
* Coordinate ScreenSample[3];
* GetCalibration(&matrix, &ScreenSample[0]);
*/
void GetCalibration(Matrix * matrixPtr, Coordinate * screenPtr);

/*
* Set touchpanel calibration using screensample and matrix values.
* 
* In your code, create 2 structures based on Matrix and screenPtr,
* copy saved calibration values into these structures
* and call this function with these structures.
* Example:
* Matrix matrix;
* Coordinate ScreenSample[3];
* <pseudocode> load matrix with values from external storage (eg eeprom).
* <pseudocode> load ScreenSample with values from external storage (eg eeprom).
* SetCalibration(&matrix, &ScreenSample[0]);
*/
void SetCalibration(Matrix * matrixPtr, Coordinate * screenPtr);

#define    SPI_RD_DELAY    1
#define    CHX             0xD0    // 12 bit mode
#define    CHY             0x90


/*
* Obtain raw x,y data from ADS7846
*
* @param pointer to raw x and y coordinates (pointer to store data)
* @returns x (horizontal position)
* @returns y (vertical position)
*
*/
void TP_GetAdXY(int *x,int *y);

/*
* Obtain raw single channel data from ADS7846 (Called by TP_GetADXY)
*
* @param channel to be read (CHX or CHY)
* @returns raw scaled down value (return value range must be between 0 and 1024)
*
*/
int Read_XY(unsigned char XY);

/*
* Draw a calibration crosshair on the LCD screen
*
* @param x (horizontal position)
* @param y (vertical position)
*
*/
void DrawCross(unsigned int Xpos,unsigned int Ypos);

/*
* Set the calibration matrix
*
* @param displayPTR (pointer to display data)
* @param screenPTR  (pointer to screen data)
* @param matrixPTR  (pointer to calibration matrix)
*
* @returns 0 when matrix.divider != 0
* @returns 1 when matrix.divider = 0
*
*/
unsigned int setCalibrationMatrix( Coordinate * displayPtr,Coordinate * screenPtr,Matrix * matrixPtr);

#endif