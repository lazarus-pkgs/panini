# panini

Panini is a visual tool for creating perspective views from panoramic and wide angle photographs. More than a pano viewer, more than a view camera, with features of both. For Linux/Unix, Win32, and Mac systems with OpenGL 2.0+.

## Installation
See [INSTALL](INSTALL.md).

## Dependencies

* Qt6 (specifically `Core`, `Gui`, `Widgets`, `OpenGL`, and `OpenGLWidgets`)
* zlib

## Get source code

```bash
git clone https://github.com/lazarus-pkgs/panini
cd panini
```

## Compile

```bash
qmake6 panini.pro
make
./panini
```

Also see: [BUILD](BUILD.md).

## Troubleshooting / Wayland Compatibility

If you are running on a Wayland-based desktop session and encounter dialog rendering or window management issues with Qt6, you can force the application to run via XWayland (X11 compatibility layer) by setting the `QT_QPA_PLATFORM` environment variable:

```bash
QT_QPA_PLATFORM=xcb ./panini
```

## Keyboard Shortcuts

Panini supports the following standard keyboard shortcuts for file browsing within the same folder:

* **Page Down**: Go to the **next** image file in the current directory.
* **Page Up**: Go to the **previous** image file in the current directory.
