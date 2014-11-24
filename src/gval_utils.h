/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_utils.h : Utility Functions
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

#ifndef GVAL_UTILS_H_
#define GVAL_UTILS_H_

typedef double (*window_func_t)(unsigned int index, unsigned int wsize);

double gval_hann_window(unsigned int index, unsigned int wsize);

void gval_spectrum(double* result, const double* signal,
    unsigned int size, window_func_t window);

void gval_mfcc(double* result, const double* signal,
    unsigned int size, unsigned int n_channels, unsigned int spl_rate,
    window_func_t window);

#endif  // GVAL_UTILS_H_

