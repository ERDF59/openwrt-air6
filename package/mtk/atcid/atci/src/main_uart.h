#ifndef ATCID_MAIN_UART_H
#define ATCID_MAIN_UART_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

int serial_set_speed(int fd, int speed);
int serial_set_parity(int fd, int databits, int stopbits, int parity);

#endif