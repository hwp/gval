/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_merge.c : merge multiple features
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

#define MAX_FEATURES 5

#define DATA_TYPE double

int main(int argc, char** argv) {
  int i;
  int nfeat = 0;
  int bufsize = 0;
  char* infiles[MAX_FEATURES];
  FILE* ins[MAX_FEATURES];
  int dims[MAX_FEATURES];
  char* outfile;
  FILE* out;

  // get options
  int opt;
  int showhelp = 0;
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
      case 'h':
        showhelp = 1;
        break;
      default:
        showhelp = 1;
        break;
    }
  }

  if ((argc - optind) % 2 == 1) {
    nfeat = (argc - optind) / 2;
    for (i = 0; i < nfeat; i++) {
      infiles[i] = argv[optind + i * 2];
      dims[i] = atoi(argv[optind + i * 2 + 1]);
      if (dims[i] <= 0) {
        showhelp = 1;
        break;
      }
      bufsize += dims[i];
    }
    outfile = argv[argc - 1];
  }
  else {
    showhelp = 1;
  }

  if (showhelp) {
    fprintf(stderr, "Usage: %s file1 dim1 file2 dim2 [... up to 5] output\n", argv[0]);
    return EXIT_SUCCESS;
  }

  // no need to check lengths of each file,
  // cause we will deal with pipe as well.
  for (i = 0; i < nfeat; i++) {
    ins[i] = fopen(infiles[i], "r");
    assert(ins[i]);
  }
  out = fopen(outfile, "w");
  assert(out);

  char* buffer = malloc(bufsize * sizeof(DATA_TYPE));
  int complete = 1;
  while (complete) {
    size_t size = 0;
    int offset = 0;
    for (i = 0; i < nfeat; i++) {
      size = fread(buffer + (offset * sizeof(DATA_TYPE)),
          sizeof(DATA_TYPE), dims[i], ins[i]);
      offset += size;
      if (size < dims[i]) {
        complete = 0;
        break;
      }
    }

    if (complete) {
      assert(bufsize == offset);
      fwrite(buffer, sizeof(DATA_TYPE), bufsize, out);
    }
  }

  for (i = 0; i < nfeat; i++) {
    fclose(ins[i]);
  }
  fclose(out);
  free(buffer);
  return EXIT_SUCCESS;
}

