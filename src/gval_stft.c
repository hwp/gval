/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_stft.c : Short Time Fourier Transform
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

#include "gval_stft.h"
#include "gval_utils.h"

#include <math.h>
#include <gst/gst.h>
#include <gsl/gsl_fft_complex.h>

GST_DEBUG_CATEGORY_STATIC(gval_stft_debug);
#define GST_CAT_DEFAULT gval_stft_debug

#define MIN_WINDOW_SIZE 4
#define MAX_WINDOW_SIZE 65536
#define DEFAULT_WINDOW_SIZE 1024

#define MIN_SHIFT_SIZE 1
#define MAX_SHIFT_SIZE 65536
#define DEFAULT_SHIFT_SIZE 256

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

  N_PROPERTIES
};

static GParamSpec* stft_props[N_PROPERTIES] = { NULL, };

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
    /*
      "application/unknown, "
      "format = (string) F64LE, "
      "n_feature = (int) [" G_STRINGIFY(MIN_WINDOW_SIZE) ", " G_STRINGIFY(MAX_WINDOW_SIZE) "]")
      */
    );

#define gval_stft_parent_class parent_class
G_DEFINE_TYPE(GvalStft, gval_stft, GST_TYPE_BASE_TRANSFORM);

static void gval_stft_dispose(GObject* object);
static void gval_stft_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec);
static void gval_stft_get_property(GObject* object, guint prop_id,
    GValue* value, GParamSpec* pspec);

static gboolean gval_stft_sink_event(GstBaseTransform* trans,
    GstEvent* event);
static GstFlowReturn gval_stft_transform(GstBaseTransform* trans,
    GstBuffer* inbuf, GstBuffer* outbuf);

static gboolean print_field(GQuark field, const GValue* value, gpointer pfx);
static void print_caps(const GstCaps* caps, const gchar* pfx);

/* initialize the stft's class */
static void gval_stft_class_init(GvalStftClass* klass) {
  GObjectClass* gobject_class = (GObjectClass*) klass;
  GstElementClass* gstelement_class
    = (GstElementClass*) klass;
  GstBaseTransformClass* base_transform_class
    = GST_BASE_TRANSFORM_CLASS (klass);

  gobject_class = (GObjectClass*) klass;
  gstelement_class = (GstElementClass*) klass;

  gobject_class->set_property = gval_stft_set_property;
  gobject_class->get_property = gval_stft_get_property;
  gobject_class->dispose = gval_stft_dispose;

  stft_props[PROP_SILENT] = g_param_spec_boolean(
      "silent", "Silent", "Produce verbose output ?",
      FALSE, G_PARAM_READWRITE);
  stft_props[PROP_WSIZE] = g_param_spec_uint(
      "wsize", "Window Size", "Window Size of the FFT",
      MIN_WINDOW_SIZE, MAX_WINDOW_SIZE, DEFAULT_WINDOW_SIZE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  stft_props[PROP_SSIZE] = g_param_spec_uint(
      "ssize", "Shift Size", "Shift Size of the FFT",
      MIN_SHIFT_SIZE, MAX_SHIFT_SIZE, DEFAULT_SHIFT_SIZE,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class,
      N_PROPERTIES, stft_props);

  gst_element_class_set_details_simple(gstelement_class,
      "stft",
      "gval/feature",
      "Short Time Fourier Transform",
      "Weipeng He <heweipeng@gmail.com>");

  gst_element_class_add_pad_template(gstelement_class,
      gst_static_pad_template_get(&src_factory));
  gst_element_class_add_pad_template(gstelement_class,
      gst_static_pad_template_get(&sink_factory));

  base_transform_class->sink_event
    = GST_DEBUG_FUNCPTR(gval_stft_sink_event);
  base_transform_class->transform
    = GST_DEBUG_FUNCPTR(gval_stft_transform);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void gval_stft_init(GvalStft* self) {
  self->silent = FALSE;
  self->wsize = 0;
  self->ssize = 0;

  self->start = 0;
  self->current = 0;
  self->skip = 0;
  self->buffer = g_malloc_n(MAX_WINDOW_SIZE,
      sizeof(gdouble));
  self->window_func = gval_hann_window;
}

static void gval_stft_dispose(GObject* object) {
  GvalStft* self = GVAL_STFT(object);
  g_free(self->buffer);

  G_OBJECT_CLASS(gval_stft_parent_class)->dispose(object);
}

static void gval_stft_set_property(GObject* object,
    guint prop_id, const GValue* value, GParamSpec* pspec) {
  GvalStft* self = GVAL_STFT(object);

  switch (prop_id) {
    case PROP_SILENT:
      self->silent = g_value_get_boolean(value);
      break;
    case PROP_WSIZE:
      self->wsize = g_value_get_uint(value);
      g_message("wsize <= %"G_GUINT32_FORMAT"\n", self->wsize);
      break;
    case PROP_SSIZE:
      self->ssize = g_value_get_uint(value);
      g_message("ssize <= %"G_GUINT32_FORMAT"\n", self->ssize);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void gval_stft_get_property(GObject* object,
    guint prop_id, GValue* value, GParamSpec* pspec) {
  GvalStft* self = GVAL_STFT(object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean(value, self->silent);
      break;
    case PROP_WSIZE:
      g_value_set_uint(value, self->wsize);
      break;
    case PROP_SSIZE:
      g_value_set_uint(value, self->ssize);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean gval_stft_sink_event(GstBaseTransform* trans,
    GstEvent* event) {
  GvalStft* self = GVAL_STFT(trans);

  gboolean ret;
  GstCaps* caps = NULL;

  switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CAPS:
      gst_event_parse_caps(event, &caps);
      /* do something with the caps */

      if (!self->silent) {
        print_caps(caps, "");
      }

      /* and forward */
      ret = GST_BASE_TRANSFORM_CLASS(gval_stft_parent_class)
        ->sink_event(trans, event);
      break;
    default:
      ret = GST_BASE_TRANSFORM_CLASS(gval_stft_parent_class)
        ->sink_event(trans, event);
      break;
  }
  return ret;
}

/* Functions below print the Capabilities in a human-friendly format */
static gboolean print_field(GQuark field, const GValue* value, gpointer pfx) {
  gchar* str = gst_value_serialize(value);

  g_print("%s  %15s: %s\n", (gchar*) pfx, g_quark_to_string(field), str);
  g_free(str);
  return TRUE;
}

static void print_caps(const GstCaps* caps, const gchar* pfx) {
  guint i;

  g_return_if_fail(caps != NULL);

  if (gst_caps_is_any(caps)) {
    g_print("%sANY\n", pfx);
    return;
  }
  if (gst_caps_is_empty(caps)) {
    g_print("%sEMPTY\n", pfx);
    return;
  }

  for (i = 0; i < gst_caps_get_size(caps); i++) {
    GstStructure* structure = gst_caps_get_structure(caps, i);

    g_print("%s%s\n", pfx, gst_structure_get_name(structure));
    gst_structure_foreach(structure, print_field, (gpointer) pfx);
  }
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn gval_stft_transform(GstBaseTransform* trans,
    GstBuffer* inbuf, GstBuffer* outbuf) {
  GvalStft* self = GVAL_STFT(trans);

  gsize size = gst_buffer_get_size(inbuf);
  gsize offset = 0;
  gsize copied = 0;
  guint empty = 0;
  gdouble* spectra;
  guint i;

  while (size > 0) {
    if (self->skip > 0) {
      copied = MIN(size, self->skip);
      size -= copied;
      self->skip -= copied;
      offset += copied;
    }
    else {
      if (self->current >= self->start) {
        empty = self->wsize - self->current;
      }
      else {
        empty = self->start - self->current;
      }

      copied = gst_buffer_extract(inbuf, offset, self->buffer + self->current, empty * sizeof(gdouble));
      g_assert(copied > 0);
      size -= copied;
      offset += copied;
      self->current = (self->current
          + copied / sizeof(gdouble)) % self->wsize;

      if (self->current == self->start) {
        // One full window
        spectra = g_malloc_n(self->wsize * 2, sizeof(gdouble));
        for (i = 0; i < self->wsize; i++) {
          spectra[i * 2] = self->buffer[(self->start + i)
            % self->wsize] * self->window_func(i, self->wsize);
          spectra[i * 2 + 1] = 0;
        }
        gsl_fft_complex_radix2_forward(spectra, 1, self->wsize);

        // Calculate Energy
        gdouble e = 0;
        for (i = 0; i < self->wsize; i++) {
          e += sqrt(spectra[i * 2] * spectra[i * 2]
              + spectra[i * 2 + 1] * spectra[i * 2 + 1]);
        }

        if (!self->silent) {
          g_print("E = %g\n", e);
        }

        g_free(spectra);
        // Shift
        if (self->ssize <= self->wsize) {
          self->start = (self->start + self->ssize)
            % self->wsize;
        }
        else {
          self->skip = (self->ssize - self->wsize)
            * sizeof(gdouble);
        }
      }
    }
  }

  if (!self->silent) {
    /*
       g_print("Have data of size %"G_GSIZE_FORMAT" bytes. "
       "wsize=%"G_GUINT32_FORMAT", "
       "ssize=%"G_GUINT32_FORMAT"\n",
       gst_buffer_get_size(buf), self->wsize,
       self->ssize);
       gchar* padname = gst_pad_get_name(pad);
       g_print("Pad Name: %s\n", padname);
       g_free(padname);

       GstCaps* caps = gst_pad_get_current_caps(pad);
       print_caps(caps, "");
       gst_caps_unref(caps);
       caps = gst_pad_get_allowed_caps(pad);
       print_caps(caps, "");
       gst_caps_unref(caps);
     */
  }

  /* just push out the incoming buffer without touching it */
  gboolean r = gst_buffer_copy_into(outbuf, inbuf,
      GST_BUFFER_COPY_ALL, 0, -1);
  if (r) {
    return GST_FLOW_OK;
  }
  else {
    g_error("Failed to copy buffer");
    return GST_FLOW_ERROR;
  }
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean stft_init(GstPlugin* stft) {
  /* debug category for fltering log messages
   */
  GST_DEBUG_CATEGORY_INIT(gval_stft_debug, "stft",
      0, "Short Time Fourier Transform");

  return gst_element_register(stft, "stft", GST_RANK_NONE,
      GVAL_TYPE_STFT);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gval-package"
#endif

/* gstreamer looks for this structure to register stfts
 */
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    stft,
    "Short Time Fourier Transform",
    stft_init,
    VERSION,
    "GPL",
    "Unknown",
    "https://github.com/hwp/gval/"
    );
