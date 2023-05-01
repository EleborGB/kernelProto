/**********************************************************
 * file name: kernelProto.h
 * content  : header file to manage the KernelSistemi protocol on serial bus/ports
 * author   : Glauco Bordoni glauco.bordoni@elebor.it
 * date     : 2023-03-20
 * state    : develop <develop/testing/completed>
 * tester   :
 * usage    :
 ********************************************************* */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */



#ifdef WIN32
	#include <windows.h>
#else
	typedef void VOID;
	typedef char CHAR;
	typedef unsigned long DWORD;
	typedef unsigned char UCHAR;
	typedef unsigned short USHORT;
	typedef unsigned long ULONG;
#endif

#ifndef UINT8
#define UINT8	UCHAR
#endif
#ifndef UINT16
#define UINT16	USHORT
#endif
#ifndef UINT32
#define UINT32	unsigned int
#endif

/** 
 * error prints the error, closes the opened files and exits with error code
 * int    fileDescriptor (serial port file)
 * int    error terminating code
 * char*  the message to be printed
 */
void error(int, int, char*);


/**
 * msleep stops the program for arg millseconds
 * long the dalay time amount in milliseconds
 */
int msleep(long );


/**
 * echo prints the message if the program is in verbose mode (verbose global var!=0
 */
void echo(char *);