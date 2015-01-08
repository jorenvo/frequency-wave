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

#ifndef PIPE_H
#define PIPE_H

/* from man 7 pipe: "Since Linux 2.6.11, the pipe capacity is 65536 bytes." */
#define PIPE_SIZE_IN_BYTES 65536

void setup_pipe_buffer();
void cleanup_pipe_buffer();
void read_named_pipe();
int16_t get_pipe_buffer_sample(int index);
int samples_in_pipe_buffer(int min_amount);
void flush_pipe_buffer();

struct pipe_buffer {
  int16_t sample[AMOUNT_COLUMNS];
  int read_samples;
  int fd;
  int age; /* the amount of frames that have been rendered since the
	      mpd pipe has been flushed */
};

#endif
