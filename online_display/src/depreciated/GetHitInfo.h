/*******************************************************************************
  file name: GetHitInfo.h
  author: Zhe Yang
  created: 02/19/2019
  last modified: 02/19/2019

  description:
  -Calculate the tube position in real world from tdc id and channel id
*******************************************************************************/

#ifndef GETHITINFO_H_
#define GETHITINFO_H_

const Double_t layer_distance = 13.0769836;
const Double_t column_distance = 15.1;
const Double_t radius = 7.5;

const int hit_column_map[24] = {5, 5, 5, 5,
                                4, 4, 4, 4,
                                3, 3, 3, 3,
                                2, 2, 2, 2,
                                1, 1, 1, 1,
                                0, 0, 0, 0};
const int hit_layer_map[24] = {0, 1, 2, 3,
                               0, 1, 2, 3,
                               0, 1, 2, 3,
                               0, 1, 2, 3,
                               0, 1, 2, 3,
                               0, 1, 2, 3};

void GetHitInfo(unsigned int tdc_id, 
                unsigned int channel_id,
                double *hit_x,
                double *hit_y) {
  unsigned int hit_column;
  unsigned int hit_layer;
  if (tdc_id == 8) {
    hit_column = 30 +  hit_column_map[channel_id];
    hit_layer = 0 + hit_layer_map[channel_id];
  }
  if (tdc_id == 9) {
    hit_column = 30 +  hit_column_map[channel_id];
    hit_layer = 4 + hit_layer_map[channel_id];
  }
  if (tdc_id == 10) {
    hit_column = 24 +  hit_column_map[channel_id];
    hit_layer = 4 + hit_layer_map[channel_id];
  }
  if (tdc_id == 11) {
    hit_column = 24 +  hit_column_map[channel_id];
    hit_layer = 0 + hit_layer_map[channel_id];
  }

  if (hit_layer < 4) {
      *hit_x = 7.5 + hit_column * column_distance + 
              ((hit_layer + 1) % 2) * column_distance / 2.0;
      *hit_y = 7.5 + hit_layer * layer_distance;
  }
  if (hit_layer >= 4 && hit_layer < 8) {
    *hit_x = 7.5 + hit_column * column_distance + 
            ((hit_layer + 1) % 2) * column_distance / 2.0;
    *hit_y = 7.5 + (hit_layer - 4) * layer_distance + 224.255; // in model it's 224.231
  }
}

#endif // GETHITINFO_H_
