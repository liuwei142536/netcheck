#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
 
#include "netchecklib.h"
 
 
int main()
{
	int netlinkstatus=0;
 
	while(1)
	{
		netlinkstatus=netLinkCheck("ens33");
		printf("netlinkstatus is %d\n",netlinkstatus);
		sleep(1);
	}
 
	return 0;
}
