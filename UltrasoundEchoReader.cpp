/*
 * UltrasoundEchoReader.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: lgs8331
 */

#include "UltrasoundEchoReader.h"

#define MAXBOUND (117) /* Inches */
#define MINBOUND (3) /* Inches */

int maximum = 0;
int minimum = 999999;

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
	int distance;
	int flicker = 0;

	while(_running){
		out8( _portc, 0x0F );
		nanospin_ns(15000);
		out8( _portc, 0x00 );

		while( (in8(_portc) & ipmask) == 0  ){
		//	usleep(100);
		}
		start = ClockCycles();
		while( (in8(_portc) & ipmask) > 0 ){
			sched_yield();
			//usleep(100);
		}
		end = ClockCycles();
		distance = getDistance(start,end);

		if (distance > MINBOUND && distance < MAXBOUND) {
			if (distance > maximum) {
				maximum = distance;
			}

			if (distance < minimum) {
				minimum = distance;
			}
			// extra padding
			printf("\rDistance: %din          ", distance);
		} else {
			// flashing asterisk
			if( flicker ){
				printf("\rDistance:          ");
			}else{
				printf("\rDistance: *          ");
			}
			flicker = !flicker;
		}
		nanospin_ns(1250000);
	}
}

void UltrasoundEchoReader::startReading(){
	_running = true;
	int rc = pthread_create(&_thread, NULL, UltrasoundRunFunction, this);
	if(rc){
		//ERROR
		fprintf(stderr, "ERROR(%d): Pthread not created.\n", rc);
		return;
	}
}

void UltrasoundEchoReader::stopReading() {
	_running = false;
	pthread_join(_thread, NULL);
}

int UltrasoundEchoReader::getDistance( uint64_t start, uint64_t end ){
	uint64_t diff = end - start;
	float diffms = (float)diff / ((float)(SYSPAGE_ENTRY(qtime)->cycles_per_sec)) * 1000.0;
	return (int)((diffms * 13.54408)/2);
}
