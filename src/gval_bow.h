/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_bow.h : Extract Bag-of-Words Feature
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

#ifndef GVAL_BOW_H_
#define GVAL_BOW_H_

#include "gval_cv.hpp"

#include <stdio.h>

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GVAL_TYPE_BOW (gval_bow_get_type())
#define GVAL_BOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GVAL_TYPE_BOW,GvalBow))
#define GVAL_BOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GVAL_TYPE_BOW,GvalBowClass))
#define GVAL_IS_BOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GVAL_TYPE_BOW))
#define GVAL_IS_BOW_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GVAL_TYPE_BOW))

typedef struct {
  GstVideoFilter base;

  gboolean silent;
  gboolean flush;
  const gchar* location;
  const gchar* vocabulary;
  unsigned int nstop;
  double mscale;

  FILE* out;
  bow_t* bow;
} GvalBow;

typedef struct {
  GstVideoFilterClass base_class;
} GvalBowClass;

GType gval_bow_get_type(void);

G_END_DECLS

#endif  // GVAL_BOW_H_

