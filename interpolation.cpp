#include <Arduino.h>

uint16_t get_point(uint16_t *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
inline void set_point(uint16_t *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, uint16_t f);
inline void get_adjacents_2d(uint16_t *src, uint16_t *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
inline uint16_t cubicInterpolate(uint16_t p[], float x);
inline uint16_t bicubicInterpolate(uint16_t p[], float x, float y);
void interpolate_image(uint16_t *src, uint8_t src_rows, uint8_t src_cols, 
                       uint16_t *dest, uint8_t dest_rows, uint8_t dest_cols);


uint16_t get_point(uint16_t *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  if (x < 0)        x = 0;
  if (y < 0)        y = 0;
  if (x >= cols)    x = cols - 1;
  if (y >= rows)    y = rows - 1;
  return p[y * cols + x];
}

inline void set_point(uint16_t *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, uint16_t f) {
  if ((x < 0) || (x >= cols)) return;
  if ((y < 0) || (y >= rows)) return;
  p[y * cols + x] = f;
}

// src is a grid src_rows * src_cols
// dest is a pre-allocated grid, dest_rows*dest_cols
void interpolate_image(uint16_t *src, uint8_t src_rows, uint8_t src_cols, 
                       uint16_t *dest, uint8_t dest_rows, uint8_t dest_cols) {
  float mu_x = (src_cols - 1.0) / (dest_cols - 1.0);
  float mu_y = (src_rows - 1.0) / (dest_rows - 1.0);

  uint16_t adj_2d[16]; // matrix for storing adjacents
  
  for (uint8_t y_idx=0; y_idx < dest_rows; y_idx++) {
    for (uint8_t x_idx=0; x_idx < dest_cols; x_idx++) {
       float x = x_idx * mu_x;
       float y = y_idx * mu_y;
       get_adjacents_2d(src, adj_2d, src_rows, src_cols, x, y);

       float frac_x = x - (int)x; // we only need the ~delta~ between the points
       float frac_y = y - (int)y; // we only need the ~delta~ between the points
       float out = bicubicInterpolate(adj_2d, frac_x, frac_y);
       set_point(dest, dest_rows, dest_cols, x_idx, y_idx, out);
    }
  }
}

// p is a list of 4 points, 2 to the left, 2 to the right
inline uint16_t cubicInterpolate(uint16_t p[], float x) {
    uint16_t r = p[1] + (0.5 * x * (p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0]))));
    return r;
}

// p is a 16-point 4x4 array of the 2 rows & columns left/right/above/below
inline uint16_t bicubicInterpolate(uint16_t p[], float x, float y) {
    uint16_t arr[4] = {0,0,0,0};
    arr[0] = cubicInterpolate(p+0, x);
    arr[1] = cubicInterpolate(p+4, x);
    arr[2] = cubicInterpolate(p+8, x);
    arr[3] = cubicInterpolate(p+12, x);
    return cubicInterpolate(arr, y);
}

// src is rows*cols and dest is a 16-point array passed in already allocated!
inline void get_adjacents_2d(uint16_t *src, uint16_t *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
    for (int8_t delta_y = -1; delta_y < 3; delta_y++) { // -1, 0, 1, 2
        uint16_t *row = dest + 4 * (delta_y+1); // index into each chunk of 4
        for (int8_t delta_x = -1; delta_x < 3; delta_x++) { // -1, 0, 1, 2
            row[delta_x+1] = get_point(src, rows, cols, x+delta_x, y+delta_y);
        }
    }
}
