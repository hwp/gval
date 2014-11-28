/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_mfcc.h : Mel-frequency Cepstrum Coefficient
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

#ifndef GVAL_MFCC_H_
#define GVAL_MFCC_H_

#include "gval_utils.h"

#include <gst/audio/gstaudiofilter.h>
#include <gst/base/gstadapter.h>

#include <stdio.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GVAL_TYPE_MFCC (gval_mfcc_get_type())
#define GVAL_MFCC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GVAL_TYPE_MFCC,GvalMfcc))
#define GVAL_MFCC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GVAL_TYPE_MFCC,GvalMfccClass))
#define GVAL_IS_MFCC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GVAL_TYPE_MFCC))
#define GVAL_IS_MFCC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GVAL_TYPE_MFCC))

typedef struct {
  GstAudioFilter base;

  gboolean silent;
  guint wsize;
  guint ssize;
  guint n_channels;
  guint cbegin;
  guint csize;
  const gchar* location;

  GstAdapter* adapter;
  gsize skip;
  window_func_t window_func;
  FILE* out;
  guint rate;
} GvalMfcc;

typedef struct {
  GstAudioFilterClass parent_class;
} GvalMfccClass;

GType gval_mfcc_get_type(void);

G_END_DECLS

#endif  // GVAL_MFCC_H_

