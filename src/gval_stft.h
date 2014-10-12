/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_stft.h : Short Time Fourier Transform
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

#ifndef GVAL_STFT_H_
#define GVAL_STFT_H_

#include <gst/gst.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GVAL_TYPE_STFT \
  (gval_sftf_get_type())
#define GVAL_STFT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GVAL_TYPE_STFT,GvalStft))
#define GVAL_STFT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GVAL_TYPE_STFT,GvalStftClass))
#define GVAL_IS_STFT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GVAL_TYPE_STFT))
#define GVAL_IS_STFT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GVAL_TYPE_STFT))

typedef struct {
  GstElement element;

  GstPad* sinkpad;
  GstPad* srcpad;
  
  gboolean silent;
  guint wsize;
  guint ssize;

  guint start;
  guint current;
  gsize skip;
  gdouble* buffer;
  gdouble (*window_func)(guint index, guint wsize);
} GvalStft;

typedef struct {
  GstElementClass parent_class;
} GvalStftClass;

GType gval_sftf_get_type(void);

G_END_DECLS

#endif  // GVAL_STFT_H_

