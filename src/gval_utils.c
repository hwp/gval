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

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <fftw3.h>

double gval_hann_window(unsigned int index, unsigned int wsize) {
  return 0.5 * (1 - cos(2 * M_PI * index / (wsize - 1)));
}

void gval_spectrum(double* result, const double* signal,
    unsigned int size, window_func_t window) {
  assert(sizeof(double) == sizeof(double));

  unsigned int i;
  double* tsig = fftw_alloc_real(size);
  fftw_complex* dft = fftw_alloc_complex(size / 2 + 1);

  fftw_plan plan = fftw_plan_dft_r2c_1d(size, tsig, dft,
      FFTW_ESTIMATE);

  for (i = 0; i < size; i++) {
    tsig[i] = signal[i] * window(i, size);
  }
  fftw_execute(plan);

  // Calculate spectrum
  for (i = 0; i < size / 2 + 1; i++) {
    result[i] = sqrt(dft[i][0] * dft[i][0]
        + dft[i][1] * dft[i][1]) / (double) size;
  }

  fftw_destroy_plan(plan);
  fftw_free(tsig);
  fftw_free(dft);
}

void gval_mfcc(double* result, const double* signal,
    unsigned int size, unsigned int n_channels, unsigned int spl_rate,
    window_func_t window) {
  unsigned int i, j;

  // Power spectrum
  double* spec = malloc((size / 2 + 1) * sizeof(double));
  gval_spectrum(spec, signal, size, window);

  // Plan for dct
  double* buf = fftw_alloc_real(n_channels);
  fftw_plan plan = fftw_plan_r2r_1d(n_channels, buf, buf, FFTW_REDFT10, FFTW_ESTIMATE);

  // Filter bank with mel scale
  double f_max = spl_rate / 2.0;
  double f_unit = (double) spl_rate / size;
  double f_begin = 0.0;
  double f_mid = 700.0 * (pow(1.0 + f_max / 700.0,
        1.0 / (n_channels + 1.0)) - 1.0);
  double f_end = 700.0 * (pow(1.0 + f_max / 700.0,
        2.0 / (n_channels + 1.0)) - 1.0);
  for (i = 0; i < n_channels; i++) {
    double sum = 0.0;
    for (j = (unsigned int) ceil(f_begin / f_unit);
        j< (unsigned int) floor(f_mid / f_unit); j++) {
      assert(j >= 0 && j < size / 2 + 1);
      sum += spec[j] * (j * f_unit - f_begin)
        / (f_mid - f_begin);
    }
    for ( ; j< (unsigned int) floor(f_end / f_unit); j++) {
      assert(j >= 0 && j < size / 2 + 1);
      sum += spec[j] * (f_end - j * f_unit)
        / (f_end - f_mid);
    }

    // log of filter bank
    buf[i] = log(sum);
  }

  free(spec);

  // calculate dct
  fftw_execute(plan);

  // copy result
  for (i = 0; i < n_channels; i++) {
    result[i] = buf[i];
  }

  fftw_destroy_plan(plan);
  fftw_free(buf);
}

