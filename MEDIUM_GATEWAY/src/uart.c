
#include "PUBLIC.h"
#include <pthread.h>

#include "uart_api.h"

#undef DEBUG 

static int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300, B110, };
static int baudrate_arr[] = {115200,57600, 38400, 19200,9600,4800,2400,1200,300, 110, };

static UART_DATA uart_data[MAX_CNT];
//static pthread_mutex_t uart_data_lock;
static ST_UART_I Uart_I_Data;

int uart_fd = 0;
//int time_cnt_20 = 0;

//==========================================================================
static int32 Uart_data_process(unsigned char *revbuf, int buf_len);
static int32 Uart_data_process_ack(uint8 *p, uint8 len);
static int32 Uart_data_process_cmd(void);
static int32 set_baudrate(int fd, int baudrate);
static int32 set_Parity(int fd,int databits,int stopbits,int parity);
static int32 decode(uint8 *revbuf, uint8 *buffer, int *msg_len);

/*************************************************************************
I  II型命令切换等处理
*************************************************************************/


static int32 decode(uint8 *revbuf, uint8 *buffer, int *msg_len)
{
	  uint8 *p = NULL;
	  int *len = NULL;
	  int i = 0;
  
	  p = revbuf;
	  len = msg_len;
	  
	  for(i = 0; i < *len; i++, p++)
	  {
		    if((0xdb == *p)&&(0xdc == *(p + 1)))
		    {
			      buffer[i] = 0xc0;
			      p++;
			      (*len)--;
		    }
		    else if((0xdb == *p)&&(0xdd == *(p + 1)))
		    {
			      buffer[i] = 0xdb;
			      p++;
			      (*len)--;
		    }
		    else 
		    {
		       	buffer[i] = *p;
		    }
	  }

	return DRV_OK;
}

int32 Uart_Write(uint8 bPort,unsigned char *p,int bLen)
{
	int nwrite;
	
	if (NULL == p)
	{
		printf ("wirte uart point is NULL.\r\n");
		return DRV_ERR;
	}
	
	if (bLen <= 0)
	{
		return DRV_ERR;
	}
	
	nwrite = write(uart_fd, p ,bLen);
	if (nwrite != bLen)
	{
		printf ("write uart %d is error->expect %d,actual %d.\r\n",bPort,bLen,nwrite);
		return DRV_ERR;
	}

#if 0
	for(nwrite = 0; nwrite < bLen; nwrite++)
	{
		printf("%2x:", p[nwrite]);
	}

	printf("\r\n");
#endif

	return DRV_OK;
}




static int32 Uart_data_process_ack(uint8 *p, uint8 len)
{
	uint8 sendbuf[200];
	uint8 sendct = 0;
	uint8 i;
	int ret;
	uint8 checksum = 0;
	uint8 *tmp_p = p;

	if ((p == NULL)|| (len > 100))
	{
		return DRV_ERR;
	}

	for(i = 0; i < len; i++)
	{
		checksum += *tmp_p++;
	}


	p[len] = checksum;

	len += 1;

	sendbuf[sendct++] = 0xC0;
	for (i=0; i<len; i++)
	{
		if (*(p+i) == 0xc0)
		{
			sendbuf[sendct++] = 0xdb;
			sendbuf[sendct++] = 0xdc;
		}
		else if (*(p+i) == 0xdb)
		{
			sendbuf[sendct++] = 0xdb;
			sendbuf[sendct++] = 0xdd;
		}
		else
		{
			sendbuf[sendct++] = *(p+i);
		}
	}
	
	sendbuf[sendct++] = 0xC0;

	ret = Uart_Write(0, sendbuf ,sendct);

	return DRV_OK;	
}

int switch_vlan_init(void)
{
	char buf[100] = {0};
	uint32 vlan_id = 0;
	int i = 0;
	ST_UART_I msg;

	printf("%s\r\n", __func__);
	
	for(i = 6; i < 12; i++)
	{
		snprintf(&buf[0], sizeof(buf), "vlan%d",  i);
		vlan_id = get_config_var_val(config_cfg, buf);
		if((vlan_id > 0)&&(vlan_id <= 65535))
		{
			printf("vlan%d = 0x%x\r\n", i - 5, vlan_id);
			memset(&msg, 0x00, sizeof(msg));

			msg.type = 0x10;
			msg.cmd = 0x02;
			msg.len = 0x05;
			msg.dat[0] = i - 5;
			msg.dat[1] = (vlan_id & 0xFF);
			msg.dat[2] = (vlan_id & 0xFF00) >> 8;
			msg.dat[3] = 0x00;
			msg.dat[4] = 0x00;		

			Uart_data_process_ack((uint8 *)&msg, 3 + msg.len);
		}
	}

	return DRV_OK;
}


static int32 Uart_data_process_cmd(void)
{
	uint16 cmd_type = *((uint16 *)&Uart_I_Data.type);
	switch (cmd_type)
	{		
		case 0xEFFD:
			printf("Vlan set OK\r\n");
			break;
		case 0xEF79:
			//printf("set OK\r\n");
			break;
		default:
			break;
	}

	return DRV_OK;	
}


static int32 Uart_data_process(unsigned char *revbuf, int buf_len)
{
	uint8 buffer[MAX_DATLEN] = {0};
	uint8 *temp_buf = NULL, *msg_head = NULL;
	uint16 head_c0_cnt = 0, tail_c0_cnt = 0;
	uint8 checksum = 0, checksum_error = 0, length_error = 0;
	int temp_len = 0, i = 0, msg_len = 0, byte_cnt = 0;
	int msg_cnt = 0;

#ifdef DEBUG 
	uint8 *debug_p = NULL
#endif	  
  	//printf("Uart_data_process buf_len = %d \r\n", buf_len);
	if((NULL == revbuf) || buf_len > MAX_DATLEN)
	{
		printf("%s param error \r\n", __func__);
		return DRV_ERR;
	}
  
	 temp_buf = revbuf;
	 memset(&Uart_I_Data, 0, sizeof(Uart_I_Data));

	while(0xc0 == *temp_buf)
  	{
    		temp_buf++;
    		head_c0_cnt++;
		temp_len++;
  	}
	//printf("have %d count C0 before the msg\r\n", head_c0_cnt);
	
	head_c0_cnt = 0;
	
	if (buf_len == temp_len)
  	{
    		printf("all the string are C0 buf_len = %d\r\n", buf_len);
    		return DRV_ERR;                      
  	} 

	while(buf_len > temp_len)
	{		

	  	msg_cnt++;
	  	msg_head = temp_buf;
	  
	  	i = buf_len - temp_len;
		//printf("i = %d\r\n",i);
	  	while(i--)
	  	{
	   		if(0xc0 != *temp_buf)
    		{  
      			byte_cnt++;
				temp_len++;
    		}
    		else 
    		{
      			//tail_c0_cnt++;
      			break;
    		}
    		
    		temp_buf++;
	  	}
		
	  	//printf("byte_cnt = %d\r\n", byte_cnt);
	  	if(byte_cnt < 4)
	  	{
	    		printf("too short msg error byte_cnt = %d buf_len = %d  \
				 msg_cnt = %d \r\n", byte_cnt, buf_len, msg_cnt);
	    		//return DRV_ERR;  
	    		length_error = 1;
	  	}
	  
	  	msg_len = byte_cnt;
	  	
#ifdef DEBUG  	
		debug_p = msg_head;
		for(i = 0; i < msg_len; i ++)
		{
			printf("debug_p[%d] = %x \r\n", i, *debug_p);
			debug_p++;
		}
		 printf("###################\r\n");
#endif
		//printf("*msg_head = 0x%x\r\n", *msg_head);
		//printf("msg_len = %d\r\n", msg_len);
		
	  	decode(msg_head, buffer, &msg_len);
	  
	  	memcpy(&Uart_I_Data, buffer, buffer[1] + 3);
	  
		temp_buf = &(Uart_I_Data.type);

#ifdef DEBUG
		debug_p = temp_buf;
		for(i = 0; i < msg_len; i ++)
		{
			printf("debug_p[%d] = %x \r\n", i, *debug_p);
			debug_p++;
		}
		printf("###################\r\n");		
#endif

	  	for(i = 0; i < Uart_I_Data.len + 3; i++, temp_buf++)
	  	{
	  		//printf("%x:", *temp_buf);
	    	checksum += *temp_buf;
	  	}
	  
	  	if(checksum != *temp_buf)
	  	{
			printf("checksum error buf_len = %d, msg_cnt  = %d\r\n", buf_len, msg_cnt);
			memset(&Uart_I_Data, 0, sizeof(Uart_I_Data));
			checksum_error = 1;
			//return DRV_ERR;
	  	}		

		//Debug_Uart_data_process_cmd(); //debug port

		if((1 != checksum_error) && (1 != length_error))
		{
	  		Uart_data_process_cmd();
		}
		
	  	temp_buf = temp_len + revbuf;
		
		while(0xc0 == *temp_buf)
	  	{
	    		temp_buf++;
	    		head_c0_cnt++;
			temp_len++;
	  	}
		
		if(buf_len == temp_len)
		{
			tail_c0_cnt = head_c0_cnt;
			//printf("have %d count C0 after the msg\r\n", tail_c0_cnt);
			if(1 == checksum_error)
			{
				printf("the  last msg checksum error msg_cnt = %d \r\n", msg_cnt);
				return DRV_ERR;  
			}
			
			if(1 == length_error)
			{
				printf("the  last msg length error msg_cnt = %d \r\n", msg_cnt);
				return DRV_ERR;  
			}
		}
		
		head_c0_cnt = 0;
		byte_cnt = 0;
		checksum = 0;
		checksum_error = 0;
		length_error = 0;
		//printf("temp_len = %d\r\n", temp_len);
		//printf("buf_len = %d\r\n", buf_len);
		
	}
	
  	return DRV_OK;
}

int32 Uart_data_process_20ms(void)
{
	uint8 i = 0;
	//pthread_mutex_lock(&uart_data_lock);
	for(i = 0; i < MAX_CNT; i++)
	{
		if((0x01 == uart_data[i].head.use_flg)
			&&(time_cnt_20 - uart_data[i].head.time_cnt) >= 1)
		{		
			uart_data[i].head.proc_flg = 0x01;
			Uart_data_process(uart_data[i].data,\
					uart_data[i].head.length);

			uart_data[i].head.use_flg = 0x00;
			uart_data[i].head.length = 0x00;
			uart_data[i].head.time_cnt = 0x00;
			uart_data[i].head.proc_flg = 0x00;
		}
	}
	//pthread_mutex_unlock(&uart_data_lock);

	return DRV_OK;	
}


static int32  uart_data_save(uint8 *buf, uint32 len)
{
	uint8 i = 0;
	uint32 length = 0;

	for(i = 0; i < MAX_CNT; i++)
	{
		if(0x00 == uart_data[i].head.use_flg)
		{	
			memcpy(&uart_data[i].data[0], buf, len);
			uart_data[i].head.use_flg = 0x01;
			uart_data[i].head.length = len;
			uart_data[i].head.time_cnt = time_cnt_20;
		}
		else if((0x01 == uart_data[i].head.use_flg)&&
			(0x00 == uart_data[i].head.proc_flg))
		{
			length = uart_data[i].head.length;
			if((len + length)< MAX_DATA_LEN)
			{	
				memcpy(&uart_data[i].data[length], buf, len);
				
				uart_data[i].head.length = len + length;
				uart_data[i].head.time_cnt = time_cnt_20;
			}
			else
			{
				printf("uart_data_save length error len + length = %d\r\n",\
								len + length);	
			}

		}
	}
	
	return DRV_OK;	
}

//===========================================================================
void Uart_Pthread(void)                                                   
{	
	unsigned char  buffer[512] = {0};
	int nread;

	memset (&Uart_I_Data,0,sizeof(Uart_I_Data));
	printf("%s: start \r\n", __func__);

	while (1)
	{		
		nread = read(uart_fd, buffer, sizeof(buffer)); 
		if (nread > 0)
		{
			int i = 0;
			for(i = 0; i < nread; i++)
			{
				printf("%x:", buffer[i]);
			}

			printf("\r\n");
			
			uart_data_save(buffer, nread);
		}
		
		usleep(20000);
	}
}       


/***************************************************************************
UART:initializtion
***************************************************************************/

static int32 set_baudrate(int fd, int baudrate)
{
	int   i; 
	int   status; 
	struct termios   Opt;
	int ret = 0;
	ret = tcgetattr(fd, &Opt); 
	if(ret < 0)
	{
        return DRV_ERR;
	}
	
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) 
	{ 
		if  (baudrate == baudrate_arr[i] )
		{     
			tcflush(fd, TCIOFLUSH);     
			cfsetispeed(&Opt, speed_arr[i]);  
			cfsetospeed(&Opt, speed_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if  (status != 0) 
			{        
				perror("tcsetattr fd err");  
				return DRV_ERR;     
			}    
			tcflush(fd,TCIOFLUSH);   
		} 
	}
	printf("set_baudrate to %d ok \r\n",baudrate);

	return DRV_OK;
}

static int32 set_Parity(int fd,int databits,int stopbits,int parity)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		return(DRV_ERR);  
	}

	printf("set_Parity c_cflag=0x%x, c_iflag=0x%x, c_lflag=0x%x\r\n",
			options.c_cflag, options.c_iflag, options.c_lflag);
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
		fprintf(stderr,"Unsupported data size\n"); 
		return (DRV_ERR);  
	}
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= (PARODD | PARENB); 
			options.c_iflag |= INPCK;             	/* Disnable parity checking */ 
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;    		 /* Enable parity */    
			options.c_cflag &= ~PARODD;   
			options.c_iflag |= INPCK;       	 /* Disnable parity checking */
			break;
		case 'S': 
		case 's':     
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;  
		default:   
			fprintf(stderr,"Unsupported parity\n");    
			return (DRV_ERR);  
	}  

	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
			break;
		default:    
			fprintf(stderr,"Unsupported stop bits\n");  
			return (DRV_ERR); 
	}

	if (parity != 'n')   
		options.c_iflag |= INPCK; 
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; 
	//options.c_cc[VTIME] = 255; 
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */

	//Add by lishibing
	options.c_lflag = 0; //接收反馈：如串口接收到数据，立即将该数据发送出去.

	options.c_oflag=0;
	options.c_iflag=0;

	if (tcsetattr(fd, TCSANOW, &options) != 0)   
		return (DRV_ERR); 

	
	if  ( tcgetattr( fd, &options)  !=  0) { 
		return(DRV_ERR);  
	}

	printf("222 c_cflag=0x%x, c_iflag=0x%x, c_lflag=0x%x\r\n",
			options.c_cflag, options.c_iflag, options.c_lflag);
			
	printf("set_parity ok \r\n");
	return (DRV_OK);  
}

static int32 Uart_mpc8313_Init(void)
{
	char *dev_name  = "/dev/ttyS1";
	int baudrate = 0;
	
	uart_fd = open(dev_name, O_RDWR|O_NOCTTY|O_NDELAY);
	if (uart_fd < 0)
	{ 			
		printf("%s open fail\r\n", __func__);
		return DRV_ERR;	
	}

	baudrate = 115200;
	
	set_baudrate(uart_fd, baudrate);
	set_Parity(uart_fd,8,1,'n');
	
	return DRV_OK;	
}


int32 Uart_init(void)
{
	memset(uart_data, 0, sizeof(uart_data));
	
	if (DRV_OK != Uart_mpc8313_Init())
	{
		return DRV_ERR;
	}
	
	return DRV_OK;
}
                                                                                                                                                          
int32 Uart_Close(void)
{
	if(uart_fd)
	{
		close (uart_fd);
	}
	
	return DRV_OK;	
}

