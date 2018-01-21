/*
 * Implementation of eCompass (Tilt Compensated Digital Compass) Display
 * based an AVR ATmega8 MCU, using
 * Magnetometer(HMC5883L module),
 * Accelerometer(MPU6050 module) and
 * 128x64 SSD1306 OLED Display module(I2C)
 *
 * AVR uses Internal 8MHz clock
 *
 * Date Started: Dec 19, 2017
 * By: Visakhan C
 */
 
 
#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ssd1306.h"	/* I2C Display */
#include "hmc5883.h"	/* I2C Magnetometer (Digital compass) */
#include "mpu6050.h"	/* I2C Accelerometer+Gyroscope */
#include "16pt_font.h"
#include "8pt_font.h"

/* Definitions */
#define RED_LED				PB0
#define RED_LED_PORT		PORTB
#define RED_LED_DDR			DDRB
#define RED_LED_INIT()		(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()		(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()		(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()	(RED_LED_PORT ^= (1 << RED_LED))

/* Function declarations */
static void avr_init(void);


/* Variables */
static struct oled_disp_struct oled_azimuth = { .row_start = 1, .col_start = 100 };
static struct oled_disp_struct oled_pitch = { .row_start = 4, .col_start = 100 };
static struct oled_disp_struct oled_roll =  { .row_start = 6, .col_start = 100 };
static uint8_t mag_buffer[6];
static uint8_t acc_buffer[14];


/*  Main Program */
int main(void)
{
	int16_t mag_x, mag_y, mag_z;
	int16_t acc_x, acc_y, acc_z;
	double roll;
	double pitch;
	double azimuth;
	double X_h, Y_h;
	int8_t x, y;
	int8_t x_old = 0, y_old = 0;

	avr_init();
	oled_clear_display();

	/* Draw the markings and other non-changing items */
	oled_set_font(font_8pt);
	oled_set_position(0, 29);
	oled_putchar('N');
	oled_set_position(4, 0);
	oled_putchar('W');
	oled_set_position(4, 60);
	oled_putchar('E');
	oled_set_position(7, 29);
	oled_putchar('S');
	oled_line(1, 1, 7, 7);
	oled_line(62, 1, 56, 7);
	oled_line(1, 62, 7, 56);
	oled_line(56, 56, 62, 62);
	oled_set_font(font_16pt);
	oled_set_position(1, 76);
	oled_putstring("H =");
	oled_set_position(4, 76);
	oled_putstring("P =");
	oled_set_position(6, 76);
	oled_putstring("R =");

	while(1) {
		_delay_ms(50);
		/* Get samples */
		if(hmc5883_get_data(mag_buffer) != 0) {
			RED_LED_ON();
			while(1);
		}
		if(mpu6050_get_data(acc_buffer, sizeof(acc_buffer)) != 0) {
			RED_LED_ON();
			while(1);
		}
		mag_x = (mag_buffer[0] << 8) + mag_buffer[1];
		mag_z = (mag_buffer[2] << 8) + mag_buffer[3];
		mag_y = (mag_buffer[4] << 8) + mag_buffer[5];
		acc_x = (acc_buffer[0] << 8) + (acc_buffer[1]);
		acc_y = (acc_buffer[2] << 8) + (acc_buffer[3]);
		acc_z = (acc_buffer[4] << 8) + (acc_buffer[5]);

		/* Calculate pitch and roll, in the range (-pi,pi) */
		pitch = atan2((double)-acc_x, sqrt((long)acc_z*(long)acc_z + (long)acc_y*(long)acc_y));
		roll = atan2((double)acc_y, sqrt((long)acc_z*(long)acc_z  + (long)acc_x*(long)acc_x));

		/* Calculate Azimuth:
		 * Magnetic horizontal components, after compensating for Roll(r) and Pitch(p) are:
		 * X_h = X*cos(p) + Y*sin(r)*sin(p) + Z*cos(r)*sin(p)
		 * Y_h = Y*cos(r) - Z*sin(r)
		 * Azimuth = arcTan(Y_h/X_h)
		 */
		X_h = (double)mag_x*cos(pitch) + (double)mag_y*sin(roll)*sin(pitch) + (double)mag_z*cos(roll)*sin(pitch);
		Y_h = (double)mag_y*cos(roll) - (double)mag_z*sin(roll);
		azimuth = atan2(Y_h, X_h);
		if(azimuth < 0) {	/* Convert Azimuth in the range (0, 2pi) */
			azimuth = 2*M_PI + azimuth;
		}
		x = 32 + 24 * sin(azimuth);
		y = 32 - 24 * cos(azimuth);

		/* Update display */
		oled_set_font(font_16pt);
		oled_update_number(&oled_azimuth, (int16_t)(azimuth * 180.0 / 3.14), 0);
		oled_putchar('~');
		oled_clear_area(OLED_PAGE_RANGE(1, 1), oled_get_column(), OLED_LAST_COLUMN);
		oled_update_number(&oled_pitch, (int16_t)(pitch * 180.0 / 3.14), 0);
		oled_putchar('~');
		oled_clear_area(OLED_PAGE_RANGE(4, 4), oled_get_column(), OLED_LAST_COLUMN);
		oled_update_number(&oled_roll, (int16_t)(roll * 180.0 / 3.14), 0);
		oled_putchar('~');
		oled_clear_area(OLED_PAGE_RANGE(6, 6), oled_get_column(), OLED_LAST_COLUMN);
		if((x != x_old) || (y != y_old)) {
			oled_clear_area(OLED_PAGE_RANGE(1, 6), 8, 57);
			oled_line(32, 32, x, y);
		}
		x_old = x;
		y_old = y;
		//RED_LED_TOGGLE();
	}
}


void avr_init()
{
	RED_LED_INIT();
	RED_LED_ON();
	_delay_ms(100);
	RED_LED_OFF();
	sei();
	if(oled_init() != 0) {
		RED_LED_ON();
		while(1);
	}
	if(hmc5883_init(HMC5883_CONFIG_AVG_8|HMC5883_CONFIG_RATE_15_HZ|HMC5883_CONFIG_MODE_NORMAL, HMC5883_MODE_CONTINUOUS, HMC5883_GAIN_1_9) != 0) {
		RED_LED_ON();
		while(1);
	}
	if(mpu6050_init(MPU6050_LOW_POWER_ACCEL_MODE) != 0) {
		RED_LED_ON();
		while(1);
	}
}



