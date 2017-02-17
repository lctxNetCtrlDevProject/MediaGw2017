#include "public.h"
#include "osa.h"


void dispBuf(unsigned char *buf, int len, const char *name){
	int i = 0;
	if(!buf || len <= 0)
		return;
	if(name)
		snmp_log(LOG_WARNING,"]\r\n%s[",name);
	else
		snmp_log(LOG_WARNING,"\r\n[");
	for(i =0; i<len; i++){
		snmp_log(LOG_WARNING,"  %02X",buf[i]);
	}
	snmp_log(LOG_WARNING,"]\r\n");
}

/*transform bcd num to str num*/
int bcd_to_string(char *bcd, char *strDst, int size)
{
	int i = 0;
	char num = 0;
	//dispBuf(bcd,size,__func__);
	for (i = 0; i < size; ++i) {
		num = bcd[i] & 0xf;
		if (0xf == num) {
			strDst[i*2] = 0;
			return 0;
		} else {
			if (num < 10) {
				strDst[i*2] = 48 + num;
			} else {
				//inc_log(LOG_WARNING, "bcd_to_string:1 i=%u,num=%u\n", i, num);
				return -1;
			}
		}
		num = (bcd[i] & 0xf0) >> 4;
		if (0xf == num) {
			strDst[i*2 + 1] = 0;
			return 0;
		} else {
			if (num < 10) {
				strDst[i*2 + 1] = 48 + num;
			}  else {
				//inc_log(LOG_WARNING, "bcd_to_string:2 i=%u,num=%u\n", i, num);
				return -1;
			}
		}
	}
	return -1;
}

int string_to_bcd(char *str, char* bcdDst, int size)
{
	int i = 0;
	char num = 0;
	for (i = 0; i < size; i++) {
		num = str[i];
		if(i%2 == 0 ){
			/*proc  low half byte*/
			if ((num <= '9') && (num >= '0')) {
				bcdDst[i/2] = num - '0';
			} else {
				printf("Invalid num=%02x \r\n",num);
				bcdDst[i/2] |= 0x0F;
				return -1;
			
			}
			
		}
		else{
			/*proc high half byte*/
			if ((num <= '9') && (num >= '0')) {
				bcdDst[i/2] |= (num - '0') << 4;
			} else {
				//printf("Invalid num=%02x \r\n",num);
				bcdDst[i/2] |= 0xF0;
				return -1;
			}
		}
		
	}
	//printf("i=%d \r\n",i);
	if(i%2 == 0)
		bcdDst[i/2] |= 0x0F;
	else
		bcdDst[i/2] |= 0xF0;
	//printf("\r\n");
	return -1;	
}

/*stake funcs, now,just copy bcdStr to Str*/

void getStrNumByBcdNum(char *strNum, unsigned char *bcdNum, int bcdLen){
	if(!strNum || !bcdNum){
		OSA_ERROR("Invalid Para");
		return;
	}


	memcpy(strNum,bcdNum,bcdLen);

}

