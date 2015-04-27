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

using namespace cv;

#include <vector>
using std::vector;

#include <stdlib.h>
#include <assert.h>


void gval_draw_keypoints(void* img, int rows, int cols) {
  Mat image(rows, cols, CV_8UC3, img);

  SiftFeatureDetector detector;
  vector<KeyPoint> points;
  detector.detect(image, points);

  drawKeypoints(image, points, image, Scalar::all(-1),
      DrawMatchesFlags::DEFAULT);
}

static void gval_keypoint_filter(vector<KeyPoint>& points, double mscale) {
  vector<KeyPoint> copy(points);
  points.clear();
  for (vector<KeyPoint>::const_iterator it = copy.begin();
      it != copy.end(); it++) {
    if (it->size > mscale) {
      points.push_back(*it);
    }
  }
}

void gval_sift_descriptor(void* img, int rows, int cols,
    double mscale, void** result, int* n_points, int* dim) {
  Mat image(rows, cols, CV_8UC3, img);

  SiftFeatureDetector detector;
  vector<KeyPoint> points;
  detector.detect(image, points);
  gval_keypoint_filter(points, mscale);

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

  int type = m.type();
  int rows = m.rows;
  int cols = m.cols;
  int elem_size = m.elemSize();
  void* data = NULL;

  if (m.rows > 0 && m.cols > 0) {
    assert(m.dims == 2);
    if (!m.isContinuous()) {
      m = m.clone();
      assert(m.isContinuous());
    }

    assert(m.total() == rows * cols);
    data = m.data;
  }

  size_t ret;
  ret = fwrite(&type, sizeof(type), 1, stream);
  assert(ret == 1);
  ret = fwrite(&rows, sizeof(rows), 1, stream);
  assert(ret == 1);
  ret = fwrite(&cols, sizeof(cols), 1, stream);
  assert(ret == 1);
  ret = fwrite(&elem_size, sizeof(elem_size), 1, stream);
  assert(ret == 1);
  if (data) {
    ret = fwrite(data, elem_size, rows * cols, stream);
    assert(ret == rows * cols);
  }
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
  if (matrix == NULL) {
    return;
  }

  Mat* mat = (Mat*) matrix;
  if (mat->refcount == NULL) {
    free(mat->data);
  }

  delete mat;
}

int rank_cmp(const int* a, const int* b, int* value) {
  return value[*b] - value[*a];
}

void compute_rank(int* value, int* rank, int size) {
  int* order = (int*) malloc(sizeof(int) * size);
  for (int i = 0; i < size; i++) {
    order[i] = i;
  }

  qsort_r(order, size, sizeof(int),
      (int (*)(const void*, const void*, void*))rank_cmp, value);
  for (int i = 0; i < size; i++) {
    rank[order[i]] = i;
  }

  free(order);
}

bow_t* gval_load_bow(const char* voc_file) {
  bow_t* ret = (bow_t*) malloc(sizeof(bow_t));
  assert(ret);

  FILE* in = fopen(voc_file, "r");
  assert(in);
  Mat* voc = (Mat*) gval_read_cvmat(in);
  ret->size = voc->rows;
  ret->df = (int*) malloc(sizeof(int) * ret->size);
  ret->rank = (int*) malloc(sizeof(int) * ret->size);
  assert(ret->df && ret->rank);
  fread(ret->df, sizeof(int), ret->size, in);
  fread(&ret->dtotal, sizeof(int), 1, in);
  fclose(in);

  compute_rank(ret->df, ret->rank, ret->size);
  // debug
  for (int i = 0; i < ret->size; i++) {
    fprintf(stderr, "%d %d\n", ret->df[i], ret->rank[i]);
  }

  Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher);
  Ptr<DescriptorExtractor> extractor(new SiftDescriptorExtractor); 
  BOWImgDescriptorExtractor* bow 
    = new BOWImgDescriptorExtractor(extractor, matcher);

  bow->setVocabulary(voc->clone());
  gval_free_cvmat(voc);

  ret->extractor = bow;

  return ret;
}

void gval_free_bow(bow_t* bow) {
  if (bow) {
    free(bow->df);
    free(bow->rank);
    delete (BOWImgDescriptorExtractor*) bow->extractor;
    free(bow);
  }
}

static Scalar gen_color(double hue) {
  double g = fmax(1 - fabs(hue - 1.0 / 3.0), 0);
  double b = fmax(1 - fabs(hue - 2.0 / 3.0), 0);

  if (hue > 2.0 / 3.0) {
    hue -= 1.0;
  }
  double r = fmax(1 - fabs(hue), 0);

  return Scalar(255 * r, 255 * g, 255 * b);
}

void gval_bow_extract(void* img, int rows, int cols,
    bow_t* bow, int nstop, double mscale, double* result, int dim) {
  Mat image(rows, cols, CV_8UC3, img);
  BOWImgDescriptorExtractor extractor 
    = *(BOWImgDescriptorExtractor*) bow->extractor;

  int dsize = extractor.descriptorSize();
  assert(dim == dsize - nstop);


  SiftFeatureDetector detector;
  vector<KeyPoint> points;
  detector.detect(image, points);
  gval_keypoint_filter(points, mscale);

  if (!points.empty()) {
    Mat hist;
    vector<vector<int> > pids;
    extractor.compute(image, points, hist, &pids);

    assert(hist.rows == 1);
    assert(hist.cols == dsize);
    assert(hist.type() == CV_32F);
    assert(pids.size() == dsize);

    int c = 0;
    double sum = 0.0;
    for (int i = 0; i < dsize; i++) {
      if (bow->rank[i] >= nstop) {
        result[c] = hist.at<float>(i)
          * log((double) bow->dtotal / (double) bow->df[i]);
        sum += result[c] * result[c];
        c++;
      }
    }
    assert(c == dim);

    sum = sqrt(sum);
    if (sum > 0.0) {
      for (int i = 0; i < dim; i++) {
        result[i] /= sum;
      }
    }

    for (int i = 0; i < dsize; i++) {
      if (bow->rank[i] >= nstop) {
        Scalar color = gen_color((double) i / (double) dsize);
        for (vector<int>::const_iterator it = pids[i].begin(); it != pids[i].end(); it++) {
          KeyPoint p = points[*it];
          RotatedRect rRect(p.pt, Size2f(2.0 * p.size, 2.0* p.size), p.angle);
          Point2f vertices[4];
          rRect.points(vertices);
          for (int i = 0; i < 4; i++) {
            line(image, vertices[i], vertices[(i + 1) % 4], color);
          }

          Point2f index = p.pt;
          index.x += p.size * cos(p.angle / 180.0 * M_PI);
          index.y += p.size * sin(p.angle / 180.0 * M_PI);
          line(image, p.pt, index, color);
        }
      }
    }
  }
  else {
    for (int i = 0; i < dim; i++) {
      result[i] = 0.0;
    }
  }
}

