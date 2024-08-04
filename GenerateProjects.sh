#!/bin/sh
mkdir KarmaLightHouse
cd KarmaLightHouse
cmake -DCMAKE_TOOLCHAIN_FILE=KarmaCmake/ios-cmake/ios.toolchain.cmake -GXcode -DPLATFORM=OS64COMBINED -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=RaviMohan -DVulkan_LIBRARY=/usr/local/lib/libvulkan.dylib -DOPENGL_gl_LIBRARY=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.5.sdk/System/Library/Frameworks/OpenGL.framework -DOPENGL_INCLUDE_DIR=/opt/X11/include ..