// draw_keypoints.cpp
// Extract all keypoints and saved as separate files.
//
// Author : Weipeng He <heweipeng@gmail.com>
// Copyright (c) 2014, All rights reserved.

extern "C" {
#include "gval_utils.h"
}

#include "gval_cv.hpp"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using std::string;

#define FRAME_SIZE 128 

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

static Mat get_patch(KeyPoint p, Mat img) {
  Size dsize(p.size * 2, p.size * 2);
  Mat ret;
  Mat rot_mat = getRotationMatrix2D(p.pt, p.angle, 1.0);
  warpAffine(img, ret, rot_mat, img.size(), INTER_CUBIC);
  getRectSubPix(ret, dsize, p.pt, ret);
  resize(ret, ret, Size(FRAME_SIZE, FRAME_SIZE), 0, 0, INTER_CUBIC);
  
  return ret;
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

int main(int argc, char** argv) {
  gval_debug_init();

  // get options
  int showhelp = 0;
  string voc_file;
  double mscale = 0.0;

  int opt;
  while ((opt = getopt(argc, argv, "v:m:h")) != -1) {
    switch (opt) {
      case 'h':
        showhelp = 1;
        break;
      case 'v':
        voc_file = string(optarg);
        break;
      case 'm':
        mscale = atof(optarg);
        break;
      default:
        showhelp = 1;
        break;
    }
  }

  if (showhelp || voc_file.empty() || argc - optind < 1) {
    fprintf(stderr, "Usage: %s -v voc_file image\n", argv[0]);
    return EXIT_SUCCESS;
  }

  bow_t* bow = gval_load_bow(voc_file.c_str());
  assert(bow && bow->extractor);
  BOWImgDescriptorExtractor extractor 
    = *(BOWImgDescriptorExtractor*) bow->extractor;

  Mat image, raw;
  string name(argv[optind]);
  raw = imread(name); // Read the file
  assert(raw.data);
  size_t e = name.rfind('.');
  size_t s = name.rfind('/') + 1;
  name = name.substr(s, e - s);
 
  cvtColor(raw, image, CV_BGR2GRAY);

  SiftFeatureDetector detector;
  std::vector<KeyPoint> points;
  detector.detect(image, points);
  gval_keypoint_filter(points, mscale);

  vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
  compression_params.push_back(0);

  Mat hist;
  vector<vector<int> > pids;
  if (!points.empty()) {
    extractor.compute(image, points, hist, &pids);

    int dsize = extractor.descriptorSize();

    Mat full = raw.clone();
    for (int i = 0; i < dsize; i++) {
      Scalar color = gen_color((double) i / (double) dsize);
      for (vector<int>::const_iterator it = pids[i].begin(); it != pids[i].end(); it++) {
        KeyPoint p = points[*it];
        RotatedRect rRect(p.pt, Size2f(2.0 * p.size, 2.0* p.size), p.angle);
        Point2f vertices[4];
        rRect.points(vertices);
        for (int i = 0; i < 4; i++) {
          line(full, vertices[i], vertices[(i + 1) % 4], color);
        }

        Point2f index = p.pt;
        index.x += p.size * cos(p.angle / 180.0 * M_PI);
        index.y += p.size * sin(p.angle / 180.0 * M_PI);
        line(full, p.pt, index, color);
      }
    }
    imwrite(name + "_full.png", full, compression_params);

    for (int i = 0; i < dsize; i++) {
      for (int j = 0; j < pids[i].size(); j++) {
        KeyPoint p = points[pids[i][j]];
        Mat patch = get_patch(p, raw);
        std::stringstream ss;
        ss << name << "_v" << i << "_" << j << ".png";

        imwrite(ss.str(), patch, compression_params);
      }
    }

    fprintf(stderr, "%d keypoints drawed\n", points.size()); 
  }
  else {
    fprintf(stderr, "No keypoints detected\n"); 
  }

  return 0;
}

