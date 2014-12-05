/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_cv.cpp : CV function using OpenCV (C++)
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

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include <stdlib.h>
#include <assert.h>

using namespace cv;

void gval_draw_keypoints(void* img, int rows, int cols) {
  Mat image(rows, cols, CV_8UC3, img);

  SiftFeatureDetector detector;
  std::vector<KeyPoint> points;
  detector.detect(image, points);

  drawKeypoints(image, points, image, Scalar::all(-1),
      DrawMatchesFlags::DEFAULT);
}

void gval_extract_descriptor(void* img, int rows,
    int cols, void** result, int* n_points, int* dim) {
  Mat image(rows, cols, CV_8UC3, img);

  SiftFeatureDetector detector;
  std::vector<KeyPoint> points;
  detector.detect(image, points);
  
  SiftDescriptorExtractor extractor;
  Mat* descriptor = new Mat();
  extractor.compute(image, points, *descriptor);

  *n_points = descriptor->rows;
  *dim = descriptor->cols;
  assert(*n_points >= 0 && *dim >= 0);
  *result = descriptor;
}

void gval_write_cvmat(const void* matrix, FILE* stream) {
  Mat m = *(Mat*) matrix;
  assert(m.dims == 2);
  if (!m.isContinuous()) {
    m = m.clone();
    assert(m.isContinuous());
  }

  int type = m.type();
  int rows = m.rows;
  int cols = m.cols;
  assert(m.total() == rows * cols);
  int elem_size = m.elemSize();
  void* data = m.data;

  size_t ret;
  ret = fwrite(&type, sizeof(type), 1, stream);
  assert(ret == 1);
  ret = fwrite(&rows, sizeof(rows), 1, stream);
  assert(ret == 1);
  ret = fwrite(&cols, sizeof(cols), 1, stream);
  assert(ret == 1);
  ret = fwrite(&elem_size, sizeof(elem_size), 1, stream);
  assert(ret == 1);
  ret = fwrite(data, elem_size, rows * cols, stream);
  assert(ret == rows * cols);
}

void* gval_read_cvmat(FILE* stream) {
  int type, rows, cols, elem_size;
  void* data;

  size_t ret;
  ret = fread(&type, sizeof(type), 1, stream);
  if (ret == 0 && feof(stream)) {
    return NULL;
  }
  assert(ret == 1);
  ret = fread(&rows, sizeof(rows), 1, stream);
  assert(ret == 1);
  ret = fread(&cols, sizeof(cols), 1, stream);
  assert(ret == 1);
  ret = fread(&elem_size, sizeof(elem_size), 1, stream);
  assert(ret == 1);
  data = malloc(elem_size * rows * cols);
  ret = fread(data, elem_size, rows * cols, stream);
  assert(ret == rows * cols);

  Mat* m = new Mat(rows, cols, type, data);
  return m;
}

void gval_free_cvmat(void* matrix) {
  delete (Mat*) matrix;
}

