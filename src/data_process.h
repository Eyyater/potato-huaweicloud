#ifndef DATA_PROCESS_H
#define DATA_PROCESS_H

void normalize_array(const float *input, float *output, int n);
float predict_data(const float *k, float b, const float *vis, const float *ir);

#endif