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

#include <assert.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>

gdouble gval_hann_window(guint index, guint wsize) {
  return 0.5 * (1 - cos(2 * M_PI * index / (wsize - 1)));
}

void gval_spectrum(gdouble* result, const gdouble* signal,
    guint size, window_func_t window) {
  assert(sizeof(gdouble) == sizeof(double));

  guint i;
  double* tsig = fftw_alloc_real(size);
  fftw_complex* dft = fftw_alloc_complex(size / 2 + 1);

  fftw_plan plan = fftw_plan_dft_r2c_1d(size, tsig, dft,
      FFTW_ESTIMATE);

  for (i = 0; i < size; i++) {
    tsig[i] = signal[i] * window(i, size);
  }
  fftw_execute(plan);

  fftw_destroy_plan(plan);
  fftw_free(tsig);
  fftw_free(dft);

  // Calculate spectrum
  for (i = 0; i < size / 2 + 1; i++) {
    result[i] = sqrt(dft[i][0] * dft[i][0] 
        + dft[i][1] * dft[i][1]);
  }
}

