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

#include "gval_keypoints.h"
#include "gval_cv.hpp"

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

#include <stdio.h>

/* prototypes */

static gboolean gval_keypoints_set_info(GstVideoFilter* filter, GstCaps* incaps,
    GstVideoInfo* in_info, GstCaps* outcaps, GstVideoInfo* out_info);
static GstFlowReturn gval_keypoints_transform_frame_ip(
    GstVideoFilter* filter, GstVideoFrame* frame);

/* pad templates */

#define VIDEO_SRC_CAPS GST_VIDEO_CAPS_MAKE("{ RGB }")

#define VIDEO_SINK_CAPS GST_VIDEO_CAPS_MAKE("{ RGB }")

/* class initialization */

G_DEFINE_TYPE(GvalKeypoints, gval_keypoints,
    GST_TYPE_VIDEO_FILTER)

static void gval_keypoints_class_init(GvalKeypointsClass* klass) {
  //GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  //GstBaseTransformClass* base_transform_class
  //  = GST_BASE_TRANSFORM_CLASS(klass);
  GstVideoFilterClass* video_filter_class
    = GST_VIDEO_FILTER_CLASS(klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template(
      GST_ELEMENT_CLASS(klass),
      gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS,
        gst_caps_from_string(VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template(
      GST_ELEMENT_CLASS(klass),
      gst_pad_template_new("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
        gst_caps_from_string(VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata(GST_ELEMENT_CLASS(klass),
      "Key Points", "gval/visualize", "Draw Key Points",
      "Weipeng He <heweipeng@gmail.com>");

  video_filter_class->set_info
    = GST_DEBUG_FUNCPTR(gval_keypoints_set_info);
  video_filter_class->transform_frame_ip
    = GST_DEBUG_FUNCPTR(gval_keypoints_transform_frame_ip);
}

static void gval_keypoints_init(GvalKeypoints* keypoints) {
  // TODO
}

static gboolean gval_keypoints_set_info(GstVideoFilter* filter,
    GstCaps* incaps, GstVideoInfo* in_info,
    GstCaps* outcaps, GstVideoInfo* out_info) {
  printf("in info\n");
  printf("format: %s\n", in_info->finfo->name);
  printf("description: %s\n", in_info->finfo->description);
  printf("width: %d\n", in_info->width);
  printf("height: %d\n", in_info->height);
  printf("size: %zd\n", in_info->size);
  printf("views: %d\n", in_info->views);
  printf("par_n: %d\n", in_info->par_n);
  printf("par_d: %d\n", in_info->par_d);
  printf("fps_n: %d\n", in_info->fps_n);
  printf("fps_d: %d\n", in_info->fps_d);

  printf("out info\n");
  printf("format: %s\n", out_info->finfo->name);
  printf("description: %s\n", out_info->finfo->description);
  printf("width: %d\n", out_info->width);
  printf("height: %d\n", out_info->height);
  printf("size: %zd\n", out_info->size);
  printf("views: %d\n", out_info->views);
  printf("par_n: %d\n", out_info->par_n);
  printf("par_d: %d\n", out_info->par_d);
  printf("fps_n: %d\n", out_info->fps_n);
  printf("fps_d: %d\n", out_info->fps_d);

  return TRUE;
}

static GstFlowReturn gval_keypoints_transform_frame_ip(GstVideoFilter* filter,
    GstVideoFrame* frame) {
  //GvalKeypoints* this = GVAL_KEYPOINTS(filter);
  
  gval_draw_keypoints(GST_VIDEO_FRAME_PLANE_DATA(frame, 0),
      GST_VIDEO_FRAME_HEIGHT(frame),
      GST_VIDEO_FRAME_WIDTH(frame));

  return GST_FLOW_OK;
}

