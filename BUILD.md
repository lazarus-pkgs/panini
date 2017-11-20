# Build

To build Panini with Qt Creator, open "Panini.pro" (a simple double click should work) and select "build all" or "build project" on the Build menu.  

To build from a command shell (on Windows, use the one supplied with Qt):  In the panini directory, type:
`qmake panini.pro`

On Linux or Windows that command will create Makefiles, on OSX it creates an XCode project, that you can open and build.
To get a Makefile instead, use:
`qmake -spec macx-g++`

Qt Creator issues this command, too.

The version number of panini is defined in the `.pro` file.

To build on Windows or Linux (or from a Mac Makefile) type `make release` or `make debug`.
The resulting executable will be Release/Panini or Debug/Panini on Windows or Linux; on OSX, possibly just Panini.app in the package root.

To install, just copy Panini.exe or Panini.app to wherever you like. On Windows, you may have to add the Qt4 bin directory to the "all users" search path, though that will probably have been done when Qt was installed.  This is important because that directory contains required plugin subdirectories as well as the Qt DLLs (DON'T copy Qt DLL's to Windows\System32.  Any system which requires that is too old to run Panini).  If you have a preexisting installation of MingGW, it is OK to put the MinGW DLL in System32; but if you installed MinGw with Qt, there will be a copy of the MinGW DLL in Qt4 bin.

The qmake project file, Panini.pro, is set up to build both debug and release versions (release by default, except XCode always defaults to debug).  This has some quirks which you should be aware of.  There are two subordinate Makefiles. "make release" runs one, building executable bin/panini and "make debug" runs the other, building bin/panini-d; plain "make" runs the default build.  However both builds use the same object file names; so if you run "make release" then "make debug", it may incorrectly create panini by linking the existing release object files, which lack symbol tables and so won't work with the debugger, or vice versa.  The safest thing is to "make clean" when swtiching configurations.

Qt Creator (version 4.5) will show release and debug build configurations and a single "run configuration" just called Panini.  That actually runs the exe corresponding to the current build configuration (both the "run" and the "debug" buttons do this) so to debug you have to first select the debug build.
