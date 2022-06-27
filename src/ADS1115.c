#include <ADS1115.h>
#include <stdio.h>
#include "esp_log.h"
#include <string.h>

#include "portsESP32.h"
/// ful scale range in volts for ADS1115

#if defined(ADS1015) && defined(ADS1115)
	#error "No se pueden definir ambos dispositivos, comente el dispositivo a usar en ADS1115."
#endif




#define FSR_0 6.144f
#define FSR_1 4.096f
#define FSR_2 2.048f
#define FSR_3 1.024f
#define FSR_4 0.512f
#define FSR_5 0.256f

#ifdef ADS1015
#	define NUMBERS_BITS_ADS1015 2048.0 // (11 bits)
#elif ADS1115
	#define NUMBERS_BITS_ADS1115 65535.0 // (16 bits)
#else
#error "debe definir ADS1115 O ADS1015 "
#endif
/// registros punteros del ads1115
#define ADDRESS_POINTER_REG_CONVERSION_REGISTER 0x00
#define ADDRESS_POINTER_REG_CONFIG_REGISTER     0x01
#define ADDRESS_POINTER_REG_HI_THRESH_REGISTER  0x02
#define ADDRESS_POINTER_REG_LO_THRESH_REGISTER  0x03
///! ERRORES DEFINIDOS PARA LECTURAS/ESCRITURAS ADS1115
#define ERROR_INIT_ADS1115_OK 0x01 // configuración inicializada correctamente
#define ERROR_INIT_ADS1115_WRITE_OK 0x02
#define ERROR_INIT_ADS1115_READ_OK 0x03
#define ERROR_INIT_ADS1115_WRITE_ERROR 0x04 // error en la comunicación I2C
#define ERROR_INIT_ADS1115_READ_ERROR  0x05 // el dispositivo se lee pero no se escribe correctamente
#define ERROR_INIT_ADS1115_NOT_CONFIG 0x06
#define ERROR_INIT_ADS1115_FAIL  0xFF 		// no se puede LEER NI  configurar el dispositivo
/// fin definicion de errores relacionados al I2C


static void errorI2CRead(uint8_t *error_code) ;
static void errorI2CWrite(uint8_t *error_code) ;
static void setSPS(ADS1115_sps_t sps) ;
static void alertDRYport(ADS1115_alert_comparator_t alert_user) ;
static float factor_conv_ad ;
typedef struct {
	uint8_t MODE : 1 ;
	uint8_t PGA  : 3 ;
	uint8_t MUX  : 3 ;
	uint8_t OS   : 1 ;

	uint8_t COMP_QUE : 2 ;
	uint8_t COMP_LAT : 1 ;
	uint8_t COMP_POL : 1 ;
	uint8_t COMP_MODE : 2 ;
	uint8_t DR : 2 ;
}config_register_t ;


typedef struct {
	uint8_t I2C_address ;
	ADS1x15_config_t user_config ;
	config_register_t config_register ;
}ads1x15_handle_t ;
/// ifdef -> ads1115 or ads1015
static ads1x15_handle_t ADS1x15 ;


/**
 *
 * @param i2c_address: 8 bytes para el dispositivo, añadiendole un 0 delante de la direcciòn
 * @param ads1115Config estructura de configuración definida por el usuario
 * @return
 *   ERROR_INIT_ADS1115_OK
 *   ERROR_INIT_ADS1115_WRITE_OK
 *   ERROR_INIT_ADS1115_READ_OK
 *	 ERROR_INIT_ADS1115_WRITE_ERROR
 *	 ERROR_INIT_ADS1115_READ_ERROR
 * 	 ERROR_INIT_ADS1115_NOT_CONFIG
 *   ERROR_INIT_ADS1115_FAIL
 *
 */
uint8_t ADS1115init(uint8_t i2c_address, ADS1x15_config_t *ads1115Config){
	uint8_t error_code_init;
	uint8_t address_pointer = ADDRESS_POINTER_REG_CONFIG_REGISTER ;
	uint8_t config_user[3]  = {address_pointer ,0,0} ;
	uint8_t config_init[2] ;
	uint8_t read_config_check[2];
	//! LECTURA DE CONFIGURACION INICIAL !
	ADS1x15.I2C_address = i2c_address ; ///TODO: CHECK ERRORS FOR NEW VERSIONS
	ADS1x15.user_config = *ads1115Config ;
	error_code_init = I2CWriteToSlave(ADS1x15.I2C_address, &address_pointer, 1 ) ;
	errorI2CWrite(&error_code_init) ;
	if (error_code_init != ERROR_INIT_ADS1115_WRITE_OK) {
		return error_code_init ;
	}
	//printf
	error_code_init = I2CReadToSlave(ADS1x15.I2C_address, config_init ,2 ) ;
	errorI2CRead(&error_code_init) ;
	if (error_code_init != ERROR_INIT_ADS1115_READ_OK) {
		return error_code_init ;
	}

	// CONFIGURACIÒN DEL USUARIO
	selectChannel(ADS1x15.user_config.channel_select ) ;
	setPGA(ADS1x15.user_config.setPGA) ;
	setMode(ADS1x15.user_config.mode_measurement);
	setSPS(ADS1x15.user_config.setSPS) ;
	alertDRYport(ADS1x15.user_config.alert_mode) ;
	memcpy(&config_user[1],(uint8_t *)&ADS1x15.config_register ,2) ;//destino, fuente, tamaño
	// Escritura de la configuraciòn del usuario

	error_code_init = I2CWriteToSlave(ADS1x15.I2C_address,config_user,3) ; //escribir registro de configuración
	errorI2CWrite(&error_code_init) ;

	if (error_code_init != ERROR_INIT_ADS1115_WRITE_OK) {
			return error_code_init ;
	}
	error_code_init =I2CWriteToSlave(ADS1x15.I2C_address, &address_pointer, 1 ) ; //register pointer address en 0x01
	errorI2CRead(&error_code_init) ;
	if (error_code_init != ERROR_INIT_ADS1115_READ_OK) {
			return error_code_init ;
	}
	// check de configuración de los parametros

	error_code_init = I2CReadToSlave(ADS1x15.I2C_address, read_config_check ,2 ) 	   ;
	if (error_code_init != ERROR_INIT_ADS1115_READ_OK) {
		return error_code_init ;
	}

	//// CHECKING DE LA CONFIGURACION DE USUARIO-- CAMBIAR LA INICIALIZACIÓN
	if (read_config_check[0] == config_user[1] ){
		if (read_config_check[1] ==config_user[2]) {
			error_code_init = ERROR_INIT_ADS1115_OK ;
		}
	}else{
		error_code_init = ERROR_INIT_ADS1115_NOT_CONFIG  ;
	}
	return error_code_init ;
}


/**
 *
 * @param ADS115xPGA
 */
void setPGA(ADS1x15_PGA_values_t ADS115xPGA ){
	ADS1x15.config_register.PGA =  ADS115xPGA ;
#ifdef ADS1015
	switch (ADS115xPGA){
		case (FSR_6144):
			factor_conv_ad = (FSR_0/NUMBERS_BITS_ADS1015) ;
			break ;
		case FSR_4096:
			factor_conv_ad = (FSR_1/NUMBERS_BITS_ADS1015) ;
			break ;
		case FSR_2048:
			factor_conv_ad = (FSR_2/NUMBERS_BITS_ADS1015) ;
			break ;
		case FSR_1024:
			factor_conv_ad = (FSR_3/NUMBERS_BITS_ADS1015) ;
			break ;
		case FSR_512:
			factor_conv_ad = (FSR_3/NUMBERS_BITS_ADS1015) ;
			break ;
		case FSR_256:
		case FSR1_256:
		case FSR2_256:
			factor_conv_ad = (FSR_2/NUMBERS_BITS_ADS1015)  ;
			break ;

	}
#elif ADS1115
	switch (ADS115xPGA){
		case (FSR_6144):
			factor_conv_ad = (FSR_0/NUMBERS_BITS_ADS1115) ;
			break ;
		case FSR_4096:
			factor_conv_ad = (FSR_1/NUMBERS_BITS_ADS1115) ;
			break ;
		case FSR_2048:
			factor_conv_ad = (FSR_2/NUMBERS_BITS_ADS1115) ;
			break ;
		case FSR_1024:
			factor_conv_ad = (FSR_3/NUMBERS_BITS_ADS1115) ;
			break ;
		case FSR_512:
			factor_conv_ad = (FSR_3/NUMBERS_BITS_ADS1115) ;
			break ;
		case FSR_256:
		case FSR1_256:
		case FSR2_256:
			factor_conv_ad = (FSR_2/NUMBERS_BITS_ADS1015)  ;
			break ;

	}


#endif
}

// single end chanel_1 = GND ; channel0 AINP, CHANNEL 1 AINN
void selectChannel(ADS1x15_channel_t channel)
{
	ADS1x15.config_register.MUX = channel ;

}


// void readConfig()


void setMode(ADS1115x_mode_measurment_t mode)
{
	ADS1x15.config_register.MODE = mode ;
}


static void setSPS(ADS1115_sps_t sps){
	ADS1x15.config_register.DR = sps ;
}



static void alertDRYport(ADS1115_alert_comparator_t alert_user){
	uint8_t buffer_config_hi_thresh[3] ;
	uint8_t buffer_config_lo_thresh[3] ;
	buffer_config_hi_thresh[0] = ADDRESS_POINTER_REG_HI_THRESH_REGISTER ;
	buffer_config_lo_thresh[0] = ADDRESS_POINTER_REG_LO_THRESH_REGISTER ;

	if (alert_user.enableAlert == ON_WINDOW){

//		ADS1x15.config_register.COMP_MODE = 1 ;

	}else if (alert_user.enableAlert == ON_COMPARATOR){
		printf("on comparator \r\n") ;
		ADS1x15.config_register.COMP_QUE  = 2 ;

		ADS1x15.config_register.COMP_MODE = 0 ;
		ADS1x15.config_register.COMP_POL = alert_user.polarity_alert ;
		printf("BEFORE_barrel_shifter: %04x %04x\r\n",alert_user.HI_Thresh , alert_user.LO_Thresh) ;

		alert_user.HI_Thresh = alert_user.HI_Thresh<<4 ;
		alert_user.LO_Thresh = alert_user.LO_Thresh<<4 ;
		printf("BEFORE_barrel_shifter: %04x %04x\r\n",alert_user.HI_Thresh , alert_user.LO_Thresh) ;
		buffer_config_hi_thresh[1] = (uint8_t) (alert_user.HI_Thresh>>8);
		buffer_config_hi_thresh[2] = (uint8_t) (alert_user.HI_Thresh);
		buffer_config_lo_thresh[1] = (uint8_t) (alert_user.LO_Thresh>>8);
		buffer_config_lo_thresh[2] = (uint8_t) (alert_user.LO_Thresh);

		I2CWriteToSlave(ADS1x15.I2C_address, buffer_config_hi_thresh, 3) ;
		I2CWriteToSlave(ADS1x15.I2C_address, buffer_config_lo_thresh, 3) ;

		printf("end_of_comparators!\r\n") ;
	}else if (alert_user.enableAlert == CONVERSION_READY){
//		printf("conversion_ready") ;
	}else if (alert_user.enableAlert == OFF){
		ADS1x15.config_register.COMP_MODE = 0 ;
		ADS1x15.config_register.COMP_POL  = 0 ;
		ADS1x15.config_register.COMP_LAT  = 0 ;
		ADS1x15.config_register.COMP_QUE  = 3 ;
	}


}


/// falta controlar los errores en caso que existan en los puertos
/// I2C.
/// La función lee el valor del adc y lo transforma en un valor de tension en
/// volts
float getVoltage(){
		uint8_t address_pointer  ;
		uint8_t raw_data[3] = {0x01,0,0} ;
		uint8_t read_data[2] ;
		int16_t voltage ; // voltage in mv
		float volt = 0 ;
		if (ADS1x15.user_config.mode_measurement == CONTINIOUS_MODE){
			I2CWriteToSlave(ADS1x15.I2C_address,&address_pointer,1)        ;
		}else if (ADS1x15.user_config.mode_measurement == SINGLE_SHOT_MODE){
			address_pointer = ADDRESS_POINTER_REG_CONFIG_REGISTER ;
			ADS1x15.config_register.OS = 1 ;
			raw_data[0] = address_pointer  ;
			memcpy(&raw_data[1],(uint8_t *)&ADS1x15.config_register ,2) ;//destino, fuente, tamaño
			I2CWriteToSlave(ADS1x15.I2C_address ,raw_data , 3) ;
			ADS1x15.config_register.OS = 0 ;
			address_pointer = ADDRESS_POINTER_REG_CONVERSION_REGISTER ;
			I2CWriteToSlave(ADS1x15.I2C_address,&address_pointer,1) ;
		}
		I2CReadToSlave(ADS1x15.I2C_address, read_data,2) ;
		uint8_t aux = read_data[0] ;
		read_data[0] = read_data[1] ;
		read_data[1] = aux ;
		// convert to voltage
		memcpy (&voltage,read_data,sizeof(int16_t) ) ;
		voltage = voltage >>4 ;
		volt = (voltage) * factor_conv_ad ;
		return volt ;





}

void errorI2CWrite(uint8_t *error_code){
	if (*error_code == ERROR_I2C_OK){
		*error_code = ERROR_INIT_ADS1115_WRITE_OK ;
	}else if (*error_code == ERROR_I2C_FAIL){
		*error_code = ERROR_INIT_ADS1115_WRITE_ERROR ;
	}else {
		*error_code = ERROR_INIT_ADS1115_FAIL ;
	}
}



void errorI2CRead(uint8_t *error_code){
	if (*error_code == ERROR_I2C_OK){
		*error_code = ERROR_INIT_ADS1115_READ_OK ;
	}else if (*error_code == ERROR_I2C_FAIL){
		*error_code = ERROR_INIT_ADS1115_WRITE_ERROR ;
	}else {
		*error_code = ERROR_INIT_ADS1115_FAIL  	    ;
	}
}



