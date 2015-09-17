gval
====

General Visual-Audio Learning

This is a C/C++ library for visual and audio feature extraction from video files.
This library is implemented as a GStreamer plugin.

>**Note**: 
>This library is designed to be used in **Linux** environment. 
>It might work in other platforms as gstreamer also support Windows and OSX. However, the library is solely tested and supported in Linux.

## Implemented Functions
* Audio Features
  * MFCC
* Visual Features
  * SIFT
  * Bag-of-words model with SIFT

## Installation Guide
### Dependency
Prior to the installation of this library, please make sure you have these dependent libraries installed and can be found by pkg-config.

* [GStreamer](http://gstreamer.freedesktop.org/) and its components:
  * gst-libav
  * gst-plugins-base
  * gst-plugins-good
  * gst-plugins-bad
  * gst-plugins-ugly
* [GLib](https://developer.gnome.org/glib/stable/) and its component:
  * [GObject](https://developer.gnome.org/gobject/stable/)
* [OpenCV](http://opencv.org/)
* [Boost](http://www.boost.org/) and its components:
  * system
  * filesystem
* [fftw3](http://www.fftw.org/)

For building the library you also need:

* [CMake](http://www.cmake.org/)

## Build and Install
The building and installation of the library is quite standard in the ``CMake way''.

To build:
```
mkdir build
cd build
cmake ..
make
```
To install:
```
make install
```
The default directory for installation is `/usr/local`. To change the installation directory
```
cmake -DCMAKE_INSTALL_PREFIX=/desired/directory ..
```

## Usage
### Check Plugin Information
Upon successful installation, you should be able to find GStreamer plugin named `gval_plugin` with `gst-inspect-1.0`. By doing
```
gst-inspect-1.0 gval_plugin
```
You can find the information about this plugin.

>**Note**:
>If GStreamer can't find the plugin, please make sure the installed library, i.e. `libgval.so` is in the GStreamer plugin path, which is normally `/usr/local/lib/gstreamer-1.0` or environment variable `GST_PLUGIN_PATH` is set properly according to the actual path. 
>For example, if `libgval.so` is installed in `~/.local/lib/gstreamer-1.0`, `GST_PLUGIN_PATH` should be set to `$HOME/.local/lib`.

### Plugin Elements
This plugin contains five elements:

| Element   | Full Name |
| --------- | --------- |
| bow       | Bag-of-words Model with SIFT descriptors |
| mfcc      | Mel-frequency cepstrum coefficients |
| keypoints | Key Points                          |
| sift      | Scale invariant feature transform   |
| stft      | Short time Fourier transform        |

