#!/bin/sh

GLISTWD="$(pwd)/.."
BUILD_DIR="$GLISTWD/_build"

source ios-secret.sh

# EXPOSED VARIABLES (CAN CHANGE)
BUILD_TYPE="Debug"
PLATFORM=SIMULATOR64COMBINED #OS64, SIMULATOR64COMBINED
ARCHS=arm64
DEPLOYMENT_VERSION="13.0"

# EXPOSED VARIABLES END

export CC="/usr/bin/clang"
export CXX="/usr/bin/clang++"
export OBJC="/usr/bin/clang"
export OBJCXX="/usr/bin/clang++"

if [ $1 = "ios" ]
then
cmake -S "$GLISTWD" \
-D CMAKE_TOOLCHAIN_FILE="$GLISTWD/_apple/ios.toolchain.cmake" \
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
