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

#include <stdio.h>
#include <fftw3.h>
#include <stdint.h>
#include <math.h>
#include "common.h"
#include "pipe_buffer.h"
#include "fft.h"

static fftw_plan fft_p;
static fftw_complex *fft_in = 0;
static fftw_complex *fft_out = 0;

void init_fft()
{
  printf("initializing fft...\n");

  fft_in = fftw_alloc_complex(AMOUNT_COLUMNS);
  fft_out = fftw_alloc_complex(AMOUNT_COLUMNS);

  /* try to read already existing wisdom from file */
  fftw_import_wisdom_from_filename(FFTW_WISDOM_FILENAME);

  fft_p = fftw_plan_dft_1d(AMOUNT_COLUMNS, fft_in, fft_out, FFTW_FORWARD, FFTW_PATIENT);

  fftw_export_wisdom_to_filename(FFTW_WISDOM_FILENAME);

  printf("initialized fft (cost %d)\n", (int) fftw_cost(fft_p));
}

void cleanup_fft()
{
  fftw_destroy_plan(fft_p);
  fftw_cleanup();
}

fftw_complex *calculate_fft()
{
  if (!fft_in)
    init_fft();

  /* copy array from buffer */
  for (int sample_index = 0; sample_index < AMOUNT_COLUMNS; ++sample_index) {
    fft_in[sample_index][0] = get_pipe_buffer_sample(sample_index); /* real */
    fft_in[sample_index][1] = 0; /* complex */
  }

  fftw_execute(fft_p);

  return fft_out;
}

double normalize_fft_magnitude(double amplitude, double phase)
{
  double normalized_fft_value = sqrt(pow(amplitude, 2) + pow(phase, 2));

  /* normalize according to http://www.fftw.org/fftw2_doc/fftw_2.html */
  /* division by two because we have amount_of_buckets_to_render frequency buckets */
  normalized_fft_value /= AMOUNT_COLUMNS / 2;
  /* normalize resulting 16 bit values */
  normalized_fft_value /= INT16_MAX;

  return normalized_fft_value;
}
