#include "main_uart.h"
#include "atcid.h"

static int speed_arr[] = {B921600, B460800, B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1800, B1200, B600, B300, B200};
static int name_arr[] = {921600,  460800,  230400,  115200,  57600,  38400,  19200,  9600,  4800,  2400,  1800,  1200,  600,  300,  200};

/*===========================================================================
  FUNCTION  serial_set_speed
===========================================================================*/
/*!
@brief
  set serial speed.

@param
 *      int fd [IN]:	serial fd
 *      int speed [IN]:	baud rate lock in name_arr speed_arr

@return
  0  on success
  -1 on failure

@note
  NONE.
*/
/*==========================================================================*/
int serial_set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios options;
	if (tcgetattr(fd, &options) != 0)
	{
		perror("serial_set_speed 1");
		return -1;
	}
	for (i=0; i<sizeof(speed_arr)/sizeof(int); i++)
	{
		if (speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &options);
			if (status != 0)
			{
				perror("serial_set_speed 2");
				return -1;
			}
			tcflush(fd, TCIOFLUSH);
			return 0;
		}
	}
	return -1;
}

/*===========================================================================
  FUNCTION  serial_set_parity
===========================================================================*/
/*!
@brief
  set serial parity.

@param
 *      int fd [IN]:	serial fd
 *      int databits [IN]:	databits
 *      int stopbits [IN]:	stopbits
 *      int parity [IN]:	parity

@return
  0  on success
  -1 on failure

@note
  NONE.
*/
/*==========================================================================*/
int serial_set_parity(int fd, int databits, int stopbits, int parity)
{
	struct termios options;
	if (tcgetattr(fd, &options) != 0)
	{
		perror("serial_set_parity 1");
		return -1;
	}
	
	tcflush(fd, TCIOFLUSH);

	//设置数据位数
	options.c_cflag &= ~CSIZE;
	switch (databits) 
	{
		case 5:
			options.c_cflag |= CS5;
			break;
		case 6:
			options.c_cflag |= CS6;
			break;
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			perror("Unsupported data sizen");
			return -1;
	}
	
	//设置校验位
	switch (parity)
	{
		case 'n':
		case 'N':
			//Clear parity enable
			options.c_cflag &= ~PARENB;
			//Enable parity checking
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
			//设置为奇效验
			options.c_cflag |= (PARODD | PARENB);
			//Disnable parity checking
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':
			//Enable parity
			options.c_cflag |= PARENB;
			//转换为偶效验
			options.c_cflag &= ~PARODD;
			//Disnable parity checking
			options.c_iflag |= INPCK;       
			break;
		case 'S':
		case 's':
			//as no parity
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			perror("unsupported parityn");
			return -1;
	}
	
	//设置停止位
	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			perror("unsupported stop bitsn");
			return -1;
	}
	
	//Set input parity option/
	if (parity != 'n')
	{
		options.c_iflag |= INPCK;
	}
	
	//不做转义
	options.c_iflag &= ~(INLCR | IGNCR | ICRNL);
	options.c_oflag &= ~(ONLCR | OCRNL);
	//不等回车换行
	//options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//add by jerry, 打开回显功能
	options.c_lflag = ICANON | ECHO;
	//屏蔽软件流控
	options.c_iflag &= ~(IXON | IXOFF | IXANY);

	//add by jerry, ignore "\n"
	options.c_iflag |= (INLCR | IGNCR);

	//设置超时15 seconds
	cfmakeraw(&options);
	options.c_cc[VTIME] = 0;
	//Update the options and do it NOW
	options.c_cc[VMIN] = 1; 

	tcflush(fd, TCIFLUSH);
	
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("serial_set_parity 2");
		return -1;
	}

	//for debug
	if (tcgetattr(fd, &options) != 0)
	{
		perror("serial_set_parity 1");
		return -1;
	}
	tcflush(fd, TCIOFLUSH);

	if(options.c_lflag & ICANON)
	{
		LOGATCI(LOG_INFO, "have ICANON flag!");
	}
	if(options.c_lflag & ECHO)
	{
		LOGATCI(LOG_INFO, "have ECHO flag!");
	}
	return 0;
}