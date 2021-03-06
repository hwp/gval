/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_cv.hpp : CV function using OpenCV (C++)
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

#ifndef GVAL_CV_HPP_
#define GVAL_CV_HPP_

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include <stdio.h>

typedef struct {
  int size;         // size of vocabulary
  int dtotal;       // number of documents
  int* df;          // document frequency
  int* rank;        // rank of df
  void* extractor;
} bow_t;

/**
 * Find SIFT keypoints and draw on the image.
 * The raw data should be in 8-bit RGB format.
 */
EXTERNC
void gval_draw_keypoints(void* img, int rows, int cols);

/**
 * Extract SIFT descriptors.
 * The raw data should be in 8-bit RGB format.
 *
 * result is a pointer to C++ Class cv::Mat.
 * It should be freed using gval_free_cvmat().
 */
EXTERNC
void gval_sift_descriptor(void* img, int rows, int cols,
    double mscale, void** result, int* n_points, int* dim);

/**
 * Write a cv::Mat to stream.
 */
EXTERNC
void gval_write_cvmat(const void* matrix, FILE* stream);

/**
 * Read a cv::Mat from stream.
 * @return the matrix or NULL if EOF
 * @warning The returned matrix should be freed using gval_free_cvmat().
 */
EXTERNC
void* gval_read_cvmat(FILE* stream);

/**
 * Free a cv::Mat.
 */
EXTERNC
void gval_free_cvmat(void* matrix);

/**
 * Load a Bag-of-Words feature extractor from
 * a vocabulary file.
 *
 * the returned pointer should be freed using gval_free_bow()
 */
EXTERNC
bow_t* gval_load_bow(const char* voc_file);

/**
 * Free a cv::BOWImgDescriptorExtractor
 */
EXTERNC
void gval_free_bow(bow_t* bow);

/**
 * Extract BoW feature.
 * The raw data should be in 8-bit RGB format.
 * 
 * result is a double array which is allocated in advance.
 */
EXTERNC
void gval_bow_extract(void* img, int rows, int cols,
    bow_t* bow, int nstop, double mscale, double* result, int dim);

#undef EXTERNC

#endif  // GVAL_CV_HPP_

