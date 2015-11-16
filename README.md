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
The `stft` element will take raw PCM audio data as input and compute the short time Fourier transform. The transform will be saved to an external file, the path of which is set by property `location`. The input data will be passed through to the output without any modification.

The properties of the element include:

| Properties | Description |
| ---------- | ----------- |
| silent     | Suppress verbose output  |
| wsize      | Window size of the FFT   |
| ssize      | Shift size of the window |
| location   | Path to the output file  |

For example, if you want to compute STFT of an audio file (file.ogg) with window size of 256 and shift size of 64 at sample rate 8000 Hz, you can do
```
gst-launch-1.0 filesrc location='file.ogg' ! decodebin ! audioresample ! audio/x-raw,rate=8000 ! audioconvert ! stft wsize=256 ssize=64 location='result.fvec' ! fakesink
```
The result will be save to file `result.fvec` as a raw binary data file. More specifically, the result feature vectors will be stored frame by frame. Each frame consists of a vector of dimension same as the window size. The values are stored in `double` format, which is 8-byte (IEEE 754) and little endian on most systems.

### <a name="mfcc"></a>Mel-frequency Cepstrum Coefficients
The `mfcc` element will take raw PCM audio data as input and compute the mel-frequency cepstrum coefficients. The result will be saved to an external file, the path of which is set by property `location`. The input data will be passed through to the output without any modification.

The properties of the element include:

| Properties | Description |
| ---------- | ----------- |
| silent     | Suppress verbose output     |
| wsize      | Window size of the FFT      |
| ssize      | Shift size of the window    |
| banks      | Number of filter banks      |
| cbegin     | Begin index of coefficients |
| csize      | Number of coefficients      |
| location   | Path to the output file     |

For example, if you want to compute MFCC of an audio file (file.ogg) with window size of 256, shift size of 64, 32 filter banks (taking the second to the seventeenth coefficients) at sample rate 8000 Hz, you can do
```
gst-launch-1.0 filesrc location='file.ogg' ! decodebin ! audioresample ! audio/x-raw,rate=8000 ! audioconvert ! mfcc wsize=256 ssize=64 banks=32 cbegin=1 csize=16 location='result.fvec' ! fakesink
```
The result will be save to file `result.fvec` as a raw binary data file. More specifically, the result feature vectors will be stored frame by frame. Each frame consists of a vector of dimension same as the number of coefficients. The values are stored in `double` format, which is 8-byte (IEEE 754) and little endian on most systems.

## Visual Features
### <a name="keyp"></a>Key Points
The `keypoints` element plots the difference of Gaussians (DoG) keypoints to a video stream.

For example, the following command will read a video file (file.avi) and show the DoG keypoints.
```
gst-launch-1.0 filesrc location='file.avi' ! avidemux name='demux' demux.video_0 ! decodebin ! videoconvert ! keypoints ! videoconvert ! autovideosink
```

### <a name="sift"></a>Scale Invariant Feature Transform
The `sift` element computes the SIFT descriptors of all DoG keypoints from a video stream.

The properties of the element include:

| Properties | Description |
| ---------- | ----------- |
| silent     | Suppress verbose output           |
| mscale     | Minimal scale of keypoint (pixel) |
| location   | Path to the output file           |

For example, the following command compute the SIFT descriptors in the video file (file.avi) and save the result to `result.desc`.
```
gst-launch-1.0 filesrc location='file.avi' ! avidemux name='demux' demux.video_0 ! decodebin ! videoconvert ! sift location='result.desc' ! fakesink
```
The result is saved frame by frame. Each frame is saved as a `cvmat` which should be read by `gval_cvmat_read`. The pointer read can be directly converted to the OpenCV object `Mat`.

### <a name="bow"></a>Bag-of-words Model with SIFT descriptor
The `bow` element computes the Bag-of-words Model with SIFT descriptors from a video stream.

The properties of the element include:

| Properties | Description |
| ---------- | ----------- |
| silent     | Suppress verbose output           |
| mscale     | Minimal scale of keypoint (pixel) |
| vocabulary | BoW vocabulary file               |
| nstop      | Number of words to be ignored     |
| location   | Path to the output file           |

For example, the following command compute the BoW features in the video file (file.avi) with a vocabulary file (a.voc) and save the result to `result.fvec`.
```
gst-launch-1.0 filesrc location='file.avi' ! avidemux name='demux' demux.video_0 ! decodebin ! videoconvert ! bow vocabulary='a.voc' location='result.fvec' ! fakesink
```
The result is saved frame by frame. Each frame is saved as a double vector of dimensions same as the size of the vocabulary. 

The vocabulary can be built by using program `build_voc`:
```
build_voc -k n_cluster input_dir output
```
The program read all files with `.desc` extension and save the result to `output`. The number of clusters is specified by the `-k` option.

