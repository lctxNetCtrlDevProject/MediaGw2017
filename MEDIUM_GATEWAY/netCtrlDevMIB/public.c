#include "public.h"
#include "debug.h"

/*transform bcd num to str num*/
/*stake funcs, now,just copy bcdStr to Str*/
void getStrNumByBcdNum(char *strNum, unsigned char *bcdNum, int bcdLen){
	if(!strNum || !bcdNum){
		ERROR("Invalid Para");
		return;
	}


	memcpy(strNum,bcdNum,bcdLen);

}

