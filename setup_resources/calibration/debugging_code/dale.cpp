#include <stdio.h>

#include <windows.h>
#include "flock_of_birds\bird.h"

#pragma pack(1)

int main()
{
printf("Hello world.\n");

#if 0
WORD addr[] = {8};
int rval = birdRS232WakeUp(0,1,1,addr,115200,2000,2000);
printf("rval %d\n", rval);
printf("sizeof(addr) %d\n", sizeof(addr));
#else
WORD addr[] = {0,5,6,7,8};
//WORD addr[] = {0,6,8,7,5};
int rval = birdRS232WakeUp(100,0,4,addr,115200,2000,2000,GMS_GROUP_MODE_NEVER);

BIRDSYSTEMCONFIG sysconfig;
printf("sizeof(sysocnfig) %d\n", sizeof(sysconfig));
rval = birdGetSystemConfig(100,&sysconfig);

sysconfig.dMeasurementRate = 130;
rval = birdSetSystemConfig(100,&sysconfig);
rval = birdGetSystemConfig(100,&sysconfig);

printf("rval %d\n", rval);
printf("sizeof(addr) %d\n", sizeof(addr));
printf("SampRate %f\n",sysconfig.dMeasurementRate);

#endif

//printf("rval %d\n", rval);
//printf("sizeof(addr) %d\n", sizeof(addr));


return 0;
}
