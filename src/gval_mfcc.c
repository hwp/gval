/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_mfcc.c : Mel-frequency Cepstrum Coefficient
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
#  include <config.h>
#endif

#include "gval_mfcc.h"
#include "gval_utils.h"

#include <math.h>
#include <gst/gst.h>

GST_DEBUG_CATEGORY_STATIC(gval_mfcc_debug);
#define GST_CAT_DEFAULT gval_mfcc_debug

#define MIN_WINDOW_SIZE 4
#define MAX_WINDOW_SIZE 65536
#define DEFAULT_WINDOW_SIZE 1024

#define MIN_SHIFT_SIZE 1
#define MAX_SHIFT_SIZE 65536
#define DEFAULT_SHIFT_SIZE 256

#define MIN_N_CHANNELS 4
#define MAX_N_CHANNELS 128
#define DEFAULT_N_CHANNELS 16

/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  PROP_0,

  PROP_SILENT,
  PROP_WSIZE,
  PROP_SSIZE,
  PROP_N_CHANNELS,
  PROP_LOCATION,

  N_PROPERTIES
};

static GParamSpec* mfcc_props[N_PROPERTIES] = { NULL, };

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(
      "audio/x-raw, "
      "format = (string) F64LE, "
      "channels = (int) 1, "
      "rate = (int) [8000, 96000]"
      )
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("ANY")
    );

#define gval_mfcc_parent_class parent_class
G_DEFINE_TYPE(GvalMfcc, gval_mfcc, GST_TYPE_BASE_TRANSFORM);

static void gval_mfcc_dispose(GObject* object);
static void gval_mfcc_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec);
static void gval_mfcc_get_property(GObject* object, guint prop_id,
    GValue* value, GParamSpec* pspec);

static gboolean gval_mfcc_sink_event(GstBaseTransform* trans,
    GstEvent* event);
static GstFlowReturn gval_mfcc_transform_ip(GstBaseTransform* trans,
    GstBuffer* inbuf);

/* initialize the mfcc's class */
static void gval_mfcc_class_init(GvalMfccClass* klass) {
  GObjectClass* gobject_class = (GObjectClass*) klass;
  GstElementClass* gstelement_class
    = (GstElementClass*) klass;
  GstBaseTransformClass* base_transform_class
    = GST_BASE_TRANSFORM_CLASS (klass);

  gobject_class = (GObjectClass*) klass;
  gstelement_class = (GstElementClass*) klass;

  gobject_class->set_property = gval_mfcc_set_property;
  gobject_class->get_property = gval_mfcc_get_property;
  gobject_class->dispose = gval_mfcc_dispose;

  mfcc_props[PROP_SILENT] = g_param_spec_boolean(
      "silent", "Silent", "Produce verbose output ?",
      FALSE, G_PARAM_READWRITE);
  mfcc_props[PROP_WSIZE] = g_param_spec_uint(
      "wsize", "Window Size", "Window Size of the FFT",
      MIN_WINDOW_SIZE, MAX_WINDOW_SIZE, DEFAULT_WINDOW_SIZE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  mfcc_props[PROP_SSIZE] = g_param_spec_uint(
      "ssize", "Shift Size", "Shift Size of the FFT",
      MIN_SHIFT_SIZE, MAX_SHIFT_SIZE, DEFAULT_SHIFT_SIZE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  mfcc_props[PROP_N_CHANNELS] = g_param_spec_uint(
      "banks", "Banks", "Number of filter banks",
      MIN_N_CHANNELS, MAX_N_CHANNELS, DEFAULT_N_CHANNELS,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  mfcc_props[PROP_LOCATION] = g_param_spec_string(
      "location", "Location", "Path to the output file",
      NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class,
      N_PROPERTIES, mfcc_props);

  gst_element_class_set_details_simple(gstelement_class,
      "mfcc",
      "gval/feature",
      "Mel-frequency Cepstrum Coefficient",
      "Weipeng He <heweipeng@gmail.com>");

  gst_element_class_add_pad_template(gstelement_class,
      gst_static_pad_template_get(&src_factory));
  gst_element_class_add_pad_template(gstelement_class,
      gst_static_pad_template_get(&sink_factory));

  base_transform_class->sink_event
    = GST_DEBUG_FUNCPTR(gval_mfcc_sink_event);
  base_transform_class->transform_ip
    = GST_DEBUG_FUNCPTR(gval_mfcc_transform_ip);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void gval_mfcc_init(GvalMfcc* this) {
  this->silent = FALSE;
  this->wsize = 0;
  this->ssize = 0;
  this->location = NULL;

  this->adapter = gst_adapter_new();
  this->skip = 0;
  this->window_func = gval_hann_window;
  this->out = NULL;
}

static void gval_mfcc_dispose(GObject* object) {
  GvalMfcc* this = GVAL_MFCC(object);
  g_object_unref(this->adapter);
  if (this->out) {
    fclose(this->out);
  }

  G_OBJECT_CLASS(gval_mfcc_parent_class)->dispose(object);
}

static void gval_mfcc_set_property(GObject* object,
    guint prop_id, const GValue* value, GParamSpec* pspec) {
  GvalMfcc* this = GVAL_MFCC(object);

  switch (prop_id) {
    case PROP_SILENT:
      this->silent = g_value_get_boolean(value);
      break;
    case PROP_WSIZE:
      this->wsize = g_value_get_uint(value);
      break;
    case PROP_SSIZE:
      this->ssize = g_value_get_uint(value);
      break;
    case PROP_N_CHANNELS:
      this->n_channels = g_value_get_uint(value);
      break;
    case PROP_LOCATION:
      this->location = g_value_dup_string(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void gval_mfcc_get_property(GObject* object,
    guint prop_id, GValue* value, GParamSpec* pspec) {
  GvalMfcc* this = GVAL_MFCC(object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean(value, this->silent);
      break;
    case PROP_WSIZE:
      g_value_set_uint(value, this->wsize);
      break;
    case PROP_SSIZE:
      g_value_set_uint(value, this->ssize);
      break;
    case PROP_N_CHANNELS:
      g_value_set_uint(value, this->n_channels);
      break;
    case PROP_LOCATION:
      g_value_set_string(value, this->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean gval_mfcc_sink_event(GstBaseTransform* trans,
    GstEvent* event) {
  GvalMfcc* this = GVAL_MFCC(trans);

  gboolean ret;

  switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CAPS:
      /* forward */
      ret = GST_BASE_TRANSFORM_CLASS(gval_mfcc_parent_class)
        ->sink_event(trans, event);
      {
        GstCaps* caps = gst_pad_get_current_caps(
            GST_BASE_TRANSFORM_SRC_PAD(trans));
        GstStructure* s = gst_caps_get_structure(caps, 0);
        this->rate = g_value_get_int(
            gst_structure_get_value(s, "rate"));

        if (!this->silent) {
          printf("Sample Rate : %d\n", this->rate);
          printf("Window Size: %d\n", this->wsize);
          printf("Shift Size: %d\n", this->ssize);
          printf("# Filter Banks : %d\n", this->n_channels);
        }
      }
      break;
    default:
      ret = GST_BASE_TRANSFORM_CLASS(gval_mfcc_parent_class)
        ->sink_event(trans, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn gval_mfcc_transform_ip(GstBaseTransform* trans,
    GstBuffer* buf) {
  GvalMfcc* this = GVAL_MFCC(trans);

  gst_adapter_push(this->adapter, buf);
  gst_buffer_ref(buf);

  gsize skipped = gst_adapter_available(this->adapter);
  if (this->skip > 0) {
    skipped = MIN(this->skip, skipped);
    gst_adapter_flush(this->adapter, skipped);
    this->skip -= skipped;
  }

  gsize avail = gst_adapter_available(this->adapter);
  while (avail >= this->wsize * sizeof(gdouble)) {
    // One full window
    const gdouble* data = gst_adapter_map(this->adapter,
        this->wsize * sizeof(gdouble));
    gdouble* mfcc = g_malloc_n(this->n_channels,
        sizeof(gdouble));
    gval_mfcc(mfcc, data, this->wsize, this->n_channels,
        this->rate, this->window_func);
    gst_adapter_unmap(this->adapter);

    // Write to file
    if (this->location) {
      if (!this->out) {
        this->out = fopen(this->location, "w");
      }
      g_assert(this->out);
      fwrite(mfcc, sizeof(double), this->n_channels,
          this->out);
    }

    g_free(mfcc);

    // Shift
    skipped = MIN(this->ssize * sizeof(gdouble), avail);
    gst_adapter_flush(this->adapter, skipped);
    avail -= skipped;
    this->skip = this->ssize * sizeof(gdouble) - skipped;
  }

  return GST_FLOW_OK;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean mfcc_init(GstPlugin* mfcc) {
  /* debug category for fltering log messages
   */
  GST_DEBUG_CATEGORY_INIT(gval_mfcc_debug, "mfcc",
      0, "Mel-frequency Cepstrum Coefficient");

  return gst_element_register(mfcc, "mfcc", GST_RANK_NONE,
      GVAL_TYPE_MFCC);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gval-package"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://www.github.com/hwp/gval"
#endif


/* gstreamer looks for this structure to register mfccs
 */
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    mfcc,
    "Mel-frequency Cepstrum Coefficient",
    mfcc_init,
    VERSION,
    "GPL",
    "Unknown",
    GST_PACKAGE_ORIGIN
    );
