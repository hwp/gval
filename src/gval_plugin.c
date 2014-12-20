/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * gval_plugin.c : Plugin loading
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

#include "gval_utils.h"
#include "gval_stft.h"
#include "gval_mfcc.h"
#include "gval_keypoints.h"
#include "gval_sift.h"
#include "gval_bow.h"
      
static gboolean plugin_init(GstPlugin* plugin) {
  gval_debug_init();

  gboolean ret 
    = gst_element_register(plugin, "stft", GST_RANK_NONE,
      GVAL_TYPE_STFT);
  ret = gst_element_register(plugin, "sift",
      GST_RANK_NONE, GVAL_TYPE_SIFT) && ret;
  ret = gst_element_register(plugin, "keypoints",
      GST_RANK_NONE, GVAL_TYPE_KEYPOINTS) && ret;
  ret = gst_element_register(plugin, "mfcc",
      GST_RANK_NONE, GVAL_TYPE_MFCC) && ret;
  ret = gst_element_register(plugin, "bow",
      GST_RANK_NONE, GVAL_TYPE_BOW) && ret;

  return ret;
}

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    gval_plugin,
    "GVAL",
    plugin_init,
    VERSION,
    "GPL",
    PACKAGE_NAME,
    GST_PACKAGE_ORIGIN
    );

