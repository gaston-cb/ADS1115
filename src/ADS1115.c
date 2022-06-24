#include <ADS1115.h>
#include <stdio.h>
#include "esp_log.h"
#include <string.h>

#include "portsESP32.h"
/// ful scale range in volts
#define FSR_0 6.144f
#define FSR_1 4.096f
#define FSR_2 2.048f
#define FSR_3 1.024f
#define FSR_4 0.512f
#define FSR_5 0.256f
#define NUMBERS_BITS_ADS1015 2048.0 // (11 bits)

// registros punteros del ads1115
#define ADDRESS_POINTER_REG_CONVERSION_REGISTER 0x00
#define ADDRESS_POINTER_REG_CONFIG_REGISTER     0x01
#define ADDRESS_POINTER_REG_HI_THRESH_REGISTER  0x02
#define ADDRESS_POINTER_REG_LO_THRESH_REGISTER  0x03
/// ERRORES DEFINIDOS PARA INICIALIZAR ADS1115
#define ERROR_INIT_ADS1115_OK 0x01 // configuración inicializada correctamente
#define ERROR_INIT_ADS1115_WRITE_OK 0x02
#define ERROR_INIT_ADS1115_READ_OK 0x03
#define ERROR_INIT_ADS1115_WRITE_ERROR 0x04 // error en la comunicación I2C
#define ERROR_INIT_ADS1115_READ_ERROR  0x05 // el dispositivo se lee pero no se escribe correctamente
#define ERROR_INIT_ADS1115_NOT_CONFIG 0x06
#define ERROR_INIT_ADS1115_FAIL  0xFF // no se puede LEER NI  configurar el dispositivo
/// fin definicion de errores de inicio de I2C


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
	ADS1115_config_t user_config ;
	config_register_t config_register ;
}ads111x_handle_t ;

static ads111x_handle_t ADS1115 ;

// void ADS111x

/**
 *
 * @param i2c_address: 8 bytes para el dispositivo, añadiendole un 0 delante de la direcciòn
 * @param ads1115Config estructura de configuración definida por el usuario
 * @return ERROR: 0X01 -> OK
 * 				  OXO2 ->WRITE_OK
 * 				  0X03 ->READ_OK
 * 				  OXO4 WRITE_ERROR
 *
 */
uint8_t ADS1115init(uint8_t i2c_address, ADS1115_config_t *ads1115Config){
	uint8_t error_code_init;
	uint8_t address_pointer = ADDRESS_POINTER_REG_CONFIG_REGISTER ;
	uint8_t config_user[3]  = {address_pointer ,0,0} ;
	uint8_t config_init[2] ;
	uint8_t read_config_check[2];
	// LECTURA DE CONFIGURACION INICIAL !
	ADS1115.I2C_address = i2c_address ; ///TODO: CHECK ERRORS FOR NEW VERSIONS
	ADS1115.user_config = *ads1115Config ;
	error_code_init = I2CWriteToSlave(ADS1115.I2C_address, &address_pointer, 1 ) ;
	errorI2CWrite(&error_code_init) ;
	if (error_code_init != ERROR_INIT_ADS1115_WRITE_OK) {
		return error_code_init ;
	}
	//printf
	error_code_init = I2CReadToSlave(ADS1115.I2C_address, config_init ,2 ) ;
	errorI2CRead(&error_code_init) ;
	if (error_code_init != ERROR_INIT_ADS1115_READ_OK) {
		return error_code_init ;
	}

	// CONFIGURACIÒN DEL USUARIO
	selectChannel(ADS1115.user_config.channel_select ) ;
	setPGA(ADS1115.user_config.setPGA) ;
	setMode(ADS1115.user_config.mode_measurement);
	setSPS(ADS1115.user_config.setSPS) ;
	alertDRYport(ADS1115.user_config.alert_mode) ;
	memcpy(&config_user[1],(uint8_t *)&ADS1115.config_register ,2) ;//destino, fuente, tamaño
	// Escritura de la configuraciòn del usuario

	error_code_init = I2CWriteToSlave(ADS1115.I2C_address,config_user,3) ; //escribir registro de configuración
	errorI2CWrite(&error_code_init) ;

	if (error_code_init != ERROR_INIT_ADS1115_WRITE_OK) {
			return error_code_init ;
	}
	error_code_init =I2CWriteToSlave(ADS1115.I2C_address, &address_pointer, 1 ) ; //register pointer address en 0x01
	errorI2CRead(&error_code_init) ;
	if (error_code_init != ERROR_INIT_ADS1115_READ_OK) {
			return error_code_init ;
	}
	// check de configuración de los parametros

	error_code_init = I2CReadToSlave(ADS1115.I2C_address, read_config_check ,2 ) 	   ;
	if (error_code_init != ERROR_INIT_ADS1115_READ_OK) {
		return error_code_init ;
	}

	//// CHECKING DE LA CONFIGURACION DE USUARIO
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
void setPGA(ADS111x_PGA_values_t ADS115xPGA ){
	ADS1115.config_register.PGA =  ADS115xPGA ;
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

}

// single end chanel_1 = GND ; channel0 AINP, CHANNEL 1 AINN
void selectChannel(ADS1115_channel_t channel)
{
	ADS1115.config_register.MUX = channel ;

}


// void readConfig()


void setMode(ADS1115x_mode_measurment_t mode)
{
	ADS1115.config_register.MODE = mode ;
}


static void setSPS(ADS1115_sps_t sps){
	ADS1115.config_register.DR = sps ;
}



static void alertDRYport(ADS1115_alert_comparator_t alert_user){
	if (alert_user.enableAlert == ON_WINDOW){
// 		printf ("alert_onWindow") ;
	}else if (alert_user.enableAlert == ON_COMPARATOR){
//  		printf ("alert_onComparator") ;
	}else if (alert_user.enableAlert == CONVERSION_READY){
//		printf("conversion_ready") ;
	}else if (alert_user.enableAlert == OFF){
		ADS1115.config_register.COMP_MODE = 0 ;
		ADS1115.config_register.COMP_POL  = 0 ;
		ADS1115.config_register.COMP_LAT  = 0 ;
		ADS1115.config_register.COMP_QUE  = 3 ;
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
		if (ADS1115.user_config.mode_measurement == CONTINIOUS_MODE){
			I2CWriteToSlave(ADS1115.I2C_address,&address_pointer,1)        ;
		}else if (ADS1115.user_config.mode_measurement == SINGLE_SHOT_MODE){
			address_pointer = ADDRESS_POINTER_REG_CONFIG_REGISTER ;
			ADS1115.config_register.OS = 1 ;
			raw_data[0] = address_pointer  ;
			memcpy(&raw_data[1],(uint8_t *)&ADS1115.config_register ,2) ;//destino, fuente, tamaño
			I2CWriteToSlave(ADS1115.I2C_address ,raw_data , 3) ;
			ADS1115.config_register.OS = 0 ;
			address_pointer = ADDRESS_POINTER_REG_CONVERSION_REGISTER ;
			I2CWriteToSlave(ADS1115.I2C_address,&address_pointer,1) ;
		}
		I2CReadToSlave(ADS1115.I2C_address, read_data,2) ;
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



