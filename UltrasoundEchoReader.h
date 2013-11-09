/* =========================================================================== *
 * File:		UltrasoundEchoReader.h
 * Author:		Christoffer Rosen, Lennard Streat
 * Description:	The header for the UltrasoundEchoReader class.
 * =========================================================================== */
#include <stdio.h>
#include <stdlib.h>			/* Resource(s): EXIT_* */
#include <stdint.h>			/* Resource(s): uintptr_t */
#include <pthread.h>
#include <hw/inout.h>		/* Resource(s): in*(), out*() */
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>	/* Resource(s): cycles_per_second */

#ifndef ULTRASOUNDECHOREADER_H_
#define ULTRASOUNDECHOREADER_H_

class UltrasoundEchoReader {
public:
	/* Object Construction: */
	UltrasoundEchoReader( uintptr_t portc );
	virtual ~UltrasoundEchoReader( void );

	/* Threading: */
	void startReading( void );
	void stopReading( void );

	/* Measurement: */
	unsigned int getDistance( uint64_t start, uint64_t end );

protected:
	void run( void );
private:
	static void * UltrasoundRunFunction(void * This) {((UltrasoundEchoReader*)This)->run(); return NULL;}
	pthread_t _thread;
	uintptr_t _portc;
	bool _running;
}; /* UltrasoundEchoReader */
#endif /* ULTRASOUNDECHOREADER_H_ */
