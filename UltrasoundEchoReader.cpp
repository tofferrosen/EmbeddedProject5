/* =========================================================================== *
 * File:		UltrasoundEchoReader.cpp
 * Author:		Christoffer Rosen, Lennard Streat
 * Description:	Interacts with an SRF04 Ultrasound range sensor to measure
 *				the distance from the SRF04 to objects in it's path.
 * =========================================================================== */
#include "UltrasoundEchoReader.h"

/* Bounds on range for valid sensor readings: */
#define MAXBOUND (117)	/* Inches */
#define MINBOUND (3)	/* Inches */

/* Timing & I/O: */
#define DELAY_TIME (97085000)	/* Min delay between measurements */
#define PULSE_WIDTH (15000)		/* Pulse width (in ns) used for input trigger */
#define LOGIC_HIGH (0x0F)		/* For the input trigger */
#define LOGIC_LOW (0x00)		/* For the input trigger */
#define C4BIT (0b00010000)		/* A mask used to access the 4th bit of Port C */

/* Maximum and minimum values of sensor: */
unsigned int maximum = 0;
unsigned int minimum = -1;

/**
 * Function:	UltrasoundEchoReader
 * Description:	Constructor for the UltrasoundEchoReader class.
 * Returns:		A new UltrasoundEchoReader object.
 */
UltrasoundEchoReader::UltrasoundEchoReader(uintptr_t portc) {
	_portc = portc;
} /* UltrasoundEchoReader() */

UltrasoundEchoReader::~UltrasoundEchoReader() {} /* ~UltrasoundEchoReader() */

/**
 * Function:	run
 * Description:	Reads the input from the SRF04 range sensor and prints the
 *				measurement to the console. Handles updating the maximum and
 *				minimum global variables. They were not protected, because
 *				at most, only one thread would ever be accessing them.
 * Returns:		N/A
 */
void UltrasoundEchoReader::run( void ){
	uint8_t data = 0x00;
	uint64_t start, end;
	unsigned int distance;
	int flicker = 0;

	while(_running){
		/* Send pulse to SRF04 to begin reading range: */
		out8( _portc, LOGIC_HIGH );
		nanospin_ns( PULSE_WIDTH );
		out8( _portc, LOGIC_LOW );

		/* Read length of SRF04 output pulse: */
		while( (in8(_portc) & C4BIT) == 0  ); /* Wait for signal to go high */
		start = ClockCycles(); /* Capture the time when signal rose high */
		while( (in8(_portc) & C4BIT) > 0 ){ /* Wait for signal to fall */
			sched_yield();
		}
		end = ClockCycles(); /* Capture the time when signal fell low */
		distance = getDistance(start,end); /* Calculate distance */

		/* Update the minimum & maximum value measured: */
		if ( distance > MINBOUND && distance < MAXBOUND ) {
			if ( distance > maximum ) {
				maximum = distance;
			}
			if ( distance < minimum ) {
				minimum = distance;
			}
			/* Padding clears previous print and \r overwrites previous line: */
			printf( "\rDistance: %din                    ", distance );
		} else {
			/* Display flashing asterisk when out of range */
			if( flicker ){
				printf("\rDistance:                      ");
			}else{
				printf("\rDistance: *                    ");
			}
			flicker = !flicker;
		}
		nanospin_ns( DELAY_TIME );
	}
} /* run() */

/**
 * Function:	startReading
 * Description:	Initializes the internal pthread, which then calls the run
 *				method.
 * Returns:		N/A
 */
void UltrasoundEchoReader::startReading( void ){
	_running = true;
	int rc = pthread_create(&_thread, NULL, UltrasoundRunFunction, this);
	if(rc){
		/* Error detected: */
		fprintf(stderr, "ERROR(%d): Pthread not created.\n", rc);
		return;
	}
} /* startReading() */

/**
 * Function:	stopReading
 * Description:	Attempts to join the UltrasoundEchoReader thread with the parent
 *				thread.
 * Returns:		N/A
 */
void UltrasoundEchoReader::stopReading( void ) {
	_running = false;
	pthread_join(_thread, NULL);
} /* stopReading() */

/**
 * Function:	getdistance
 * Description:	Calculates the distance that the sensor should read, given
 *				some input CPU clock cycle measurements.
 * Returns:		Measurement, in inches as an unsigned integer.
 */
unsigned int UltrasoundEchoReader::getDistance( uint64_t start, uint64_t end ){
	uint64_t diff = end - start;
	float diffms = (float)diff / ((float)(SYSPAGE_ENTRY(qtime)->cycles_per_sec)) * 1000.0;
	return (unsigned int)((diffms * 13.54408)/2);
} /* getDistance() */
