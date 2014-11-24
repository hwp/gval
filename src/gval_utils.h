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

#include <glib.h>

typedef gdouble (*window_func_t)(guint index, guint wsize);

gdouble gval_hann_window(guint index, guint wsize);

void gval_spectrum(gdouble* result, const gdouble* signal,
    guint size, window_func_t window);

void gval_mfcc(gdouble* result, const gdouble* signal,
    guint size, guint n_channels, guint spl_rate,
    window_func_t window);

#endif  // GVAL_UTILS_H_

