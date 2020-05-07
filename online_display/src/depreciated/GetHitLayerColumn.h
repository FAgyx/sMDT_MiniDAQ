/*******************************************************************************
  file name: GetHitLayerColumn.h
  author: Zhe Yang
  created: 02/19/2019
  last modified: 02/19/2019

  description:
  Get the hit's position in layer and column.
  Use some constants in GetHitInfo.h

*******************************************************************************/

#ifndef GETHITLAYERCOLUMN_H_
#define GETHITLAYERCOLUMN_H_

#include <iostream>

#include "src/depreciated/GetHitInfo.h"

void GetHitLayerColumn(unsigned int tdc_id, 
                       unsigned int channel_id,
                       int *hit_layer,
                       int *hit_column) {
  if (tdc_id == 8) {
    *hit_column = 30 +  hit_column_map[channel_id];
    *hit_layer = 0 + hit_layer_map[channel_id];
  }
  if (tdc_id == 9) {
    *hit_column = 30 +  hit_column_map[channel_id];
    *hit_layer = 4 + hit_layer_map[channel_id];
  }
  if (tdc_id == 10) {
    *hit_column = 24 +  hit_column_map[channel_id];
    *hit_layer = 4 + hit_layer_map[channel_id];
  }
  if (tdc_id == 11) {
    *hit_column = 24 +  hit_column_map[channel_id];
    *hit_layer = 0 + hit_layer_map[channel_id];
  }
}

#endif // GETHITLAYERCOLUMN_H_
