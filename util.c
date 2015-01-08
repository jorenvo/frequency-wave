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

#include <math.h>
#include <time.h>

long get_current_time_in_ms()
{
  long ms;
  time_t s;
  struct timespec spec;

  clock_gettime(CLOCK_REALTIME, &spec);

  s  = spec.tv_sec;
  ms = round(spec.tv_nsec / 1.0e6); /* convert nanoseconds to milliseconds */
  ms += s * 1000;

  return ms;
}

void hsv_to_rgb(float hue, float *red, float *green, float *blue)
{
  /* do conversion to rgb */
  /* find chroma (C = V * S) */
  float chroma = 1;

  hue /= (float) 60;

  float x = chroma * (1 - fabs(fmod(hue, 2) - 1));

  if (hue >= 0 && hue < 1) {
    *red = chroma;
    *green = x;
    *blue = 0;
  }
  else if (hue >= 1 && hue < 2) {
    *red = x;
    *green = chroma;
    *blue = 0;
  }
  else if (hue >= 2 && hue < 3) {
    *red = 0;
    *green = chroma;
    *blue = x;
  }
  else if (hue >= 3 && hue < 4) {
    *red = 0;
    *green = x;
    *blue = chroma;
  }
  else if (hue >= 4 && hue < 5) {
    *red = x;
    *green = 0;
    *blue = chroma;
  }
  else if (hue >= 5 && hue < 6) {
    *red = chroma;
    *green = 0;
    *blue = x;
  }
}

/* do color determination in hsv, cut of right red part of hue (blue starts at 240) */
void get_heat_map_color(float value, float *red, float *green, float *blue)
{
  /* value in degrees (360 - right red and purple = 240) */
  float hue = value * 240;

  /* compress 240 degrees of color to [0, .5] */
  hue *= 2;

  /* >= .5 will just be red */
  hue = fmin(hue, 240);

  /* turn thing around (because high hue values are cold colors) */
  hue = -(hue - 240);

  hsv_to_rgb(hue, red, green, blue);
}

/* lookup in rgb_color_map (fast get_heat_map_color()) */
void get_heat_map_color_lookup(float value, float *red, float *green, float *blue)
{
  /* determined by get_heat_map_color() and hsv_to_rgb() */
  int rgb_color_map[][3] = {
    {0, 0, 255},
    {0, 0, 255},
    {0, 255, 255},
    {0, 255, 0},
    {0, 255, 0},
    {0, 255, 0},
    {255, 255, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0},
    {255, 0, 0}
  };

  int *color = rgb_color_map[(int) (value * 16)];

  *red = color[0];
  *green = color[1];
  *blue = color[2];
}
