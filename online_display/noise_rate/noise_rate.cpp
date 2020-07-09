/* time example */
#include <stdio.h>      /* printf */
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <unistd.h>

int noise_rate()
{
  time_t start_time, current_time;


  time(&start_time);  /* get current time; same as: timer = time(NULL)  */
  sleep(5);
  time(&current_time);
  double seconds;

  seconds = difftime(current_time,start_time);

  printf ("time diff = %.f \n", seconds);

  return 0;
}