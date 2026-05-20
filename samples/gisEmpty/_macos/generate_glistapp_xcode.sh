#!/bin/sh

BINARY_DIR="$HOME/dev/glist/zbin/glistzbin-macos/mingw64/lib"
GLISTWD="$(pwd)/.."
BUILD_DIR="$GLISTWD/_build"

source ios-secret.sh

# EXPOSED VARIABLES (CAN CHANGE)
BUILD_TYPE="Debug"
PLATFORM=SIMULATOR64COMBINED #OS64, SIMULATOR64COMBINED
ARCHS=arm64
DEPLOYMENT_VERSION="13.0"

# EXPOSED VARIABLES END

export CC="/opt/homebrew/opt/llvm/bin/clang"
export CXX="/opt/homebrew/opt/llvm/bin/clang++"
export OBJC="/opt/homebrew/opt/llvm/bin/clang"
export OBJCXX="/opt/homebrew/opt/llvm/bin/clang++"

if [ $1 = "ios" ]
then
cmake -S "$GLISTWD" \
-D CMAKE_TOOLCHAIN_FILE="$GLISTWD/macos/ios.toolchain.cmake" \
-G Xcode \
-B "$GLISTWD/_build" \
-D PLATFORM=$PLATFORM \
-D DEPLOYMENT_TARGET=$DEPLOYMENT_VERSION \
-D GLIST_IOS_DEVELOPMENT_TEAM=$GLIST_IOS_DEVELOPMENT_TEAM \
-D GLIST_IOS_PRODUCT_BUNDLE_IDENTIFIER=$GLIST_IOS_PRODUCT_BUNDLE_IDENTIFIER \
-D GLIST_IOS_PRODUCT_NAME=$GLIST_IOS_PRODUCT_NAME
elif [ $1 = "macos" ]
then
cmake -S "$GLISTWD" \
-G Xcode \
-B "$GLISTWD/_build"
fi

xattr -w com.apple.quarantine "$(xattr -p com.apple.quarantine "$BINARY_DIR/libfmod.dylib" | sed 's/^.\{4\}/00c1/')" $BINARY_DIR/libfmod.dylib
xattr -w com.apple.quarantine "$(xattr -p com.apple.quarantine "$BINARY_DIR/libfmodL.dylib" | sed 's/^.\{4\}/00c1/')" $BINARY_DIR/libfmodL.dylib

if [ ! -d "$BUILD_DIR/$BUILD_TYPE" ]
then
    cd $BUILD_DIR
    mkdir $BUILD_TYPE
fi
cd $BUILD_DIR/$BUILD_TYPE
mkdir assets

cd $GLISTWD

cp -r "assets/" "_build/$BUILD_TYPE/assets/"

open _build/GlistApp.xcodeproj
