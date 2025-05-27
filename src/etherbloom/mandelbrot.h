#ifndef MANDELBROT_H
#define MANDELBROT_H

#include "Complex.h"

namespace mandelbrot 
{
    // helper
    float c_abs(Complex number);
    size_t check_if_part_of_mandelbrot(Complex point, size_t max_iter, size_t cutoff);
    void draw_mandelbrot_set(float min_x, float max_x, float min_y, float max_y, size_t max_iter);
    void main();
}

#endif