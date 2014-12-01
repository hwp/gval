/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_sift.c : Extract SIFT descriptors
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gval_sift.h"
#include "gval_cv.hpp"

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

#include <stdio.h>

GST_DEBUG_CATEGORY_STATIC(gval_sift_debug_category);
#define GST_CAT_DEFAULT gval_sift_debug_category

/* prototypes */

static void gval_sift_set_property(GObject* object,
    guint property_id, const GValue* value, GParamSpec* pspec);
static void gval_sift_get_property(GObject* object,
    guint property_id, GValue* value, GParamSpec* pspec);
static void gval_sift_dispose(GObject* object);

static gboolean gval_sift_set_info(GstVideoFilter* filter, GstCaps* incaps,
    GstVideoInfo* in_info, GstCaps* outcaps, GstVideoInfo* out_info);
static GstFlowReturn gval_sift_transform_frame_ip(
    GstVideoFilter* filter, GstVideoFrame* frame);

enum {
  PROP_0,

  PROP_SILENT,
  PROP_LOCATION,
  
  N_PROPERTIES
};

static GParamSpec* sift_props[N_PROPERTIES] = { NULL, };

/* pad templates */

#define VIDEO_SRC_CAPS GST_VIDEO_CAPS_MAKE("{ RGB }")

#define VIDEO_SINK_CAPS GST_VIDEO_CAPS_MAKE("{ RGB }")

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(GvalSift, gval_sift,
    GST_TYPE_VIDEO_FILTER,
    GST_DEBUG_CATEGORY_INIT(gval_sift_debug_category,
      "sift", 0,
      "debug category for sift element"));

static void gval_sift_class_init(GvalSiftClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
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
      "SIFT", "gval/feature", "SIFT Descriptors",
      "Weipeng He <heweipeng@gmail.com>");

  gobject_class->set_property = gval_sift_set_property;
  gobject_class->get_property = gval_sift_get_property;
  gobject_class->dispose = gval_sift_dispose;
  
  sift_props[PROP_SILENT] = g_param_spec_boolean(
      "silent", "Silent", "Produce verbose output ?",
      FALSE, G_PARAM_READWRITE);
  sift_props[PROP_LOCATION] = g_param_spec_string(
      "location", "Location", "Path to the output file",
      NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class,
      N_PROPERTIES, sift_props);

 
  video_filter_class->set_info
    = GST_DEBUG_FUNCPTR(gval_sift_set_info);
  video_filter_class->transform_frame_ip
    = GST_DEBUG_FUNCPTR(gval_sift_transform_frame_ip);
}

static void gval_sift_init(GvalSift* this) {
  this->silent = FALSE;
  this->location = NULL;

  this->out = NULL;
}

void gval_sift_dispose(GObject* object) {
  GvalSift* this = GVAL_SIFT(object);

  if (this->out) {
    fclose(this->out);
  }

  G_OBJECT_CLASS(gval_sift_parent_class)->dispose(object);
}

void gval_sift_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec) {
  GvalSift* this = GVAL_SIFT(object);

  switch(prop_id) {
    case PROP_SILENT:
      this->silent = g_value_get_boolean(value);
      break;
    case PROP_LOCATION:
      this->location = g_value_dup_string(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void gval_sift_get_property(GObject* object,
    guint prop_id, GValue* value, GParamSpec* pspec) {
  GvalSift* this = GVAL_SIFT(object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean(value, this->silent);
      break;
    case PROP_LOCATION:
      g_value_set_string(value, this->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static gboolean gval_sift_set_info(GstVideoFilter* filter,
    GstCaps* incaps, GstVideoInfo* in_info,
    GstCaps* outcaps, GstVideoInfo* out_info) {
  GvalSift* this = GVAL_SIFT(filter);

  if (!this->silent) {
    printf("video info\n");
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
  }

  return TRUE;
}

static GstFlowReturn gval_sift_transform_frame_ip(GstVideoFilter* filter,
    GstVideoFrame* frame) {
  GvalSift* this = GVAL_SIFT(filter);

  if (this->location) {
    if (!this->out) {
      this->out = fopen(this->location, "w");
    }
    g_assert(this->out);

    void* descriptor;
    int n_points;
    int dim;
    gval_extract_descriptor(
        GST_VIDEO_FRAME_PLANE_DATA(frame, 0),
        GST_VIDEO_FRAME_HEIGHT(frame),
        GST_VIDEO_FRAME_WIDTH(frame), 
        &descriptor, &n_points, &dim);

    if (!this->silent) {
      printf("Descriptors (dim %d) of %d key points extracted.\n", dim, n_points);
    }

    fwrite(descriptor, sizeof(float), n_points * dim,
        this->out);
  }

  return GST_FLOW_OK;
}

static gboolean plugin_init(GstPlugin* plugin) {
  return gst_element_register(plugin, "sift",
      GST_RANK_NONE, GVAL_TYPE_SIFT);
}

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    gval_plugin,
    "SIFT",
    plugin_init,
    VERSION,
    "GPL",
    PACKAGE_NAME,
    GST_PACKAGE_ORIGIN
    );

