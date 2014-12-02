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
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

#include <fftw3.h>

#define MEL_SCALE_PORTION(f, p) \
  (700.0 * (pow(1.0 + (f) / 700.0, (p)) - 1.0))

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
  double f_mid = MEL_SCALE_PORTION(f_max,
      1.0 / (n_channels + 1.0));
  double f_end = MEL_SCALE_PORTION(f_max,
      2.0 / (n_channels + 1.0));
  for (i = 0; i < n_channels; i++) {
    double sum = 0.0;
    //printf("%g, %g, %g\n", f_begin, f_mid, f_end);
    for (j = (unsigned int) ceil(f_begin / f_unit);
        j <= (unsigned int) floor(f_mid / f_unit); j++) {
      assert(j >= 0 && j < size / 2 + 1);
      double w = (j * f_unit - f_begin) / (f_mid - f_begin);
      //printf("%d, %g, %g\n", j, j * f_unit, w);
      sum += spec[j] * w;
    }
    for ( ; j <= (unsigned int) floor(f_end / f_unit); j++) {
      assert(j >= 0 && j < size / 2 + 1);
      double w = (f_end - j * f_unit) / (f_end - f_mid);
      //printf("%d, %g, %g\n", j, j * f_unit, w);
      sum += spec[j] * w;
    }

    // log of filter bank
    buf[i] = log(sum);

    // Calculate new points
    f_begin = f_mid;
    f_mid = f_end;
    f_end = MEL_SCALE_PORTION(f_max,
      (i + 3.0) / (n_channels + 1.0));
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

void gval_sigaction(int signum, siginfo_t* siginfo,
    void* context) {
  fprintf(stderr, "Signal Catched (No: %d, Code: %d)\n",
      siginfo->si_signo, siginfo->si_code);
  fprintf(stderr, "Debug with gdb, run: gdb -p %d\n",
      siginfo->si_pid);
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(2);
  }
  else if (pid) {
    char s[13];
    sprintf(s, "%d", pid);
    execlp("gdb", "gdb", "-p", s, NULL);
  } else {
    setpgid(0, getpid());
    kill(getpid(), SIGSTOP);
  }
}

void gval_debug_init(void) {
#ifndef NDEBUG
  struct sigaction act;
  memset(&act, '\0', sizeof(act));
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = gval_sigaction;

  if (sigaction(SIGINT, &act, NULL) < 0
      || sigaction(SIGABRT, &act, NULL) < 0
      || sigaction(SIGSEGV, &act, NULL) < 0) {  
    perror ("sigaction");  
    exit(2);  
  }  
#endif // NDEBUG
}

