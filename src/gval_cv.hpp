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

/**
 * Find SIFT keypoints and draw on the image.
 * The raw data should be in 8-bit RGB format.
 */
EXTERNC
void draw_keypoints(unsigned char* img, int rows, int cols);

#undef EXTERNC

#endif  // GVAL_CV_HPP_

