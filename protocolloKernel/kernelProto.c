/**********************************************************
 * file name: kernelProto.c
 * content  : main file to manage the KernelSistemi protocol on serial bus/ports
 * author   : Glauco Bordoni glauco.bordoni@elebor.it
 * date     : 2023-03-20
 * state    : develop <develop/testing/completed>
 * tester   :
 * usage    :
 *     ./kernelProto ttyUSB0 19200
 ********************************************************* */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> //true, false
#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h>
#include <time.h> //int nanosleep(const struct timespec *req, struct timespec *rem);
#include <errno.h>    


#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>


#include "kernelProto.h"

#define STX	0x02
#define ETX	0x03
#define EOT	0x04
#define ACK	0x06
#define NAK	0x16

//bool verbose=true;
bool verbose=false;

int calcolaCheckSum(char *);

int main (int argc, char *argv[]) {
    char *tty; //serial port name ex: /dev/ttyUSB1
	char msg[255];//message to be displayed by echo

    int serial_port_handler = -1;
	struct termios ttyParam;
    
    DWORD bitrate;	//serial port speed
    DWORD flags;    //serial port setting flags
	
	char read_input_ans [256];
	
	printf("\n --== kernelProto started ==--\n");

    if (argc < 3) {
		error( serial_port_handler,-4,"Usage: kernelProto <Serial Port> <Bit rate> -v\nExample:\n\tkernelProto /dev/ttyUSB1 19200\n\tkernelProto ttyS3 9600\n");
	}

	if (argc == 4) {
		if (strcmp(argv[3],"-v")==0){
			verbose=true;
			printf("Verbose mode enabled.\n",argv[3]);
		}
	}
	
	//set serial the device 
    tty = argv[1]; //ex: /dev/ttyUSB1

	// set the boudrate
	bitrate=atoi(argv[2]);

    if ((bitrate != 19200 ) && (bitrate != 38400)){
		char str[255];//=char[
		sprintf(str,"Wrong bitrate %ld. Only 19200 and 38400 allowed\n", bitrate);
		error( serial_port_handler,-7,str);
	}
	
	speed_t newbaudrate = 0;
	
	switch (bitrate){
		case 38400:
				newbaudrate = B38400;
			break;
		case 19200:
		default:
				newbaudrate = B19200;
			break;		
	}


	//open the serial port in read and write mode
    serial_port_handler = open(tty, O_RDWR);
    if (serial_port_handler <= 0){
		char str[255];//=char[
		sprintf(str,"open failed for %s with %d\n", tty, serial_port_handler);
		error( serial_port_handler,-1,str);
	}
    printf("Serial port opened in Read and Write mode.\n");



	//set the speed speed 
	cfsetispeed(&ttyParam, newbaudrate); //set input speed
	cfsetospeed(&ttyParam, newbaudrate);	//set output speed

	ttyParam.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)

	ttyParam.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	
	ttyParam.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
	ttyParam.c_cflag |= CS8; // 8 bits per byte (most common)

	ttyParam.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	ttyParam.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
	
	ttyParam.c_lflag &= ~ICANON;	//DISABLE CANONICAL MODE(DON'T WAIT FOR \n
	
	ttyParam.c_lflag &= ~ECHO; // Disable echo
	
	ttyParam.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	
	ttyParam.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	ttyParam.c_cc[VMIN] = 0; // Block until VMIN characters have been received
	
	if (tcsetattr(serial_port_handler, TCSANOW, &ttyParam) != 0) {
		char str[255];//=char[
		sprintf(str,"Error from tcsetattr: %s\n", strerror(errno));
		error( serial_port_handler,-3,str);
	}
	
	printf("Set speed %d to %s with tcsetattr\n", newbaudrate, tty);
	
/*	

	unsigned char read_input_msg[]   = { STX , '0', '1', 'd', '0', '0', '0', '0' , '0', '1', 'E', '6', ETX};
	unsigned char write_output_msg[] = { STX , '0', '1', 'd', '0', '0', '0', '0' , '0', '1', 'E', '6', ETX};
	
	
	write(serial_port_handler, read_input_msg, sizeof(read_input_msg));
	
	sprintf(msg,"command sent: %s\n",read_input_msg);
	echo(msg);
	
	msleep(10); //wait 10 milliseconds
	
	memset(&read_input_ans, 0, sizeof(read_input_ans));

	// Read bytes. The behaviour of read() (e.g. does it block?,
	// how long does it block for?) depends on the configuration
	// settings above, specifically VMIN and VTIME
	int n = read(serial_port_handler, &read_input_ans, sizeof(read_input_ans));
	printf("received: %s\n",read_input_ans);
	
	close(serial_port_handler);
	
	return 0;
*/
	/* **********************************************************
	 *  start the named socket
     ************************************************************ */
	char *socket_path = "/tmp/icp-kernel";
	struct sockaddr_un addr;
	char buf[100];					//socekt receiving buffer
	int fd,cl,rc;					//socket file descriptor, socket connection link, socket dreceived status 
	
	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		error(serial_port_handler,-30,"socket error\n");
	}
	printf("socket created, fd=%d\n",fd);

	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (*socket_path == '\0') {
		*addr.sun_path = '\0';
		strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
	} else {
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
		echo("cleaning up the socket\n");
		unlink(socket_path);
	}

	printf("setup up the bind.\n");
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		error(serial_port_handler,-31,"bind error\n");
	}
	
	printf("start listening.\n");
	if (listen(fd, 5) == -1) {
		error(serial_port_handler,-32,"listen error\n");
		exit(-1);
	}

	printf("start waiting for data from the socket.\n");

	//command managemtn variables
	char cmd[256]; 				//stores the command to be delivered on serial line
	int counter=0;				//number of charactres of command to deliver on serial line
	const char delim[] = ","; 	//socket message field delimiter
	int tokenCounter=0;			//number of field of the message managed
	char *ptr= NULL;				//pointer to the token 					
	int numeroData=0;			
	bool writeCommand=false;	//false= READ command , true = WRITE command			

	//dati ricevuti
	int nunCharRead=0;
	char str[255];
	char dummy[103];
	char subbuff[5];
	
	while (1) { //infinite loop
		echo("waiting for connection to the socket.\n");
		if ( (cl = accept(fd, NULL, NULL)) == -1) {
			perror("accept error");
			continue;
		}
		echo("Socket connection set. Waiting for data.\n");
		memset(buf, 0, sizeof(buf)); //clena up the socket receive buffer
		
		while ( (rc=read(cl,buf,sizeof(buf))) > 0) {
			sprintf(msg,"read %u bytes: %.*s\n", rc, rc, buf); //stampa la stringa della dimensione giusta
			echo(msg);

			/**
			message format:
			device_address, command, data_address, number_of_data, val1 ,val2....

			device_address: the address of the device on the serial bus
			command       : string "READ", "WRITE"
			data_address  : the address of the first data to read/write
			number_of_data: the number of data to read
			val1..valn    : the values to be written
				
			*/
			
			memset(cmd, 0, sizeof(cmd)); //clean up the command string

			ptr = strtok(buf, delim);	//tokenize the message received
			
			tokenCounter=0;		//number of token founded
			numeroData=0;		//number of dataword to read
			counter=0;			//index of first free character to the command string
			cmd[counter]=STX;	//the command string 
			counter++;			//increment the index to free character
			writeCommand=false;	//false= READ command , true = WRITE command		
								//default is READ command -> it doesn't have to manage the EOT character
			
			// START of COMMAND INTERPRETATION
			while(ptr != NULL) {
							
				switch (tokenCounter) {
					case 0: // slave address
						sprintf(&cmd[counter],"%02X",atoi(ptr));
						counter+=2;
						break;
					case 1:	// type of command
						if (strstr(ptr, "READ") != NULL) 
							cmd[counter]='d';
						else {//if (strstr(ptr, "WRITE") != NULL) { 
							cmd[counter]='D';
							writeCommand=true;		//false= READ command , true = WRITE command	//[ un comando di scrittura deve gestie
						}
						counter++;
						break;
					case 2: //data address
						sprintf(&cmd[counter],"%04X",atoi(ptr));
						counter+=4;
						break;		
					case 3: //number of data to read or first data's value
						if (writeCommand==false) {	//false= READ command , true = WRITE command
													//read  numeroData data
 							// 01d000001E6
							numeroData=atoi(ptr);
							sprintf(&cmd[counter],"%02X",numeroData);
							counter+=2;
						} else {          //write -> first data value to send
						    // 01D000100222E	
							numeroData=-1;
							sprintf(&cmd[counter],"%04X",atoi(ptr));
							counter+=4;
						}
						break;		
					default	:	//following data values
						sprintf(&cmd[counter],"%04X",atoi(ptr));
						counter+=4;
						break;
				}
				tokenCounter++;
				//printf("-->%d,%s\n",counter,cmd);
				ptr = strtok(NULL, delim); //next token
			}
			
			if (writeCommand==true)	//false= READ command , true = WRITE command {
				cmd[counter++]=EOT;
			
			
			calcolaCheckSum(cmd); //calculate the checksum for the comman to be sent on serial line
			cmd[strlen(cmd)]=ETX;


			// deliver of command on serial line
			if (verbose)
				printf("command to send: %s\n",cmd);
			
			write(serial_port_handler, cmd, strlen(cmd)); //write the command on the serial line


			// waiting for answer
			msleep(30); //wait 30 milliseconds			
			memset(&read_input_ans, 0, sizeof(read_input_ans)); //clean up the receiving buffer

			if (writeCommand==true)	{ //false= READ command , true = WRITE command 
				nunCharRead = read(serial_port_handler, &read_input_ans, 5);
				sprintf(msg,"received %d chars: ",nunCharRead);
				echo(msg);
				if (verbose) {
					for (int i=0; i < nunCharRead; i++){
						printf("%d,",read_input_ans[i]);
					}
					printf("\n");
				}
			} else {
				nunCharRead = read(serial_port_handler, &read_input_ans, 5+(4*numeroData));
				if (verbose) 
					printf("received %d chars: %s\n",nunCharRead,read_input_ans);
			}
			

			//prepare the JSON answer
			memset(&str, 0, sizeof(str)); //clean up the buffer of the message to send
			sprintf(str,"{\"tot\":%d,\n\"risp\":\n[",numeroData);
			if (writeCommand==true)	{ 		//if  WRITE command 
				if (str[1]==ACK)			//check the ACK/NACK char	
					strcat (str,"\"OK\"");	//good
				else if (str[1]==NAK)		//check the ACK/NACK char	
					strcat (str,"\"KO\"");	//bad
				else
					strcat (str,"\"??\"");	//bad
			} else {
				for (int iter=0; iter< numeroData; iter++){
					if (iter)
						strcat(str,",");
					memset(&subbuff, 0, sizeof(subbuff));
					memcpy( subbuff, &read_input_ans[1+iter*4], 4 );
					
					sprintf(dummy,"{\"id\":%d,\"val\":%ld}\n",iter,strtol(subbuff, NULL, 16));
					strcat (str,dummy);
				}
			}
			
			strcat(str,"]}");

			//send the answer
			if (verbose)
				printf("risposta=\n%s\n",str);
			write(cl,str,strlen(str));
			
			//clean up for next message
			strtok("ciao","c");	//it is a dummy way to reset strtok function to operate again 
			memset(buf, 0, sizeof(buf));
		}
	

		if (rc == -1) {
			error(serial_port_handler,-33,"Error in reading socket.");
		}

		else if (rc == 0) {
			echo("Socket connection closed by client (EOF)\n");
			close(cl);
		}
	}


    // wait(1l);
    echo("Normal termination \n");
    close(serial_port_handler);
    echo("Serila port closed.\n");
    return 0;
}

/**
 * calcolaCheckSum caomputes the checksum, it jumps the first character and append the checksum to the received string in fondo alla stringa rievuta.
 * cmd		: the string to calculate the cehckcsm on 
 * retrrn	: the value of checksum 
 **/
int calcolaCheckSum(char *cmd){
	int chk=0;
	int poschk=strlen(cmd);

	
	//skip STX,... ,CHK1,CHK2 & ETX
	for(int iter=1;iter<poschk;iter++){	
		chk+=cmd[iter];
		//printf("-chk->%d,%c,%d\n",iter,cmd[iter],chk);
	}
	chk%=256;
	sprintf(&cmd[poschk],"%02X", chk);
	return chk;
}




/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec){
	if (verbose)
		printf("Waits for %ld microseconds\n",msec);
    struct timespec ts;
    int res;

    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


/**
 * display the error message close the filesa amd exit with errroe code
 * param:
 * int h        : file handler
 * int exitCode : the xit code to be returned
 * char* msg	: the tring message 
 */
void error(int h,int exitCode, char* msg){
	perror(msg);
	if (h>-1)
		close(h);
	exit(exitCode);
}

/**
 * print the message if the program is in verbose mode 
 */
inline void echo(char *testo){
	if (verbose)
		printf("%s",testo);
}	
