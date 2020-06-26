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

*******************************************************************************/

#include <iostream>

#include "src/Event.cpp"
#include "src/Geometry.cpp"
#include "src/Cluster.cpp"

#define MAX_TIME_DIFFERENCE 10.0*25.0 // represents 8 coarse time steps or 200 ns
#define MIN_HITS_NUMBER 6
#define MAX_HITS_NUMBER 12
const int PASTEVENTCHECK = true;
const int NOTPASTEVENTCHECK = false;

using namespace std;
using namespace Muon;

bool CheckEvent(Event e, Geometry geo) {

  // need precisely one trigger
  if (e.TriggerHits().size() != 1) 
    return NOTPASTEVENTCHECK;

  // need at least 4 wire hits
  if (e.WireHits().size() < MIN_HITS_NUMBER)
    return NOTPASTEVENTCHECK;

  if (e.WireHits().size() > MAX_HITS_NUMBER) 
    return NOTPASTEVENTCHECK;
  
  // need precisely one cluster in each multilayer
  int nML0 = 0;
  int nML1 = 0;
  for (Cluster c : e.Clusters()) {
    if (geo.MultiLayer(c) == 0)
      nML0++;
    if (geo.MultiLayer(c) == 1) 
      nML1++;
  }

  if (nML1 !=1 || nML0 != 1)
    return NOTPASTEVENTCHECK;
  

  // need the maximum time difference to be 200 ns
  double max_time = 0;
  double min_time = 2e100;

  for (int i = 0; i < e.WireHits().size(); i++) {
    if (e.WireHits().at(i).TDCTime() > max_time) {
      max_time = e.WireHits().at(i).TDCTime();
    }
    if (e.WireHits().at(i).TDCTime() < min_time) {
      min_time = e.WireHits().at(i).TDCTime();
    }
  }
  if ((max_time - min_time < MAX_TIME_DIFFERENCE) && (max_time - min_time > 0))
    return PASTEVENTCHECK;
  else
    return NOTPASTEVENTCHECK;
}
