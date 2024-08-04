#!/bin/sh
mkdir KarmaLightHouse
cd KarmaLightHouse
# Make sure to set the build settings > signing -> Development Team and Code signing identity appropriately
cmake -DCMAKE_TOOLCHAIN_FILE=KarmaCmake/ios-cmake/ios.toolchain.cmake -GXcode -DPLATFORM=OS64COMBINED -DVulkan_LIBRARY=/usr/local/lib/libvulkan.dylib -DOPENGL_gl_LIBRARY=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.5.sdk/System/Library/Frameworks/OpenGL.framework -DOPENGL_INCLUDE_DIR=/opt/X11/include ..
