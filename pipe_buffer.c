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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include "common.h"
#include "pipe_buffer.h"

static struct pipe_buffer buffer;

void setup_pipe_buffer()
{
  buffer.fd = open("/tmp/mpd.fifo", O_RDONLY | O_NONBLOCK);
}

void cleanup_pipe_buffer()
{
  close(buffer.fd);
}

void read_named_pipe()
{
  int read_return;

  /* read from pipe */
  read_return = read(buffer.fd, buffer.sample + buffer.read_samples, sizeof(buffer.sample) - (2 * buffer.read_samples));

  /* remember amount of 16 bit samples read */
  if (read_return > -1)
    buffer.read_samples += read_return / 2;
}

int16_t get_pipe_buffer_sample(int index)
{
  return buffer.sample[index];
}

int samples_in_pipe_buffer(int min_amount)
{
  return buffer.read_samples >= min_amount;
}

void flush_pipe_buffer()
{
  if (buffer.age >= 16) {
    char pipe_data[PIPE_SIZE_IN_BYTES];

    while (read(buffer.fd, pipe_data, PIPE_SIZE_IN_BYTES) > 0)
      ;

    buffer.age = 0;
  }

  buffer.read_samples = 0;

  ++buffer.age;
}
