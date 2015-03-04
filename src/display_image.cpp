// display_image.cpp
// display a image and show the keypoints
//
// Author : Weipeng He <heweipeng@gmail.com>
// Copyright (c) 2014, All rights reserved.

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

using std::cout;
using std::endl;

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " <file>" << endl;
    return -1;
  }

  Mat image, raw;
  raw = imread(argv[1]); // Read the file
  cvtColor(raw, image, CV_BGR2GRAY);

  if (!image.data) {                                // Check for invalid input
    cout <<  "Could not open or find the image" << endl;
    return -1;
  }

  SiftFeatureDetector detector;
  std::vector<KeyPoint> points;
  detector.detect(image, points);

  SiftDescriptorExtractor extractor;
  Mat descriptor;
  extractor.compute(image, points, descriptor);

  drawKeypoints(raw, points, raw, Scalar::all(-1),
      DrawMatchesFlags::DEFAULT);

  namedWindow("Display window", WINDOW_AUTOSIZE);   // Create a window for display.
  imshow("Display window", raw);                  // Show our image inside it.

  waitKey(0);                                       // Wait for a keystroke in the window

  if (argc == 3) {
    imwrite(argv[2], raw);
  }

  return 0;
}

