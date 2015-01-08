/* Copyright (C) 2014, 2015 Joren Van Onder */

/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program; if not, write to the Free Software Foundation, */
/* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef RENDER_H
#define RENDER_H

#define SAMPLES_PER_COLUMN 1
#define AMOUNT_HORIZONTAL_SCALE_MARKS 10

#define RENDER_WAVE_MODE 0
#define RENDER_FFT_MODE 1
#define RENDER_3D_MODE 2
#define AMOUNT_OF_RENDER_MODES 3

#define INTERFACE_UPDATE_IN_MS 200

void setup_scale_arrays();
void setup_appropriate_projection();
void render_process_information();
void increment_fft_base_x_scale();
void decrement_fft_base_x_scale();
void increment_fft_base_y_scale();
void decrement_fft_base_y_scale();
void toggle_rendering_half();
void toggle_use_color();
char get_render_mode();
void set_render_mode(char mode);
void render(fftw_complex *fft);

#define AMOUNT_OF_WINDOWS 100

struct fft_point_cache {
  double fft_magnitude;
  double x, y;
  float r, g, b;
};

struct fft_window_buffer {
  struct fft_point_cache fft_windows[AMOUNT_OF_WINDOWS][AMOUNT_COLUMNS / 2];
  int newest_window;
};

#endif
