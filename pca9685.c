#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <unistd.h>
#include "pca9685.h"

#define MODE1 0x00
#define PCA9685_PRESCALE 0xFE

//Channel 0
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09

// Channel 1
#define LED1_ON_L 0x0A
#define LED1_ON_H 0x0B
#define LED1_OFF_L 0x0C
#define LED1_OFF_H 0x0D


int pca9685_init(void) {

    char filename[20]; 
	
    snprintf(filename, sizeof(filename), "/dev/i2c-%d", 1);

    int file = open(filename, O_RDWR);

    // For Error Handling // 

    if (file < 0) {
        perror("File Access Failed");
        return -1;
    }

    int addr = 0x40;

    // Error handling once again //

    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        
	perror("I2C Address Access Failed");
	close(file);
	return -1;
    }


    __s32 mode1_value = i2c_smbus_read_byte_data(file, MODE1);


    const uint8_t SLEEP_MASK = 0x10; 

    if (mode1_value < 0) {
	perror("I2C Transaction Failed");
	close(file);
	return -1;
    }
        uint8_t sleepBit = mode1_value & SLEEP_MASK; //inspecting sleep bit
    
        if (sleepBit == 0) {                          
            mode1_value = mode1_value | SLEEP_MASK;  //setting sleep bit      
        }
     
        if (i2c_smbus_write_byte_data(file, MODE1, mode1_value) < 0) {
            perror("Write to Mode 1 failed");
	    close(file);
 	    return -1;
        }

        if (i2c_smbus_write_byte_data(file, PCA9685_PRESCALE, 121) < 0) {
            perror("Write to Prescale failed");
	    close(file);
	    return -1;
        }
   
        mode1_value = mode1_value & ~SLEEP_MASK;

        if (i2c_smbus_write_byte_data(file, MODE1, mode1_value) < 0) {

        perror("Wake up failed");
	close(file);
        return -1;
        }
 
    usleep(500);
     
return file;
}



// CHANNEL 0
int pca9685_set_channel0_pwm(int file, uint16_t onCount, uint16_t offCount) {

  if (onCount > 4095 || offCount > 4095) {
    return -1;
    }    
    uint8_t onLow = onCount & 0xFF;
    uint8_t onHigh = (onCount >> 8) & 0x0F;

    uint8_t offLow = offCount & 0xFF;
    uint8_t offHigh = (offCount >> 8) & 0x0F;
    
    if (i2c_smbus_write_byte_data(file, LED0_ON_L, onLow) < 0) {
        perror("Failed to write LED0_ON_L");		    
	return -1;	    
    }
    if (i2c_smbus_write_byte_data(file, LED0_ON_H, onHigh) < 0) {
        perror("Failed to write LED0_ON_H");		    
	return -1;	    
    }
    if (i2c_smbus_write_byte_data(file, LED0_OFF_L, offLow) < 0) {
        perror("Failed to write LED0_OFF_L");		    
	return -1;	    
    }
    if (i2c_smbus_write_byte_data(file, LED0_OFF_H, offHigh) < 0) {
        perror("Failed to write LED0_OFF_H");		    
	return -1;	    
    }

return 0;
}

// CHANNEL 1
int pca9685_set_channel1_pwm(int file, uint16_t onCount, uint16_t offCount) {

    if (onCount > 4095 || offCount > 4095) {
    return -1;
    }
    uint8_t onLow = onCount & 0xFF;
    uint8_t onHigh = (onCount >> 8) & 0x0F;

    uint8_t offLow = offCount & 0xFF;
    uint8_t offHigh = (offCount >> 8) & 0x0F;
    

    if (i2c_smbus_write_byte_data(file, LED1_ON_L, onLow) < 0) {
        perror("Failed to write LED1_ON_L");		    
	return -1;	    
    }
    if (i2c_smbus_write_byte_data(file, LED1_ON_H, onHigh) < 0) {
        perror("Failed to write LED1_ON_H");		    
	return -1;	    
    }
    if (i2c_smbus_write_byte_data(file, LED1_OFF_L, offLow) < 0) {
        perror("Failed to write LED1_OFF_L");		    
	return -1;	    
    }
    if (i2c_smbus_write_byte_data(file, LED1_OFF_H, offHigh) < 0) {
        perror("Failed to write LED1_OFF_H");		    
	return -1;	    
    }

return 0;
}
