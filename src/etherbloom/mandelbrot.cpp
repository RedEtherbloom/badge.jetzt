#include "Complex.h"
#include "Arduino.h"
#include <Adafruit_SSD1306.h>

#include "../base/button.h"
#include "../base/display.h"
#include "../base/led.h"
#include "mandelbrot.h"

namespace mandelbrot {
  // Max iterations of mandelbrot set to check
  const size_t DEFAULT_MAX_ITER = 20;
  // Number of milliseconds to keep coord mode active
  const size_t COORD_MODE_MIN_ACTIVE = 200;
  // Will evaluate to 3.
  const float DIVERGENCE_CUTOFF = 2;
  const float MANDELBROT_MAX_X = 2;
  const float MANDELBROT_MIN_X = -4;
  const float MANDELBROT_MAX_Y = 1;
  const float MANDELBROT_MIN_Y = -1;
  const size_t MANDELBROT_ASPECT_RATIO = (MANDELBROT_MAX_X - MANDELBROT_MIN_X) / (MANDELBROT_MAX_Y - MANDELBROT_MIN_Y);

  const size_t DISPLAY_WIDTH = 128;
  const size_t DISPLAY_HEIGHT = 32;
  const float MOVE_FACTOR_Y = 1/8.0;
  const float MOVE_FACTOR_X = MOVE_FACTOR_Y;
  const float ZOOM_INCREASE_FACTOR = 1.1;

  float zoom = 1.0;
  Complex middle(0, 0);
  size_t iter = DEFAULT_MAX_ITER;
  bool printCoordMode = false;
  bool drawTimeLed = false;
  bool needRedraw = true;
  unsigned long coordModeActive = 0;

  float c_abs(Complex number) {
    float real = number.real();
    float imaginary = number.imag();

    return sqrt(real * real + imaginary * imaginary);
  }

  float calc_range(float min, float max) {
    return abs(max - min);
  }

  size_t check_if_part_of_mandelbrot(Complex point, size_t max_iter, size_t cutoff) {
    // First iteration
    Complex number = point;

    for(size_t i = 0; i < max_iter; i++) {
      if (c_abs(number) > 2) {
        return i;
      }
      // Take square
      number = number.c_sqr() + point;
    }

    return max_iter;
  }

  void draw_mandelbrot_set(float min_x, float max_x, float min_y, float max_y, size_t max_iter) {
    Adafruit_SSD1306& oledDisplay = display::get();

    if (drawTimeLed) {
      led::set_l(0, 5, 5);
    }
    Serial.println("min_x: " + String(min_x) + " max_x: " + String(max_x) + "min_y: " + String(min_y) + " max_y: " + String(max_y) + " max_iter: " + String(max_iter));

    float x_range = calc_range(min_x, max_x);
    float y_range = calc_range(min_y, max_y);

    for (float x = 0; x < DISPLAY_WIDTH; x++) {
      for (float y = 0; y < DISPLAY_HEIGHT; y++) {
        float real = (x/DISPLAY_WIDTH) * x_range + min_x;
        float imaginary = (y/DISPLAY_HEIGHT) * y_range + min_y;
        Complex point(real, imaginary);

        size_t part_of_mandelbrot = check_if_part_of_mandelbrot(point, max_iter, DIVERGENCE_CUTOFF);
        uint16_t color = 0;
        if (part_of_mandelbrot == max_iter) {
          color = SSD1306_BLACK;
        } else {
          color = SSD1306_WHITE;
        }

        // TODO: Check if this display has gray support
        oledDisplay.writePixel(x, y, color);
      }
    }

    if (drawTimeLed) {
      led::set_l(0, 0, 0);
    }
  }

  void main() {
    bool modifier_pressed = button::down(BTN3);

    if (!modifier_pressed) {
      if (button::down(BTN1)) {
        zoom *= ZOOM_INCREASE_FACTOR;
        needRedraw = true;
      } else if (button::down(BTN2)) {
        zoom /= ZOOM_INCREASE_FACTOR;
        if (zoom <= 0) {
          zoom = 0.01;
        }
        needRedraw = true;
      }
    } else {
      if (button::down(BTN1)) {
        iter += 1;
        needRedraw = true;
      } else if (button::down(BTN2)) {
        iter -= 1;
        if (iter <= 0) {
          iter = 1;
        }
        needRedraw = true;
      }
    }

    float x_range = calc_range(MANDELBROT_MIN_X, MANDELBROT_MAX_X) / zoom;
    float y_range = calc_range(MANDELBROT_MIN_Y, MANDELBROT_MAX_Y) / zoom;

    if (!modifier_pressed) {
      if (button::down(BTN4)) {
        middle.setImag(middle.imag() - MOVE_FACTOR_Y * y_range);
        needRedraw = true;
      } else if (button::down(BTN5)) {
        middle.setImag(middle.imag() + MOVE_FACTOR_Y * y_range);
        needRedraw = true;
      }
    } else {
      if (button::down(BTN4)) {
        middle.setReal(middle.real() - MOVE_FACTOR_X * x_range);
        needRedraw = true;
      } else if (button::down(BTN5)) {
        middle.setReal(middle.real() + MOVE_FACTOR_X * x_range);
        needRedraw = true;
      }
    }

    if (!modifier_pressed) {
      if (button::down(BTN6)) { 
        if (printCoordMode) {
          // Mask disabling the coord mode for a minimum time to avoid accidental untriggering by e.g. too long held buttons
          if ((millis() - coordModeActive) >= COORD_MODE_MIN_ACTIVE) {
            printCoordMode = false;
            coordModeActive = 0;
            needRedraw = true;
          }
        } else {
          printCoordMode = true;
          coordModeActive = millis();
          needRedraw = false;
        }
      }
    } else {
      if (button::down(BTN6)) {
        drawTimeLed = !drawTimeLed;
        needRedraw = true;
      }
    }
    if (printCoordMode) {
      display::clearDisplay();
      display::println("Z: " + String(zoom) + " I: " + String(iter));
      display::println("X: " + String(middle.real()) + " Y: " + String(middle.imag()));
      display::println("LED Render: " + String(drawTimeLed));
      display::display();
    } else if (needRedraw) {
      display::clearDisplay();
      draw_mandelbrot_set(middle.real() - x_range/2, middle.real() + x_range/2, middle.imag() - y_range/2, middle.imag() + y_range/2, iter); 
      needRedraw = false;
      display::display();
    }

    delay(50);
  }
}