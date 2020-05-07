#include <stdio.h> 
#include <time.h>


int main(int argc, char const *argv[]) {
	time_t sys_time;
	struct tm * timeinfo;
	char *timestr;
	sys_time = time(0);
	timeinfo = localtime(&sys_time);
	sprintf(timestr,"%04d%02d%02d_%02d%02d%02d.dat",timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	
	
	printf("Now = %s", ctime(&sys_time));
	
	printf("%s\n",timestr);


}