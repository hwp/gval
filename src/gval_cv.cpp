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

/**
 * single channel matrix
 */
void gval_mat2bytes(Mat& matrix, void** result,
    int* rows, int* cols);

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
  
  if (points.size() > 0) {
    SiftDescriptorExtractor extractor;
    Mat descriptor;
    extractor.compute(image, points, descriptor);

    assert(descriptor.channels() == 1 
        && descriptor.depth() == CV_32F
        && descriptor.elemSize() == sizeof(float));
    gval_mat2bytes(descriptor, result, n_points, dim);
  }
  else {
    *n_points = 0;
    *dim = 0;
  }
}

void gval_mat2bytes(Mat& matrix, void** result,
    int* rows, int* cols) {
  assert(matrix.channels() == 1);

  *rows = matrix.size().height;
  *cols = matrix.size().width;
  size_t size = matrix.size().height * matrix.size().width
    * matrix.elemSize();
  *result = malloc(size);

  if (!matrix.isContinuous()) {
    matrix = matrix.clone();
    assert(matrix.isContinuous());
  }

  memcpy(*result, matrix.data, size);
}

