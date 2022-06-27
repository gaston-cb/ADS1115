// set PGA, chanel ,
//set chanell
#include <stdint.h>
//#define ADS1115
#define ADS1015

/// Los valores de las enumeraciones se corresponden con los que brinda la hoja de datos
/// En caso de modificarse, el sensor al usar este driver tendra un comportamiento erratico

typedef enum {
	FSR_6144 = 0 ,
	FSR_4096 = 1 ,
	FSR_2048 = 2 ,
	FSR_1024 = 3 ,
	FSR_512  = 4 ,
	FSR_256  = 5 ,
	FSR1_256 = 6 ,
	FSR2_256 = 7 ,
}ADS1x15_PGA_values_t ;


typedef enum {
	CONTINIOUS_MODE  = 0  ,
	SINGLE_SHOT_MODE = 1  ,
}ADS1115x_mode_measurment_t ;


typedef enum {
	CHANNEL_0_GND = 4, ///! SINGLE MODE
	CHANNEL_1_GND = 5, ///! SINGLE MODE
	CHANNEL_2_GND = 6, ///! SINGLE MODE
	CHANNEL_3_GND = 7, ///! SINGLE MODE
	CHANNEL_0_1   = 0, ///! ONLY USE IF USE DIFFERENTIAL MODES
	CHANNEL_0_3   = 1, ///! ONLY USE IF USE DIFFERENTIAL MODES
	CHANNEL_1_3   = 2, ///! ONLY USE IF USE DIFFERENTIAL MODES
	CHANNEL_2_3   = 3, ///! ONLY USE IF USE DIFFERENTIAL MODES
}ADS1x15_channel_t;


typedef enum{
	ACTIVE_LOW  = 0 ,
	ACTIVE_HIGH = 1 ,

}ADS1115_polarity_alert_t;


typedef enum {
	SPS_8   = 0,
	SPS_16  = 1 ,
	SPS_32  = 2,
	SPS_64  = 3,
	SPS_128 = 4,
	SPS_250 = 5,
	SPS_475 = 6,
	SPS_860 = 7,
}ADS1115_sps_t ;
/*
 * _ADC11 15_101
typedef enum {
	SPS_128   = 0,
	SPS_250  = 1 ,
	SPS_490 = 2,
	SPS_920  = 3,
	SPS_1600 = 4,
	SPS_2400 = 5,
	SPS_3300 = 6,
	SPS_3300 = 7,
}ADS1115_sps_t ;
*/







typedef enum {
	ON_WINDOW		,
	ON_COMPARATOR	,
	OFF				,
	CONVERSION_READY,
}ADS1115_alert_t ;

typedef struct {
	ADS1115_alert_t enableAlert 			; // hI TRHESH AND LO TRHESH USE IN On_window
	ADS1115_polarity_alert_t polarity_alert ;
	uint16_t HI_Thresh 					    ;
	uint16_t LO_Thresh 						;
}ADS1115_alert_comparator_t ;



typedef struct {
	ADS1x15_channel_t channel_select ;
	ADS1x15_PGA_values_t setPGA;
	ADS1115x_mode_measurment_t mode_measurement ;
	ADS1115_sps_t setSPS  ;
	ADS1115_alert_comparator_t alert_mode ;
}ADS1x15_config_t ;

uint8_t ADS1115init(uint8_t i2c_address, ADS1x15_config_t *ads1115Config) ;
void setMode(ADS1115x_mode_measurment_t mode);
void setPGA(ADS1x15_PGA_values_t ADS115xPGA) ; // cambiar PGA en contexto de ejecución
void selectChannel(ADS1x15_channel_t channel ) ;
float getVoltage() ;
