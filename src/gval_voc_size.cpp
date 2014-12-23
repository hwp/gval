/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_voc_size.cpp : Print the size of vocabulary.
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

#include "gval_cv.hpp"

#include <stdio.h>

#include <opencv2/core/core.hpp>

using cv::Mat;

int main(int argc, char** argv) {
  FILE* in = fopen(argv[1], "r");
  assert(in);
  void* ret;
  ret = gval_read_cvmat(in);
  fclose(in);

  Mat n = *(Mat*) ret;
  printf("%d\n", n.rows);

  gval_free_cvmat(ret);

  return 0;
}

