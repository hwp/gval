/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_bow.c : Extract Bag-of-Words Feature
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

#include "gval_bow.h"
#include "gval_cv.hpp"

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* prototypes */

static void gval_bow_set_property(GObject* object,
    guint property_id, const GValue* value, GParamSpec* pspec);
static void gval_bow_get_property(GObject* object,
    guint property_id, GValue* value, GParamSpec* pspec);
static void gval_bow_dispose(GObject* object);

static gboolean gval_bow_set_info(GstVideoFilter* filter, GstCaps* incaps,
    GstVideoInfo* in_info, GstCaps* outcaps, GstVideoInfo* out_info);
static GstFlowReturn gval_bow_transform_frame_ip(
    GstVideoFilter* filter, GstVideoFrame* frame);

enum {
  PROP_0,

  PROP_SILENT,
  PROP_FLUSH,
  PROP_LOCATION,
  PROP_VOCABULARY,
  PROP_NSTOP,
  
  N_PROPERTIES
};

static GParamSpec* bow_props[N_PROPERTIES] = { NULL, };

/* pad templates */

#define VIDEO_SRC_CAPS GST_VIDEO_CAPS_MAKE("{ RGB }")

#define VIDEO_SINK_CAPS GST_VIDEO_CAPS_MAKE("{ RGB }")

/* class initialization */

G_DEFINE_TYPE(GvalBow, gval_bow, GST_TYPE_VIDEO_FILTER);

static void gval_bow_class_init(GvalBowClass* klass) {
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
      "BOW", "gval/feature", "Bag-of-Words Feature Extractor",
      "Weipeng He <heweipeng@gmail.com>");

  gobject_class->set_property = gval_bow_set_property;
  gobject_class->get_property = gval_bow_get_property;
  gobject_class->dispose = gval_bow_dispose;
  
  bow_props[PROP_SILENT] = g_param_spec_boolean(
      "silent", "Silent", "Produce verbose output ?",
      FALSE, G_PARAM_READWRITE);
  bow_props[PROP_FLUSH] = g_param_spec_boolean(
      "flush", "Flush", "Flush ouput stream immediately?",
      FALSE, G_PARAM_READWRITE);
  bow_props[PROP_LOCATION] = g_param_spec_string(
      "location", "Location", "Path to the output file",
      NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  bow_props[PROP_VOCABULARY] = g_param_spec_string(
      "vocabulary", "Vocabulary", "BoW vocabulary file",
      NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  bow_props[PROP_NSTOP] = g_param_spec_uint(
      "nstop", "# of stopwords", "Number of words to be ignored",
      0, 1024, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class,
      N_PROPERTIES, bow_props);

 
  video_filter_class->set_info
    = GST_DEBUG_FUNCPTR(gval_bow_set_info);
  video_filter_class->transform_frame_ip
    = GST_DEBUG_FUNCPTR(gval_bow_transform_frame_ip);
}

static void gval_bow_init(GvalBow* this) {
  this->silent = FALSE;
  this->flush = FALSE;
  this->location = NULL;
  this->vocabulary = NULL;

  this->out = NULL;
  this->bow = NULL;
}

void gval_bow_dispose(GObject* object) {
  GvalBow* this = GVAL_BOW(object);

  if (this->out) {
    fclose(this->out);
  }

  gval_free_bow(this->bow);

  G_OBJECT_CLASS(gval_bow_parent_class)->dispose(object);
}

void gval_bow_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec) {
  GvalBow* this = GVAL_BOW(object);

  switch(prop_id) {
    case PROP_SILENT:
      this->silent = g_value_get_boolean(value);
      break;
    case PROP_FLUSH:
      this->flush = g_value_get_boolean(value);
      break;
    case PROP_LOCATION:
      this->location = g_value_dup_string(value);
      break;
    case PROP_VOCABULARY:
      this->vocabulary = g_value_dup_string(value);
      break;
    case PROP_NSTOP:
      this->nstop = g_value_get_uint(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

void gval_bow_get_property(GObject* object,
    guint prop_id, GValue* value, GParamSpec* pspec) {
  GvalBow* this = GVAL_BOW(object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean(value, this->silent);
      break;
    case PROP_FLUSH:
      g_value_set_boolean(value, this->flush);
      break;
    case PROP_LOCATION:
      g_value_set_string(value, this->location);
      break;
    case PROP_VOCABULARY:
      g_value_set_string(value, this->vocabulary);
      break;
    case PROP_NSTOP:
      g_value_set_uint(value, this->nstop);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static gboolean gval_bow_set_info(GstVideoFilter* filter,
    GstCaps* incaps, GstVideoInfo* in_info,
    GstCaps* outcaps, GstVideoInfo* out_info) {
  GvalBow* this = GVAL_BOW(filter);

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

static GstFlowReturn gval_bow_transform_frame_ip(GstVideoFilter* filter,
    GstVideoFrame* frame) {
  GvalBow* this = GVAL_BOW(filter);

  if (this->location) {
    if (!this->out) {
      this->out = fopen(this->location, "w");
    }
    assert(this->out);

    if (!this->bow) {
      this->bow = gval_load_bow(this->vocabulary);
    }
    assert(this->bow);

    double* descriptor = NULL;
    int dim;
    gval_bow_extract(
        GST_VIDEO_FRAME_PLANE_DATA(frame, 0),
        GST_VIDEO_FRAME_HEIGHT(frame),
        GST_VIDEO_FRAME_WIDTH(frame), 
        this->bow, this->nstop, &descriptor, &dim);

    if (!this->silent) {
      printf("Image descriptor (dim %d) extracted.\n", dim);
    }

    fwrite(descriptor, sizeof(double), dim, this->out);
    if (this->flush) {
      fflush(this->out);
    }
    free(descriptor);
  }

  return GST_FLOW_OK;
}

