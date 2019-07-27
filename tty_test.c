/* TTY testing utility (using tty driver)
 * Copyright (c) 2017
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I /path/to/cross-kernel/include
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>  
#include <termios.h>  
#include <errno.h>   
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h>  
 
int speed_arr[] = {
	B115200,
	B57600,
	B38400,
	B19200,
	B9600,
	B4800,
	B2400,
	B1200,
	B300
};
 
int name_arr[] = {
	115200,
	57600,
	38400,
	19200,
	9600,
	4800,
	2400,
	1200,
	300
};
 
/**
 * libtty_setopt - config tty device
 * @fd: device handle
 * @speed: baud rate to set
 * @databits: data bits to set
 * @stopbits: stop bits to set
 * @parity: parity set
 *
 * The function return 0 if success, or -1 if fail.
 */
int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity)
{
	struct termios newtio;
	struct termios oldtio;
	int i;
	
	bzero(&newtio, sizeof(newtio));
	bzero(&oldtio, sizeof(oldtio));
	
	if (tcgetattr(fd, &oldtio) != 0) {
		perror("tcgetattr");    
		return -1; 
	}
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
 
	/* set tty speed */
	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {      
			cfsetispeed(&newtio, speed_arr[i]); 
			cfsetospeed(&newtio, speed_arr[i]);   
		} 
	}
	
	/* set data bits */
	switch (databits) {
	case 5:                
		newtio.c_cflag |= CS5;
		break;
	case 6:                
		newtio.c_cflag |= CS6;
		break;
	case 7:                
		newtio.c_cflag |= CS7;
		break;
	case 8:    
		newtio.c_cflag |= CS8;
		break;  
	default:   
		fprintf(stderr, "unsupported data size\n");
		return -1; 
	}
	
	/* set parity */
	switch (parity) {  
	case 'n':
	case 'N':
		newtio.c_cflag &= ~PARENB;    /* Clear parity enable */
		newtio.c_iflag &= ~INPCK;     /* Disable input parity check */
		break; 
	case 'o':  
	case 'O':    
		newtio.c_cflag |= (PARODD | PARENB); /* Odd parity instead of even */
		newtio.c_iflag |= INPCK;     /* Enable input parity check */
		break; 
	case 'e': 
	case 'E':  
		newtio.c_cflag |= PARENB;    /* Enable parity */   
		newtio.c_cflag &= ~PARODD;   /* Even parity instead of odd */  
		newtio.c_iflag |= INPCK;     /* Enable input parity check */
		break;
	default:  
		fprintf(stderr, "unsupported parity\n");
		return -1; 
	} 
	
	/* set stop bits */ 
	switch (stopbits) {  
	case 1:   
		newtio.c_cflag &= ~CSTOPB; 
		break;
	case 2:   
		newtio.c_cflag |= CSTOPB; 
		break;
	default:   
		perror("unsupported stop bits\n"); 
		return -1;
	}
 
	newtio.c_cc[VTIME] = 0;	  /* Time-out value (tenths of a second) [!ICANON]. */
	newtio.c_cc[VMIN] = 0;    /* Minimum number of bytes read at once [!ICANON]. */
	
	tcflush(fd, TCIOFLUSH);  
	
	if (tcsetattr(fd, TCSANOW, &newtio) != 0)  
	{
		perror("tcsetattr");
		return -1;
	}
	return 0;
}
 
/**
 * libtty_open - open tty device
 * @devname: the device name to open
 *
 * In this demo device is opened blocked, you could modify it at will.
 */
int libtty_open(const char *devname)
{
	int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY); 
	int flags = 0;
	
	if (fd == -1) {                        
		perror("open device failed");
		return -1;            
	}
	
	flags = fcntl(fd, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		printf("fcntl failed.\n");
		return -1;
	}
		
	if (isatty(fd) == 0)
	{
		printf("not tty device.\n");
		return -1;
	}
	else
		printf("tty device test ok.\n");
	
	return fd;
}
 
/**
 * libtty_close - close tty device
 * @fd: the device handle
 *
 */
int libtty_close(int fd)
{
	return close(fd);
}
 
void tty_test(int fd)
{
	int nwrite, nread;
	char buf[100];
	int i;
	
	memset(buf, 0x32, sizeof(buf));
	
	while (1) {
		nwrite = write(fd, buf, sizeof(buf));
		printf("wrote %d bytes already.\n", nwrite);
		nread = read(fd, buf, sizeof(buf));
		printf("read %d bytes already.\n", nread);
		printf("*************************\n");
		for (i = 0; i < nread; i++)
			printf(" 0x%.2x", buf[i]);
		printf("\n*************************\n");
		sleep(5);
	}
	
}
 
int main(int argc, char *argv[])
{
	int fd;
	int ret;
	
	fd = libtty_open("/dev/ttyUSB0");
	if (fd < 0) {
		printf("libtty_open error.\n");
		exit(0);
	}
	
	ret = libtty_setopt(fd, 9600, 8, 1, 'n');
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		exit(0);
	}
 
	tty_test(fd);
	
	ret = libtty_close(fd);
	if (ret != 0) {
		printf("libtty_close error.\n");
		exit(0);
	}
}
