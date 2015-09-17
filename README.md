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

### Build and Install
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

## Plugin Overview
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
| bow       | [Bag-of-words Model with SIFT descriptors](#bow) |
| mfcc      | [Mel-frequency cepstrum coefficients](#mfcc) |
| keypoints | [Key Points](#keyp)                          |
| sift      | [Scale invariant feature transform](#sift)   |
| stft      | [Short time Fourier transform](#stft) |

> **Tip**:
> The information about the elements can always be checked with `gst-inspect-1.0`.

## Audio Features
### <a name="stft"></a>Short Time Fourier Transform
The element `stft` will take raw PCM audio data as input and compute the short time Fourier transform. The transform will be saved to an external file, the path of which is set by property `location`. The input data will be passed through to the output without any modification.

The properties of the element include:
| Properties | Description |
| ---------- | ----------- |
| silent     | Suppress verbose output  |
| wsize      | Window size of the FFT   |
| ssize      | Shift size of the window |
| location   | Path to the output file  |

For example, if you want to compute STFT of an audio file (file.ogg) with window size of 256 and shift size of 64 at sample rate 8000 Hz, you can do
```
gst-launch-1.0 filesrc location=
file.ogg' ! decodebin ! audioresample ! audio/x-raw,rate=8000 ! audioconvert ! stft wsize=256 ssize=64 location='result.fvec' ! fakesink
```
The result will be save to file `result.fvec` as a raw binary data file. More specifically, the result feature vectors will be store frame by frame. Each frame consists of a vector of dimension same as the window size. The values are stored in `double` format, which is 8-byte (IEEE 754) and little endian on most systems.

### <a name="mfcc"></a>Mel-frequency Cepstrum Coefficients

## Visual Features
### <a name="keyp"></a>Key Points

### <a name="sift"></a>Scale Invariant Feature Transform

### <a name="bow"></a>Bag-of-words Model with SIFT descriptor





