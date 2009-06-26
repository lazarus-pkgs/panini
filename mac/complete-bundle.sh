# complete-bundle.sh, May 2009, Harry van der Wolf
# This shell script is neccessary to create a complete 
# and portable Panini for MacOSX 10.4 and newer.
# Version 0.3, 2009-06-20

####################################################################
# You should not need to modify anything below these comment lines #
####################################################################
App_path=./build/Release/Panini.app
Basic_path=./mac
FrameW_path=$App_path/Contents/Frameworks
plugins_path=$App_path/Contents/plugins
Resources_path=$App_path/Contents/Resources

# Copy QT frameworks into bundle
mkdir -p $App_path/Contents/Frameworks
rsync -av --exclude 'Headers/*' /Library/Frameworks/QtCore.framework $FrameW_path
rsync -av --exclude 'Headers/*' /Library/Frameworks/QtGui.framework $FrameW_path
rsync -av --exclude 'Headers/*' /Library/Frameworks/QtOpenGL.framework $FrameW_path

# Copy QT plugins and config
mkdir -p $plugins_path
mkdir -p $plugins_path/imageformats
#rsync -av /Developer/Applications/Qt/plugins/imageformats $plugins_path
cp -f /Developer/Applications/Qt/plugins/imageformats/libqjpeg.dylib $plugins_path/imageformats
cp -f /Developer/Applications/Qt/plugins/imageformats/libqtiff.dylib $plugins_path/imageformats
cp -f /Developer/Applications/Qt/plugins/imageformats/libqmng.dylib $plugins_path/imageformats
mkdir -p $Resources_path
cp $Basic_path/qt.conf $Resources_path


# Copy applications icons
cp $Basic_path/appIcon.icns $Resources_path

# Do the necessary install_name changes
install_name_tool -id "@executable_path/../Frameworks/QtCore.framework/QtCore" "$FrameW_path/QtCore.framework/QtCore"
install_name_tool -id "@executable_path/../Frameworks/QtGui.framework/QtGui" "$FrameW_path/QtGui.framework/QtGui"
install_name_tool -id "@executable_path/../Frameworks/QtOpenGL.framework/QtOpenGL" "$FrameW_path/QtOpenGL.framework/QtOpenGL"

binaries="$FrameW_path/QtCore.framework/QtCore $FrameW_path/QtOpenGL.framework/QtOpenGL $FrameW_path/QtGui.framework/QtGui $App_path/Contents/MacOS/*"
# $plugins_path/imageformats/*.dylib
#binaries="$App_path/Contents/MacOS/*"
#old_install_name_dirname="Qt"
new_install_name_dirname="@executable_path/../Frameworks"

#First the Frameworks
for exec_file in $binaries
do
# for lib in $(otool -L $exec_file | grep Qt  | sed -e 's/ (.*$//' -e 's/^.*\///')
 for lib in $(otool -L $exec_file | grep Qt)
 do
  echo " Changing install name for: $lib"
  install_name_tool -change "$lib" "$new_install_name_dirname/$lib" $exec_file
 done
done
#Now the plugins
binaries="$plugins_path/imageformats/*.dylib"
for exec_file in $binaries
do
 for lib in $(otool -L $exec_file | grep Qt)
 do
  echo " Changing install name for: $lib"
  install_name_tool -change "$lib" "$new_install_name_dirname/$lib" $exec_file
 done
done




