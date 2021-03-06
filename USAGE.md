# Usage

Panini is a tool for creating perspective views from panoramic and wide angle images. 
 
Panini can load most common photo and panoramic formats from image files or QuickTime VR (.mov) files.  Like all pano viewers, it then shows a linear perspective view that can be panned and zoomed.  But Panini can also display a range of wide angle perspectives via the stereographic and "Pannini" families of projections, and shift, rotate, and stretch the image like a software view camera.  

Panini can do those things because it paints the picture on a three dimensional surface, either a sphere or a cylinder, which you then view in perspective.  Shifting the point of view changes the apparent perspective of the image, and other controls let you frame the view to your liking.  Then you can save the screen image to a file at higher-than-screen resolution.

All of this works interactively at "mouse speed" thanks to the OpenGL video graphics system.  And thanks to the Qt application framework, it runs on Windows, Mac OSX, and most flavors of Linux and Unix.

## Why "Panini"?

The name honors 18th century artist and professor of perspective Gian Paolo Pannini (also spelled "Panini"), who painted grand views of Rome and taught a generation of artists that included Canaletto and Piranesi.  Their impressive wide angle views of building interiors and urban landscapes were famous throughout Europe.  These pictures seem to be in true perspective, but can't be: the standard linear perspective projection creates serious distortions at much smaller view angles.  Instead, they used clever combinations of cylindrical and linear projections.  These techniques were probably invented in Holland a century or more earlier, which is one reason why Panini's icon is a picture of the Erasmus Bridge in Rotterdam.

Bruno Postle recently deduced one very simple but effective way of combining the cylindrical and linear projections by studying a Pannini painting of St. Peter's; so we are calling that the "Pannini projection".  It is a linear perspective view of a cylindrical image -- the cylindrical analog of the stereographic projection of a sphere.  The Baroque artists may or may not have used this construction explictly, but it produces images that look a lot like some of the ones they drew.

# System requirements and limitations

To run Panini you need several Qt runtime libraries.

To display pictures Panini uses OpenGL, a low level graphics API that is tightly integrated with the system's video drivers.  OpenGL cannot be installed as a separate piece of software.  If the version of your OpenGL implementation is less than 1.2 or does not support cubic texture mapping, Panini will not run.  If the OpenGL version is less than 1.5 it may not be able to display all pictures correctly.

You will get best results with OpenGL version 2.0 and above.  That means you should have either a "gaming" video card, or a recent "multimedia" one.  But ultimate gaming performance is not needed; a basic version 2 system should be adequate.  Cards to upgrade most desktop PCs to that level now cost under $50.  

The "About" dialog shows information about your system's OpenGL facilities as well as the version of Panini you are running.
It reports these limits:
```
  texPwr2:	must texture dimensions be powers of 2? 0 (false) preferred.
  texMax:	maximum dimension of 2D textures, the bigger the better.
  cubeMax:	maximum dimension of cubic textures, the bigger the better.
```
  			
The maximum resolution Panini can display or render depends on the amount of texture memory available.  Cubic images generally support somewhat larger texture maps than the flat formats.  Saved image resolution is normally several times screen resolution, however that too may be limited by OpenGL resources.  If your OpenGL does not support offscreen frame buffers of arbitrary size, all saved views will be at screen resolution.  

Panini places no special demands on CPU speed or memory; if you can stitch panoramas, you will have no trouble viewing them.

# Basic viewing controls

Most viewing controls have keyboard shortcuts, shown in the *View* menu.  You can operate all viewing controls with the mouse.  Help menu item *mouse modes...* shows how.

When you start Panini without a command line argument, for example by double clicking an icon, it displays a wire frame model of the panosphere (you can switch to the panocylinder by clicking the button that shows the panosurface name).  Try out the basic viewing controls on these wire models.

The *Yaw*, *Pitch* and *Roll* controls rotate your view with respect to the panosurface, as in a flight simulator. Yaw and Pitch let you look in different directions; Roll adjusts which way is up, and changes the directions of the Yaw and Pitch axes accordingly. The status line shows these angles as "Y", "P", and "R", in degrees.

To control Yaw and Pitch with the mouse, hold the left button and move the pointer in the direction you wish to look.  Or you can hold both mouse buttons and "fly" using Roll and Pitch.

Zoom controls magnification by changing the vertical field of view of the screen window.  When you resize the window, the displayed image grows or shrinks to maintain the selected vertical field.  The mouse wheel (if present) adjusts zoom in coarse steps.  For finer control, move the mouse vertically while holding the right button.  Zoom is reported on the status line as "V", in degrees.  This is the vertical field of view on the panosphere when eye distance is <= 1, but has a more complicated relationship to visible vfov at larger eye distances and on the panocylinder.

"Eye in" and "Eye out" change perspective by moving the point of view toward or away from the center of the panosurface.  The basic effect of moving the eye out is to compress the periphery of the image.  With the eye at the center, the viewing perspective is linear, like a normal camera lens or pano viewer.  As the eye moves out, the perspective becomes more fisheye-like, and you can zoom farther out to see wider views.  But you can zoom in to the same level of image detail at any perspective setting.  When the right button is held, horizontal mouse motion controls eye distance. Eye distance is reported on the status line as "D" in units of the panosurface radius.

With the panosphere, the perspective projections are members of a family whose protoype (at Ez = 1) is the stereographic projection.  With the panocylinder, the projections are members of a family whose prototype is the recently rediscovered "Panini projection", a combination of cylindrical and linear projections.

The linear perspective projection, at Ez = 0, belongs to both families.  As eye distance increases past 1, the viewing projection becomes more nearly parallel, and at Ez = 26, it is very close to the orthographic spherical or cylindrical projection.

Becase the effect of Ez on the image is quite nonlinear, the controls that adjust it have a compensating nonlinear response built in.

The following view controls, new in version 0.6, can only be operated with the mouse.

To move the eye point perpendicular to the projection axis, hold down the Shift key and both mouse buttons, and move the mouse toward the desired eye position.  The effects are similar to "swing" and "tilt" on a view camera.  Eye position is reported on the status line as "Ex" and "Ey" in units of the panosurface radius.  "Shift-Home" resets the eyepoint shifts to zero, as does "End".

To move the whole view horizontally or vertically without changing its size or shape, hold down the Shift key and the left mouse button.  These "framing shifts" are reported on the status line as Fx and Fy.  "Shift-Home" or "End" resets them to zero.
  
# Presets

Several preset views are available via single keystrokes.  "Home" resets the yaw, pitch and roll angles to zero.  "Shift + Home" resets the eyepoint and framing shifts. "End" resets everything, restoring the standard view you got when the image was first loaded.  "F" gives the widest possible full frame view (a stereographic or Panini projection) and "S" gives a "super wide" view, obtained by moving the eye point slightly outside the panosurface.

The Turn controls dialog ("Turn image" in preset menu, or T key) lets you rotate the image on the panosurface.  For cubic panoramas this is a full 3D rotation, including Yaw and Pitch angles, so you can align the axis of the panocylinder in any direction in space.  For non-cubic sources, only the Roll angle (around source center) can be adjusted.  The "orientation" and "Roll" controls both affect this angle, orientation in steps of 90 degrees.  The Turn angles are reset by "Alt+End" but not by "End".  

Hint: The main use of Turn is to align the cylinder axis with lines that you want to keep straight in the Pannini projections.  The projection axes do not turn along with the image, however you can get rather messy "transverse" forms of some projections by adjusting the hfov and vfov.  A future release will have proper transverse projections.

For convenience in viewing series of related pictures, Panini remembers the last selected Turn angles and FOV for each source format (but version 0.6 does not save them when you exit the program).

Version 0.63 does save a few items at exit, and restore them at startup.  One is a limit on the displayed size of cube faces, effective only on Macs, to work around a bug in OSX that can cause system crashes as well as garbled display of cubic images.  You can see and change this limit via the "cube limit" item on the Presets menu.  The default limit will probably work for you, but if it gives bad displays of large cubic QTVRs, reduce it until they are displayed correctly.  Otherwise you may want to increase the limit until you find the largest size your system will display comfortably (note that the source cube face size also limits the displayed size).

Version 0.63 also saves and restores the window size.  The initial default size is smaller than before so that the window will not overflow small laptop screens, which can be awkward to correct on some systems. 

# Loading a source image

You can load an image into Panini by naming it on the command line, by selecting it with a file browser (after choosing a format from the "Source" menu) or by dragging it into the Panini window.  Details below.

To show a picture, Panini needs to know the format (projection), angular size (field of view, FOV), image dimensions and where to find the image data.  All supported sources supply image dimensions and data, and some also define projection and angular size; but in many cases you must specify the projection and field of view yourself.

You do this in a dialog that shows the file name, projection, dimensions in pixels, and horizontal and vertical fields of view in degrees.  If you have not already chosen a format, you can do that; and in all cases you can enter the field of view.

You can specify either the horizontal or vertical field of view.  They are coupled via the dimensions and projection, so that when you change one the other changes to match.  You can uncouple the FOVs when necessary, for example to load an image with non-square pixels, or one with a "transverse" projection, by checking the box.

When you select a different projection, the last FOV used for that projection is recalled.  If it does not match the current image's dimensions, the larger FOV is kept and the other is adjusted.  

Each projection has a maximum FOV, that you can not exceed.  These limits are generous, and may let you specify an FOV larger than the maximum that can actually be displayed.  In that case the image will be projected according to the given FOV, but cropped to the display limit.

When an image is loaded, the Panini window's title bar shows the file name and a size in megapixels, which will almost always be smaller than the source image size.  This is not the size of the view on screen, but of the texture image used to generate it.  In case loading or display fails, the title bar will show an error message instead.

You can load images by naming them on the command line, by selecting them via the Source menu, or by dragging them into the Panini window.

## via Command Line

When you start Panini from the command line, you can optionally specify an image to be displayed.  If you give only a partial specification, Panini will ask you for the rest of the information.

The first command line argument can be a 4 letter format name, one of:

```
	proj	PanoTools script or project
	qtvr	QuickTime VR panorama (cubic or cylindrical)
	cube	1 to 6 linear cube face images 
	rect	Rectilinear (normal lens) projection 
	fish	Fisheye lens or mirror ball projection
	sphr	Equal angle spherical projection
	ster	Stereographic (super wide) projection
	cyli	Cylindrical panorama
	equi	Equirectangular panorama
	merc	Mercator panorama
```
	
The first 3 names are rarely needed, because project and qtvr files are normally recognized by their name extensions, and we usually give multiple file names for a cubic format.

For the other formats, which imply specific input projections, you need to specify the angular size; so the next argument can be a field of view, in degrees, as a floating point number.  This is always the fov of the longer image axis, if the image is not square.  

The last thing on the command line can be a file name, or up to six file names for a cubic panorama.  Panini can read tiff, jpeg and png image files, identified by the conventional file name extensions.  

If you give only an image file name or names on the command line, Panini will assume that two or more square images are cube faces (to load a single cube face you must give the type name) and that an image file whose width is exactly twice its height is a 360 degree equirectangular panorama.  Otherwise it will ask you to specify the projection and FOV.  On Windows and Mac, dragging files onto the Panini program icon generates a files-only command line.

If the command line is only a format name (other than proj or qtvr) Panini displays  labelled empty frames (just a "front" frame for most formats; or six cube faces).

## via Source Menu

The Source menu lets you select a format, then Panini asks for files and fov. If you cancel the file selector dialog, empty frames will be displayed.  If you cancel the fov dialog, the selected file will not be loaded and the previous picture will remain.

You can choose "none" to get back the wire frame views of the panosurfaces.

The "rect" format is appropriate for normal photos.  You can also use it to display any kind of image that you want to see all at once.  Hint: you can then do serious linear "perspective correction", in the Photoshop sense, with the Yaw and Pitch controls.

The "fish" format uses the equisolid angle (also known as mirror sphere) projection, a classic panoramic format that is approximated by most modern fisheye lenses.  So you can use "fish" to display mirror sphere photos and computed panoramas up to 360 degrees in diameter.

The "sphr" format uses the equal-angle spherical projection.  Most stitchers can generate this projection, and some older or more technical fisheye lenses approximate it.

You can load 1 to 6 image files for the "cube" format.  They must be square images (but need not be the same size).  Their sorted names must fall in the conventional cube face order: front, right, back, left, top, bottom.  So if you have names like foo_front.tif, foo_right.tif,..., you would have to change them to something like foo_1.tif, foo_2.tif,....  If you give fewer than 6 cube face files, Panini will display empty frames for the missing ones.  Nonexistent or unreadable files will also generate empty frames.

But the easy way to load cube faces is...

## via Drag-and-Drop

You can load any kind of image by dragging it into the Panini window.  If a cubic image (or empty cube faces) is currently displayed, and the dropped image is square, it will be put in the cube face on which it was dropped, replacing any image already there.  Otherwise, dropped files are handled as described for files named on the command line.  

# Projections

A photographic image is a projection of part of the world, as seen from a single point, onto a flat plane, according to some mathematical rule.  That is also true of most images created by photo processing software, which however provides a much bigger choice of projection rules than camera lenses can.  For mathematical purposes, the original view of the world is equivalent to its linear projection onto a spherical surface, so the various photographic projections are considered as projections of the sphere onto the image plane.  Most were known to map makers and astronomers long before the invention of photography.  

The first job of a panorama viewer is to undo the source image projection, in effect recreating the spherical image, so that you see the orignal view of the world as the camera saw it.  Panini literally "de-projects" source images onto a sphere or cylinder.  
 
Each image is assigned a projection when it is loaded.  If that really is the projection used to make the image, then Panini can show you a "correct" world image, as well as various perspective transformations of that.   If a different projection was used, then none of the views displayed by Panini would be "correct" -- but some of them might be interesting or useful images anyhow.  

In fact, all source projections except the cubic are interchangeable -- Panini will happily display the same image according to any of them, and you can cycle through them without reloading the image by clicking the button at the bottom of the window (Shift-double-click also cycles through the projections).	The "End" key restores the projection assigned when the image was loaded.  

The current source projection is shown on the status bar.  Please remember that this is not the projection you are viewing, but the assumed projection of the input image.

# Field of view

The angular size, or field of view of an image determines how much of the panosurface it covers.  Each projection has a maximum displayable FOV, however you can specify a different FOV for any non-cubic image.  
An image is assigned a horizontal and a vertical field of view when loaded.  But for non-cubic formats, you can adjust the apparent fields of view afterward, using the hFov and vFov controls in the View menu (also Shift-right-mouse).  These controls change the apparent horizontal and vertical FOVs independently, within a range of 10% to 100% of the displayable FOV for the current projection.  The "End" key restores the assigned FOVs and projection.

Changing an FOV changes not only the apparent width or height of the picture but also the intrinsic perspective, because the image displayed through a different part of the projection function.

The apparent horizontal and vertical FOVs, shown in the status bar, also change when you select a different input projection, since each projection has different relationships of image dimension to angular size.  Hint: don't worry about the "true" fovs; adjust them so the view looks right.

# Saving views

You can save the current view to a jpeg image file at any time ("Save as..." in View menu, or Ctrl-S).  This is an exact copy of the displayed view, with the resolution increased 2.5 times (5.25 saved pixels for each screen pixel) if possible, typically giving a 3 to 8 megapixel image suitable for proof printing.  If your OpenGL does not support offscreen rendering buffers of arbitrary size, the filed view will be at screen resolution instead.  You can control the size and shape of the saved image by resizing the screen window, and center it in the frame with Shift-left mouse.
