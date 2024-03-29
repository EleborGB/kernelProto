# KernelProto

KernelProto is a Linux program that export API to communicate with *Kernel Sistemi* PLC and expansion cards.

## Table of Contents
- [KernelProto](#kernelproto)
  - [Table of Contents](#table-of-contents)
  - [General Information](#general-information)
  - [Technologies Used](#technologies-used)
  - [Features](#features)
  - [Setup](#setup)
  - [Usage](#usage)
  - [Project Status](#project-status)
  - [Room for Improvement](#room-for-improvement)
  - [Acknowledgements](#acknowledgements)
  - [Contact](#contact)
  - [License](#license)
<!-- * [License](#license) -->


## General Information
KernelProto is a program, to be run as daemon, written in C language that export API via named socket to communicate withe *Kernel Sistemi* PLC and expansion cards.
KernelProto permits to read and write data into PLC and/or an expansion card using *Kernel Sistemi* protocol.
KernelProto is written in pure C to run in Linux environment. 
KernelProto was developed in the aim to give fast serial port access to high level languages. ex. Javascript & Nodejs, Python , Java. 



## Technologies Used

- c compiler : gcc ver. 9.4.0 or more (no tested with earlier versions, I expect it works with earlier versions)
- JSON format: for answers deliverd on socket


## Features

KernelProto uses the named socket technology to implement the "inter process communication" (IPC) between different progrmas written in defferent languages.

The name of the opened socket is
/tmp/icp-kernel

The format of the message processed is:

device_address, command, data_address, number_of_data, val1 ,val2...

where:

device_address: the address of the device on the serial bus
command       : string "READ", "WRITE"
data_address  : the address of the first data to read/write
number_of_data: the number of data to read
val1...valn    : the values to be written


## Setup

Compile in Linux

gcc kernelProto.c -o kernelProto


## Usage

Open a shell
To run the KernelProto as daemon (see the & at the end) on serial port COM1 write:  

`>KernelProto /dev/tty0 19200 &`

if you use an USB adapter change the device to:

`> KernelProto /dev/ttyUSB0 19200 &`

if you want a verbose output write instead:

`> KernelProto /dev/ttyUSB0 19200 -v`


## Project Status

Project is sufficent for on street use, but of course it is in progress.
Any suggestion, bugs signalation or request of improvement are welcome.


## Room for Improvement

The library is quite flat, some messages for quality management can be added (i.e. status,logging). 

Room for improvement:
- add a message *stop/exit*  to stop the loibrary
- add a message *restart*  to restart the library


## Acknowledgements

Thanks for the support, the explanation and the patience to:
- ing. Giorgio Luppi of Kernel Sistemi srl
- Luca Morisi of Kernel Sistem srl


## Contact

Created by [elebor GB srl](http://www.elebor.it/) - feel free to contact for bugs submission, improvement suggstions  


## License

This project is open source and available under the [Mozzilla License](https://mozilla.org/MPL/2.0/)


