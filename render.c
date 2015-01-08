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
#include <GL/freeglut.h>
#include <math.h>
#include <fftw3.h>
#include "common.h"
#include "pipe_buffer.h"
#include "util.h"
#include "fft.h"
#include "render.h"

extern int camera_position_x;
extern int camera_position_y;
extern int camera_position_z;

extern double camera_rotate_x;
extern double camera_rotate_y;

static int fft_base_x_scale = 1;
static int fft_base_y_scale = 1;
static char fft_use_color = 1;
static char render_mode = 0;

static int drawn_sample_frames = 0;

static struct fft_window_buffer window_buffer;

static double scale_translate[2][AMOUNT_COLUMNS / 2];

void get_color(double value, float *r, float *g, float *b)
{
  if (fft_use_color) {
    get_heat_map_color(value, r, g, b);
  }
  else {
    *r = 1;
    *b = 1;
    *g = 1;
  }
}

static void render_wave()
{
  for (int column = 0; column < AMOUNT_COLUMNS / SAMPLES_PER_COLUMN; ++column) {
    double averaged_samples = 0;
    double render_x, render_y;

    for (int i = 0; i < SAMPLES_PER_COLUMN; ++i) {
      int16_t sample = get_pipe_buffer_sample(column * SAMPLES_PER_COLUMN + i); /* read sample */
      double scaled_sample = (((double) sample / INT16_MAX) + 1) / 2; /* scaled between 0 and 1 */

      double weighted_existing_sample_average = averaged_samples * ((double) i / (i + 1));
      double weighted_current_sample = scaled_sample / (i + 1);

      averaged_samples = weighted_existing_sample_average + weighted_current_sample;
    }

    render_x = (double) column / ((AMOUNT_COLUMNS / SAMPLES_PER_COLUMN) - 1); /* between 0 and 1 */

    render_y = averaged_samples;

    glBegin(GL_LINE_STRIP);
    {
      glVertex2f(render_x, render_y);
    }
  }

  glEnd();
}

void render_fft_scale()
{
  for (int marker_nr = 0; marker_nr < AMOUNT_COLUMNS / 2; marker_nr += 30) { /* TODO: AMOUNT_HORIZONTAL_SCALE_MARKS */
    const double marker_height = .99;
    double marker_x = scale_translate[fft_base_x_scale][marker_nr];
    char marker_text[16];

    double freq = (((double) SAMPLE_RATE / 2) / (AMOUNT_COLUMNS / 2)) * (double) marker_nr;

    sprintf(marker_text, "%d Hz", (int) freq);

    glBegin(GL_QUADS);
    {
      glVertex2f(marker_x, marker_height);
      glVertex2f(marker_x + .001, marker_height);
      glVertex2f(marker_x + .001, marker_height + .008);
      glVertex2f(marker_x, marker_height + .008);
    }
    glEnd();

    glRasterPos2f(marker_x + .003, marker_height);
    glutBitmapString(GLUT_BITMAP_HELVETICA_10, (const unsigned char*) marker_text);
  }
}

static void render_fft(fftw_complex *fft)
{
  int amount_of_buckets_to_render = AMOUNT_COLUMNS / 2;
  double render_x, next_render_x;

  for (int column = 0; column < amount_of_buckets_to_render - 1; ++column) {
    double fft_magnitude = normalize_fft_magnitude(fft[column][0], fft[column][1]);

    fft_magnitude = fft_magnitude * fft_base_y_scale;

    render_x = scale_translate[fft_base_x_scale][column];
    next_render_x = scale_translate[fft_base_x_scale][column + 1];

    glBegin(GL_QUADS);
    {
      glVertex2f(render_x, 0);
      glVertex2f(render_x, fft_magnitude);
      glVertex2f(next_render_x - .0005, fft_magnitude); /* - .0005: get some black space between columns TODO: fix aliasing */
      glVertex2f(next_render_x - .0005, 0);
    }
    glEnd();
  }

  render_fft_scale();
}

static void render_3d_fft(fftw_complex *fft)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  --window_buffer.newest_window;

  if (window_buffer.newest_window == -1)
    window_buffer.newest_window = AMOUNT_OF_WINDOWS - 1;

  for (int column = 0; column < AMOUNT_COLUMNS / 2; ++column) {
    double fft_magnitude = normalize_fft_magnitude(fft[column][0], fft[column][1]);

    window_buffer.fft_windows[window_buffer.newest_window][column].fft_magnitude = fft_magnitude;
  }
  glRotatef(camera_rotate_x, 1.0, 0.0, 0.0);
  glRotatef(camera_rotate_y, 0.0, 1.0, 0.0);
  glTranslatef(camera_position_x / 100.0, camera_position_y / 100.0, camera_position_z / 100.0);

  glTranslatef(-0.55f, -0.22f, -0.7f);
  glRotatef(-10, 0, 1, 0);

  double render_z = -1;

  for (int window_count = 0; window_count < AMOUNT_OF_WINDOWS; ++window_count) {
    int window_index = (window_buffer.newest_window + window_count) % AMOUNT_OF_WINDOWS;
    render_z += ((double) 1) / AMOUNT_OF_WINDOWS;

    for (int column = 0; column < AMOUNT_COLUMNS / 2; ++column) {
      double render_x, render_y;
      float r, g, b;

      /* grab from cache */
      if (window_index != window_buffer.newest_window) {
	struct fft_point_cache *p = &window_buffer.fft_windows[window_index][column];

	render_x = p->x;
	render_y = p->y;
	r = p->r;
	g = p->g;
	b = p->b;
      }

      /* calculate x, y and color for new window and save in cache */
      else {
	render_x = scale_translate[fft_base_x_scale][column];

	render_y = window_buffer.fft_windows[window_index][column].fft_magnitude;
	render_y = render_y * fft_base_y_scale;

	get_color(render_y, &r, &g, &b);

	/* save to cache */
	struct fft_point_cache *p = &window_buffer.fft_windows[window_index][column];
	p->x = render_x;
	p->y = render_y;
	p->r = r;
	p->g = g;
	p->b = b;
      }

      glColor3f(r, g, b);

      glBegin(GL_POINTS);
      glVertex3f(render_x, render_y, render_z);
      glEnd();
    }
  }

  glColor3f(1.0f, 1.0f, 1.0f);
  glLoadIdentity();
}

void setup_2d_projection()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  /* origin bottom left, 0 < x < 1, 0 < y < 1 */
  glTranslatef(-1.0f, -1.0f, 0.0f);
  glScalef(2.0f, 2.0f, 0.0f);

  glColor3f(1.0f, 1.0f, 1.0f);
}

void setup_3d_projection()
{
  int width = glutGet(GLUT_WINDOW_WIDTH);
  int height = glutGet(GLUT_WINDOW_HEIGHT);

  // Compute aspect ratio of the new window
  if (height == 0) height = 1;                // To prevent divide by 0
  GLfloat aspect = (GLfloat)width / (GLfloat)height;

  // Set the viewport to cover the new window
  glViewport(0, 0, width, height);

  // Set the aspect ratio of the clipping volume to match the viewport
  glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
  glLoadIdentity();             // Reset
  // Enable perspective projection with fovy, aspect, zNear and zFar
  gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

void setup_scale_arrays()
{
  int i;

  /* linear scale */
  for (i = 0; i < AMOUNT_COLUMNS / 2; ++i) {
    scale_translate[0][i] = (double) i / (AMOUNT_COLUMNS / 2);
  }

  /* log scale */
  for (i = 0; i < AMOUNT_COLUMNS / 2; ++i) {
    scale_translate[1][i] = (log(i) / log(2)) / (log(AMOUNT_COLUMNS / 2) / log(2));

    if (scale_translate[1][i] == -HUGE_VAL)
      scale_translate[1][i] = 0;
  }
}

void setup_appropriate_projection()
{
  render_mode == RENDER_3D_MODE ? setup_3d_projection() : setup_2d_projection();
}

/* todo make struct */
long last_interface_update_ms = 0;
char current_fps[8];
char x_scale[16];
char y_scale[16];

void render_process_information() {
  long current_time_in_ms = get_current_time_in_ms();
  long ms_since_last_interface_update = current_time_in_ms - last_interface_update_ms;

  setup_2d_projection();

  /* update interface when enough time has elapsed */
  if (ms_since_last_interface_update > INTERFACE_UPDATE_IN_MS) {
    double fps = drawn_sample_frames / ((double) ms_since_last_interface_update / 1000);

    sprintf(current_fps, "%ld %s", (long) fps, "fps");

    drawn_sample_frames = 0;
    last_interface_update_ms = current_time_in_ms;

    if (fft_base_x_scale == 0)
      sprintf(x_scale, "%s %s", "lin", "x");
    else
      sprintf(x_scale, "%s %s", "log", "x");

    sprintf(y_scale, "%s%d %s", "*", fft_base_y_scale, "y");
  }

  glRasterPos2f(.96, .97);
  glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*) current_fps);
  glRasterPos2f(.96, .95);
  glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*) x_scale);
  glRasterPos2f(.96, .93);
  glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*) y_scale);

  setup_appropriate_projection();
}

void render(fftw_complex *fft)
{
  glClear(GL_COLOR_BUFFER_BIT);

  if (render_mode == RENDER_WAVE_MODE)
    render_wave();
  else {
    if (render_mode == RENDER_FFT_MODE)
      render_fft(fft);
    else if (render_mode == RENDER_3D_MODE)
      render_3d_fft(fft);
  }

  ++drawn_sample_frames;
  render_process_information();
}

void increment_fft_base_x_scale() {
  fft_base_x_scale = (fft_base_x_scale + 1) % 2;
}

void decrement_fft_base_x_scale() {
  --fft_base_x_scale;

  if (fft_base_x_scale < 0)
    fft_base_x_scale = 1;
}

void increment_fft_base_y_scale() {
  ++fft_base_y_scale;
}

void decrement_fft_base_y_scale() {
  --fft_base_y_scale;
}

void toggle_use_color() {
  fft_use_color = !fft_use_color;
}

char get_render_mode() {
  return render_mode;
}

void set_render_mode(char mode) {
  render_mode = mode;
}
