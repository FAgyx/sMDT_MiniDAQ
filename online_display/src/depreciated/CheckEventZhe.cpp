/*******************************************************************************
  file name: CheckEvent.cxx
  author: Zhe Yang
  created: 01/25/2019
  last modified: 02/05/2019

  description:
  Check whether the event meet the following requirement:
  -the coarse difference between each other is less than MAX_COARSE_DIFFERENCE 
   defined at the beginning of the code
  -the number of hits happened in current event is larger than MIN_HITS_NUMBER 
   defined at the beginning of the code
  Calculate the mean value of leading/trailing coarse time

*******************************************************************************/

#include <iostream>
#define MAX_COARSE_DIFFERENCE 8
#define MIN_HITS_NUMBER 4

const int PASTEVENTCHECK = true;
const int NOTPASTEVENTCHECK = false;

using namespace std;
bool CheckEvent(unsigned int signal_cache_length,
                unsigned int signal_cache[128][5],
                unsigned int *leading_mean_coarse,
                unsigned int *trailing_mean_coarse) {
  // define local variables
  unsigned int current_coarse = 0;
  unsigned int temp_coarse = 0;
  unsigned int total_coarse = 0;
  unsigned int max_coarse = 0;
  unsigned int min_coarse = 4096; // initialize with a large number in case the 
                                  // input coarse time is small
  int total_valid_hits = 0;
  bool consist_coarse_flag = true;
  bool enough_hits_flag = true;

  // check leading edge
  for (int signal_id = 0; signal_id < signal_cache_length; signal_id++) {
    if (signal_cache[signal_id][0] == 4) {
      temp_coarse = signal_cache[signal_id][3];
      if (temp_coarse > max_coarse) {
        max_coarse = temp_coarse;
      }
      if (temp_coarse < min_coarse) {
        min_coarse = temp_coarse;
      }
      total_coarse += current_coarse;
      total_valid_hits++;
    }
  }
  consist_coarse_flag = (max_coarse - min_coarse < 8)
                        && (max_coarse - min_coarse > 0);
  *leading_mean_coarse = (total_coarse * 1.0) / signal_cache_length;

  // check trailing edge
  current_coarse = 0;
  temp_coarse = 0;
  total_coarse = 0;
  max_coarse = 0;
  min_coarse = 4096;
  consist_coarse_flag = true;
  for (int signal_id = 0; signal_id < signal_cache_length; signal_id++) {
    if (signal_cache[signal_id][0] == 5) {
      temp_coarse = signal_cache[signal_id][3];
      if (temp_coarse > max_coarse) {
        max_coarse = temp_coarse;
      }
      if (temp_coarse < min_coarse) {
        min_coarse = temp_coarse;
      }
      total_coarse += current_coarse;
    }
  }
  *trailing_mean_coarse = (total_coarse * 1.0) / signal_cache_length;

  // check whether this event meets the requirement
  consist_coarse_flag = ((max_coarse - min_coarse < MAX_COARSE_DIFFERENCE)
                        && (max_coarse - min_coarse > 0)) 
                        || consist_coarse_flag;
  enough_hits_flag = total_valid_hits > MIN_HITS_NUMBER;

  if (consist_coarse_flag && enough_hits_flag) {
    return PASTEVENTCHECK;
  } else {
    return NOTPASTEVENTCHECK;
  }
}