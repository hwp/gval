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

#include "gval_stft.h"
#include "gval_utils.h"

#include <math.h>
#include <gst/gst.h>

GST_DEBUG_CATEGORY_STATIC(gval_stft_debug);
#define GST_CAT_DEFAULT gval_stft_debug

#define MIN_WINDOW_SIZE 4
#define MAX_WINDOW_SIZE 65536
#define DEFAULT_WINDOW_SIZE 1024

#define MIN_SHIFT_SIZE 1
#define MAX_SHIFT_SIZE 65536
#define DEFAULT_SHIFT_SIZE 256

enum {
  PROP_0,

  PROP_SILENT,
  PROP_WSIZE,
  PROP_SSIZE,
  PROP_LOCATION,

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
    );

#define gval_stft_parent_class parent_class
G_DEFINE_TYPE(GvalStft, gval_stft, GST_TYPE_AUDIO_FILTER);

static void gval_stft_dispose(GObject* object);
static void gval_stft_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec);
static void gval_stft_get_property(GObject* object, guint prop_id,
    GValue* value, GParamSpec* pspec);

static gboolean gval_stft_setup(GstAudioFilter* filter,
    const GstAudioInfo* info);
static GstFlowReturn gval_stft_transform_ip(GstBaseTransform* trans,
    GstBuffer* inbuf);

/* initialize the stft's class */
static void gval_stft_class_init(GvalStftClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  GstElementClass* gstelement_class
    = GST_ELEMENT_CLASS(klass);
  GstBaseTransformClass* base_transform_class
    = GST_BASE_TRANSFORM_CLASS (klass);
  GstAudioFilterClass* audio_filter_class
    = GST_AUDIO_FILTER_CLASS(klass);

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
  stft_props[PROP_LOCATION] = g_param_spec_string(
      "location", "Location", "Path to the output file",
      NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties(gobject_class,
      N_PROPERTIES, stft_props);

  gst_element_class_set_details_simple(gstelement_class,
      "stft", "gval/feature",
      "Short Time Fourier Transform",
      "Weipeng He <heweipeng@gmail.com>");

  gst_element_class_add_pad_template(gstelement_class,
      gst_static_pad_template_get(&src_factory));
  gst_element_class_add_pad_template(gstelement_class,
      gst_static_pad_template_get(&sink_factory));

  audio_filter_class->setup 
    = GST_DEBUG_FUNCPTR(gval_stft_setup);
  base_transform_class->transform_ip
    = GST_DEBUG_FUNCPTR(gval_stft_transform_ip);
}

static void gval_stft_init(GvalStft* this) {
  this->silent = FALSE;
  this->wsize = 0;
  this->ssize = 0;
  this->location = NULL;

  this->adapter = gst_adapter_new();
  this->skip = 0;
  this->window_func = gval_hann_window;
  this->out = NULL;
}

static void gval_stft_dispose(GObject* object) {
  GvalStft* this = GVAL_STFT(object);
  g_object_unref(this->adapter);
  if (this->out) {
    fclose(this->out);
  }

  G_OBJECT_CLASS(gval_stft_parent_class)->dispose(object);
}

static void gval_stft_set_property(GObject* object,
    guint prop_id, const GValue* value, GParamSpec* pspec) {
  GvalStft* this = GVAL_STFT(object);

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
    case PROP_LOCATION:
      this->location = g_value_dup_string(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void gval_stft_get_property(GObject* object,
    guint prop_id, GValue* value, GParamSpec* pspec) {
  GvalStft* this = GVAL_STFT(object);

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
    case PROP_LOCATION:
      g_value_set_string(value, this->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

static gboolean gval_stft_setup (GstAudioFilter* filter,
    const GstAudioInfo* info) {
  GvalStft* this = GVAL_STFT(filter);

  if (!this->silent) {
    printf("Sample Rate : %d\n", info->rate);
    printf("Window Size: %d\n", this->wsize);
    printf("Shift Size: %d\n", this->ssize);
  }

  return TRUE;
}

static GstFlowReturn gval_stft_transform_ip(GstBaseTransform* trans,
    GstBuffer* buf) {
  GvalStft* this = GVAL_STFT(trans);

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
    gdouble* spectra = g_malloc_n(this->wsize / 2 + 1,
        sizeof(gdouble));
    gval_spectrum(spectra, data, this->wsize,
        this->window_func);
    gst_adapter_unmap(this->adapter);

    // Write to file
    if (this->location) {
      if (!this->out) {
        this->out = fopen(this->location, "w");
      }
      g_assert(this->out);
      fwrite(spectra, sizeof(double), this->wsize / 2 + 1,
          this->out);
    }

    g_free(spectra);

    // Shift
    skipped = MIN(this->ssize * sizeof(gdouble), avail);
    gst_adapter_flush(this->adapter, skipped);
    avail -= skipped;
    this->skip = this->ssize * sizeof(gdouble) - skipped;
  }

  return GST_FLOW_OK;
}

