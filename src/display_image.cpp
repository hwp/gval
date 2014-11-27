// display_image.cpp
// display a image and show the keypoints
//
// Author : Weipeng He <heweipeng@gmail.com>
// Copyright (c) 2014, All rights reserved.

#include <opencv2/core/core.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;

using std::cout;
using std::endl;

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " <file>" << endl;
    return -1;
  }

  Mat image;
  image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE); // Read the file

  if (!image.data) {                                // Check for invalid input
    cout <<  "Could not open or find the image" << endl;
    return -1;
  }

  SiftFeatureDetector detector;
  std::vector<KeyPoint> points;
  detector.detect(image, points);

  drawKeypoints(image, points, image, Scalar::all(-1),
      DrawMatchesFlags::DEFAULT);

  namedWindow("Display window", WINDOW_AUTOSIZE);   // Create a window for display.
  imshow("Display window", image);                  // Show our image inside it.

  waitKey(0);                                       // Wait for a keystroke in the window
  return 0;
}

