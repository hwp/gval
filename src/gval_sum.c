/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_sum.c : sum up feature vectors in a sequence
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General
 * Public License along with this program; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char** argv) {
  int i;
  int dim = 0;

  // get options
  int opt;
  int showhelp = 0;
  while ((opt = getopt(argc, argv, "d:h")) != -1) {
    switch (opt) {
      case 'd':
        dim = atoi(optarg);
        break;
      case 'h':
        showhelp = 1;
        break;
      default:
        showhelp = 1;
        break;
    }
  }

  if (showhelp || dim <= 0 || argc - optind < 2) {
    fprintf(stderr, "Usage: %s -d [dims] input output\n", argv[0]);
    return EXIT_SUCCESS;
  }

  FILE* in = fopen(argv[optind], "r");
  assert (in);

  unsigned int c = 0;
  double* sum = malloc(dim * sizeof(double));
  for (i = 0; i < dim; i++) {
    sum[i] = 0.0;
  }

  double* buffer = malloc(dim * sizeof(double));
  size_t r;
  while ((r = fread(buffer, sizeof(double), dim, in)) == dim) {
    for (i = 0; i < dim; i++) {
      sum[i] += buffer[i];
    }
    c++;
  }
  fclose(in);

  for (i = 0; i < dim; i++) {
    sum[i] /= (double) c;
  }

  FILE* out = fopen(argv[optind + 1], "w");
  assert(out);
  fwrite(sum, sizeof(double), dim, out);
  fclose(out);

  free(buffer);
  free(sum);

  return EXIT_SUCCESS;
}

