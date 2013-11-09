/* =========================================================================== *
 * File:		Project5a.cc
 * Author:		Lennard Streat, Christoffer Rosen
 * Description:	Interacts with an SRF04 Ultrasound range sensor to measure
 *				the distance from the SRF04 to objects in it's path.
 *				Data Acquision Port Pinout:
 *					- Pin 20: Pulse Trigger Input (Yellow Wire)
 *					- Pin 21: Echo Output (Blue Wire)
 *					- Pin 27: +5Volts DC (Red Wire)
 *					- Pin 28: DGND (Black Wire)
 * =========================================================================== */
/* Library Includes */
#include <stdio.h>			/* Resource(s): printf() */
#include <stdlib.h>			/* Resource(s): EXIT_* */
#include <stdint.h>			/* Resource(s): uintptr_t */
#include <hw/inout.h>		/* Resource(s): in*(), out*() */
#include <sys/neutrino.h>	/* Resource(s): ThreadCtl() */
#include <sys/syspage.h>	/* Resource(s): cycles_per_second */
#include <sys/mman.h>		/* Resource(s): mmap_device_io() */
#include <termios.h>
#include <unistd.h>
#include <iostream>
/* Project Specific Include: */
#include "UltrasoundEchoReader.h"

/* Constant used for the mman library */
#define BYTE (1)

/* Port C Data Register */
#define DIGITAL_IO_BASE_ADDR (0x280)
#define PORTC_OFFSET (10)
#define PORTC_ADDR ( DIGITAL_IO_BASE_ADDR + PORTC_OFFSET )

/* Port Direction Register */
#define PORT_DIR_OFFSET (11)
#define PORT_DIR_ADDR ( DIGITAL_IO_BASE_ADDR + PORT_DIR_OFFSET )
/* Enable Port C for i/o:
 * 	C4 = input
 * 	C3 = output */
#define DIOCR_PORTC ( 0b10001000 )

/* Global variables, used to store the values measured on the sensor */
extern int minimum, maximum; /* Defined in UltrasoundEchoReader.h */

/**
 * Function:	getch
 * Description:	Used as an alternative method from getchar. This returns
 *				from the get char operation without requiring that a Carriage
 *				return be sent to the buffer.
 * Returns:		The character received from user input.
 */
int getch( void ){
	int ch;
	struct termios oldt;
	struct termios newt;
	tcgetattr(STDIN_FILENO, &oldt); /*store old settings*/
	std::cin.clear(); // clear EOF flag
	newt = oldt; /* copy old settings to new settings */
	newt.c_lflag &= ~(ICANON | ECHO); /* change settings - disable buffered i/o and set echo mode */

	tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new terminal i/o settings immediatly */
	ch = getchar(); /* standard getchar call */
	putchar(' ');

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /* reapply the old settings */
	return ch; /* return received char */
}/* getch() */

/**
 * Function:	main
 * Description:	The main entry point for the Ultrasound Range finder program.
 * Returns:		The exit success--if it reaches the end  this will always be 0.
 */
int main( void ){
	/* Error Handling & User Input: */
	int privity_err;
	int return_code = EXIT_SUCCESS;
	int c;
	static struct termios oldt, newt;

	/* Memory Mapped IO: */
	uintptr_t portc;
	uintptr_t portc_dir;
	uint8_t cdata;

	/* Timing: */
	uint64_t cps;
	uint64_t diff;
	float diff_ms;
	int buf;

	/* Enable GPIO access to the current thread: */
	privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
	if( privity_err == -1 ){
		printf( "Error: Unable to acquire root permission for GPIO.\n" );
		return_code = EXIT_FAILURE;
	}else{
		/* Create a pointer to Data Acquisition Port (Memory Mapped IO): */
		portc = mmap_device_io( BYTE, PORTC_ADDR );
		portc_dir = mmap_device_io( BYTE, PORT_DIR_ADDR );

		/* Initialize Echo Reader: */
		out8( portc_dir, DIOCR_PORTC );
		UltrasoundEchoReader * reader = new UltrasoundEchoReader( portc );

		/* Provide user input prompt: */
		printf("Press any key to start measuring...\n");
		c = getch();

		/* Ultrasound measurement thread: */
		reader->startReading();
		c = getch();
		reader->stopReading();

		/* Display minimum & maximum values: */
		printf("\nMinimum distance: %d inches\n", minimum);
		printf("Maximum distance: %d inches \n", maximum);

		/* Restore the old settings (for user input): */
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	}

	return return_code;
}/* main() */
