/*
 * UltrasoundEchoReader.h
 *
 *  Created on: Nov 4, 2013
 *      Author: lgs8331
 */
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
	UltrasoundEchoReader(uintptr_t portc);
	virtual ~UltrasoundEchoReader();

	void startReading();
	void stopReading();
	int getDistance( uint64_t start, uint64_t end );
	void run();
protected:


private:
	static void * UltrasoundRunFunction(void * This) {((UltrasoundEchoReader*)This)->run(); return NULL;}
	pthread_t _thread;
	uintptr_t _portc;

};
#endif /* ULTRASOUNDECHOREADER_H_ */
