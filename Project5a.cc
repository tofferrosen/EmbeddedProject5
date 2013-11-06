/* =========================================================================== *
 * File:		gpio.c
 * Author:		Lennard Streat
 * Description:	Initializes general GPIO on the digital IO port.
 * =========================================================================== */

#include <stdio.h>			/* Resource(s): printf() */
#include <stdlib.h>			/* Resource(s): EXIT_* */
#include <stdint.h>			/* Resource(s): uintptr_t */
#include <hw/inout.h>		/* Resource(s): in*(), out*() */
#include <sys/neutrino.h>	/* Resource(s): ThreadCtl() */
#include <sys/syspage.h>	/* Resource(s): cycles_per_second */
#include <sys/mman.h>		/* Resource(s): mmap_device_io() */
#include <termios.h>
#include <unistd.h>

#include "UltrasoundEchoReader.h"

#define BYTE (1)
#define WORD (2)
#define DWORD (4)

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


struct termios old_termios_p;

int raw(int fd)
{
struct termios termios_p;

if( tcgetattr( fd, &termios_p ) )
return( -1 );

termios_p.c_cc[VMIN] = 1;
termios_p.c_cc[VTIME] = 0;
termios_p.c_lflag &= ~( ECHO|ICANON|ECHOE|ECHOK|ECHONL );
termios_p.c_oflag &= ~( OPOST );

return( tcsetattr( fd, TCSADRAIN, &termios_p ) );
}

int unraw(int fd)
{
struct termios termios_p;

if( tcgetattr( fd, &termios_p ) )
return( -1 );

termios_p.c_lflag |= ( ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL );
termios_p.c_oflag |= ( OPOST );

return( tcsetattr( fd, TCSADRAIN, &termios_p ) );
}

int main( void ){
	/* Error Handling: */
	int privity_err;
	int return_code = EXIT_SUCCESS;

	/* Memory Mapped IO */
	uintptr_t portc;
	uintptr_t portc_dir;
	uint8_t cdata;

	/* Timing */
	uint64_t cps;
	uint64_t diff;
	float diff_ms;
	char buf;
	char c;
	char newline = '\n';
	char newline2 = '\r';

	/* Enable GPIO access to the current thread: */
	privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
	if( privity_err == -1 ){
		printf( "Error: Unable to acquire root permission for GPIO.\n" );
		return_code = EXIT_FAILURE;
	}else{
		/* Create a pointer to Data Acquisition Port (Memory Mapped IO) */
		portc = mmap_device_io( BYTE, PORTC_ADDR );
		portc_dir = mmap_device_io( BYTE, PORT_DIR_ADDR );

		// Initialize the reader
		UltrasoundEchoReader * reader = new UltrasoundEchoReader(portc);

		printf("Press any key to start measuring...\n");
		// wait for char input to start it
		raw(STDIN_FILENO);
		read(STDIN_FILENO, &buf, sizeof(buf));

		//printf("%c",c);
		reader->run();

		read(STDIN_FILENO, &buf, sizeof(buf));

		printf("EXIT!");

		unraw(STDIN_FILENO);
	}

	return return_code;
}/* main() */
