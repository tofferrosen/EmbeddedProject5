/*
 * UltrasoundEchoReader.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: lgs8331
 */

#include "UltrasoundEchoReader.h"

float msCalc( uint64_t start, uint64_t end );

UltrasoundEchoReader::UltrasoundEchoReader(uintptr_t portc) {
	_portc = portc;
}

UltrasoundEchoReader::~UltrasoundEchoReader() {
	// TODO Auto-generated destructor stub
}

void UltrasoundEchoReader::run(){
	uint8_t data = 0x00;
	uint8_t mask = 0b00001010;
	uint8_t ipmask = 0b00010000;
	uint64_t start, end;

	while(1){
		out8( _portc, 0x0F );
		nanospin_ns(10000);
		out8( _portc, 0x00 );

		while( (in8(_portc) & ipmask) == 0  ){
			 usleep(100);
		}
		start = ClockCycles();
		while( (in8(_portc) & ipmask) > 0 ){
			 usleep(100);
		}
		end = ClockCycles();
		printf("(%lld,%lld,%f)\r",start,end,msCalc(start,end));
		nanospin_ns(10000000);
	}
}

void UltrasoundEchoReader::startReading(){
	int rc = pthread_create(&_thread, NULL, UltrasoundRunFunction, this);
	if(rc){
		//ERROR
		fprintf(stderr, "ERROR(%d): Pthread not created.\n", rc);
		return;
	}
}

float msCalc( uint64_t start, uint64_t end ){
	uint64_t diff = end - start;
	return (float)diff / ((float)(SYSPAGE_ENTRY(qtime)->cycles_per_sec)) * 1000.0;
}
