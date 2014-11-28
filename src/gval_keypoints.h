/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_keypoints.c : Show keypoints in video
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

#ifndef GVAL_KEYPOINTS_H_
#define GVAL_KEYPOINTS_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GVAL_TYPE_KEYPOINTS (gval_keypoints_get_type())
#define GVAL_KEYPOINTS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GVAL_TYPE_KEYPOINTS,GvalKeypoints))
#define GVAL_KEYPOINTS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GVAL_TYPE_KEYPOINTS,GvalKeypointsClass))
#define GVAL_IS_KEYPOINTS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GVAL_TYPE_KEYPOINTS))
#define GVAL_IS_KEYPOINTS_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GVAL_TYPE_KEYPOINTS))

typedef struct {
  GstVideoFilter base_keypoints;
} GvalKeypoints;

typedef struct {
  GstVideoFilterClass base_keypoints_class;
} GvalKeypointsClass;

GType gst_keypoints_get_type (void);

G_END_DECLS

#endif  // GVAL_KEYPOINTS_H_

