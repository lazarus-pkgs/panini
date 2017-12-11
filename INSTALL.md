# Installation

If you want to compile from source please read [BUILD](BUILD.md).

## Binary packages
Binary packages of Panini built by [OBS](http://openbuildservice.org/) can be downloaded [here](https://software.opensuse.org/download.html?project=home:jubalh:panini&package=panini).

Currently we are building packages for:
* CentOS 7
* Fedora 27
* Fedora Rawhide
* SLE 12 SP3
* openSUSE Leap 42.3
* openSUSE Tumblweed

To install it on openSUSE you can also type:
```
zypper addrepo -f obs://home:jubalh:panini panini
zypper refresh
zypper in panini
```

## AppImage
An easy way to run Panini on any Linux distribution without installation is to use the AppImage.

* Download it from the [release](https://github.com/lazarus-pkgs/panini/releases) page.
* Make it executable (`chmod u+x panini-0.72-x86_64.AppImage` or right click -> properties -> permissions)
* Click on it
