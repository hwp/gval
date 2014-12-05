/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * matwr_test.cpp : OpenCV Mat write/read test.
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

extern "C" {
#include "gval_utils.h"
}

#include "gval_cv.hpp"

#include <opencv2/core/core.hpp>

#define TEMP_FILE "matwr_test_temp"
#define ROWS 102
#define COLS 159
#define ELEMENT(x, y) (((x) + 1.0) / ((x) + (y) + 2.0))
#define N_WRITES 100

using cv::Mat;

int main(int argc, char** argv) {
  gval_debug_init();

  for (int k = 0; k < 100000; k++) {
    Mat m(ROWS, COLS, CV_32FC1);
    for (int i = 0; i < ROWS; i++) {
      for (int j = 0; j < COLS; j++) {
        m.at<float>(i, j) = ELEMENT(i, j);
      }
    }

    FILE* out = fopen(TEMP_FILE, "w");
    assert(out);
    for (int i = 0; i < N_WRITES; i++) {
      gval_write_cvmat(&m, out);
    }
    fclose(out);

    FILE* in = fopen(TEMP_FILE, "r");
    assert(in);
    void* ret;
    int ctr = 0;
    while( (ret = gval_read_cvmat(in)) != NULL) {
      Mat n = *(Mat*) ret;
      ctr++;

      for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
          if (m.at<float>(i, j) != n.at<float>(i, j)) {
            printf("Inconsistent Read: m(%d, %d) = %g, n(%d, %d) = %g\n",
                i, j, m.at<float>(i, j), i, j, n.at<float>(i, j));
            abort();
          }
        }
      }
    }
    fclose(in);

    if (ctr != N_WRITES) {
      printf("Inconsistent Numbers: write %d, read %d\n",
          N_WRITES, ctr);
      abort();
    }
  }

  // memory leak

  return 0;
}

