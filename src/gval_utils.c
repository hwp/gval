/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_utils.c : Utility Functions
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

#include "gval_utils.h"

#include <math.h>
#include <gsl/gsl_fft_complex.h>

gdouble gval_hann_window(guint index, guint wsize) {
  return 0.5 * (1 - cos(2 * M_PI * index / (wsize - 1)));
}

void gval_spectrum(gdouble* result, const gdouble* signal,
    guint size, window_func_t window) {
  guint i;
  gdouble* spectra = g_malloc_n(size * 2, sizeof(gdouble));
  for (i = 0; i < size; i++) {
    spectra[i * 2] = signal[i] * window(i, size);
    spectra[i * 2 + 1] = 0;
  }
  gsl_fft_complex_radix2_forward(spectra, 1, size);

  // Calculate spectrum
  for (i = 0; i < size / 2 + 1; i++) {
    result[i] = sqrt(spectra[i * 2] * spectra[i * 2]
        + spectra[i * 2 + 1] * spectra[i * 2 + 1]);
  }

  g_free(spectra);
}

