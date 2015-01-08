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

#include <GL/freeglut.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <fftw3.h>
#include "common.h"
#include "frequency_wave.h"
#include "pipe_buffer.h"
#include "fft.h"
#include "render.h"
#include "util.h"

int camera_position_x = 0;
int camera_position_y = 0;
int camera_position_z = 0;

double camera_rotate_x = 0;
double camera_rotate_y = 0;

void update()
{
  read_named_pipe();

  if (samples_in_pipe_buffer(AMOUNT_COLUMNS)) {
    fftw_complex *fft_out = 0;

    if (get_render_mode() == RENDER_FFT_MODE || get_render_mode() == RENDER_3D_MODE)
      fft_out = calculate_fft();

    render(fft_out);
    flush_pipe_buffer();
    glutSwapBuffers();
  }
}

void timer(int value)
{
  UNUSED(value);

  glutPostRedisplay();
  glutTimerFunc(1000 / TARGET_FPS, timer, 0);
}

void keyboard(unsigned char key, int x, int y)
{
  UNUSED(x);
  UNUSED(y);

  switch (key) {
    case '1':
      set_render_mode(RENDER_WAVE_MODE);
      setup_appropriate_projection();
      break;
    case '2':
      set_render_mode(RENDER_FFT_MODE);
      setup_appropriate_projection();
      break;
    case '3':
      set_render_mode(RENDER_3D_MODE);
      setup_appropriate_projection();
      break;
    case 'q':
      glutLeaveMainLoop();
      break;
    case 'X':
      increment_fft_base_x_scale();
      break;
    case 'x':
      decrement_fft_base_x_scale();
      break;
    case 'Y':
      increment_fft_base_y_scale();
      break;
    case 'y':
      decrement_fft_base_y_scale();
      break;
    case 'C':
      toggle_use_color();
      break;
    case 'u':
      --camera_position_x;
      break;
    case 'o':
      ++camera_position_x;
      break;
    case 'p':
      --camera_position_y;
      break;
    case ',':
      ++camera_position_y;
      break;
    case '.':
      ++camera_position_z;
      break;
    case 'e':
      --camera_position_z;
      break;
    default:
      break;
  }
}

int last_x = -1;
int last_y = -1;

void mouse(int x, int y)
{
  if (last_x == -1 && last_y == -1) {
    last_x = x;
    last_y = y;
  }

  int diff_x = x - last_x;
  int diff_y = y - last_y;

  last_x = x;
  last_y = y;

  camera_rotate_x += (float) diff_y / 8;
  camera_rotate_y += (float) diff_x / 8;
}

void cleanup()
{
  printf("cleaning up...\n");
  cleanup_fft();
  cleanup_pipe_buffer();
}

void print_fps_info()
{
  printf("assuming samplerate of 44100 Hz\nmax framerate is %f\n", (double) SAMPLE_RATE / AMOUNT_COLUMNS);
}

int main(int argc, char **argv)
{
  print_fps_info();
  setup_scale_arrays();

  glutInit(&argc, argv);

  srand(time(NULL));

  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(1920, 1080);
  glutCreateWindow("mpd vis");
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  /* callbacks */
  glutDisplayFunc(update);
  glutKeyboardFunc(keyboard);
  glutPassiveMotionFunc(mouse);

  setup_appropriate_projection();

  setup_pipe_buffer();
  timer(0);

  glutMainLoop();

  cleanup();

  return 0;
}
