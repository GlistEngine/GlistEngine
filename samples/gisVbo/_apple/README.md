# GlistApp — macOS & iOS

This folder contains the build tooling for generating an Xcode project for GlistEngine on macOS and iOS.

## Prerequisites

1. Install [Xcode](https://apps.apple.com/app/xcode/id497799835) and its command line tools (`xcode-select --install`).
2. Install [CMake](https://cmake.org/download/) (`brew install cmake`).
3. Clone this repository under your GlistEngine installation directory, so the layout is:
   - `glist/myglistapps/GlistApp` (this repo)
   - `glist/GlistEngine` (the engine)
4. For **iOS only**, clone the [gipIOS](https://github.com/GlistPlugins/gipIOS) plugin into `glist/glistplugins/gipIOS`. See its README for details.

## Building for macOS

1. Open a terminal and navigate into this folder:
   ```sh
   cd glist/myglistapps/GlistApp/_apple
   ```
2. Generate the Xcode project:
   ```sh
   sh generate_glistapp_xcode.sh macos
   ```
   The script writes the project to `../_build`, copies the `assets/` folder, and opens `GlistApp.xcodeproj` in Xcode.
3. In Xcode, next to `ALL_BUILD > My Mac` at the top, click `ALL_BUILD` and select `GlistApp`.
4. Press the Run button (top-left).

## Building for iOS

1. Make sure the `gipIOS` plugin is cloned (see Prerequisites). It is added automatically for iOS builds — you do not need to list it in `PLUGINS`.
2. Fill in your signing details in `ios-secret.sh`:
   ```sh
   GLIST_IOS_DEVELOPMENT_TEAM=YOUR_TEAM_ID
   GLIST_IOS_PRODUCT_BUNDLE_IDENTIFIER=com.yourcompany.glistapp
   GLIST_IOS_PRODUCT_NAME=GlistApp
   ```
   Your Team ID is found in your [Apple Developer account](https://developer.apple.com/account) under Membership.
3. From this folder, generate the project:
   ```sh
   cd glist/myglistapps/GlistApp/_apple
   sh generate_glistapp_xcode.sh ios
   ```
   By default this targets the iOS Simulator (`SIMULATOR64COMBINED`). To build for a physical device, edit `generate_glistapp_xcode.sh` and set `PLATFORM=OS64`.
4. In Xcode, select the `GlistApp` scheme and a target device/simulator, then press Run.

## Configuration reference

The exposed variables at the top of `generate_glistapp_xcode.sh` can be changed:

| Variable | Default | Description |
| --- | --- | --- |
| `BUILD_TYPE` | `Debug` | Build configuration folder used for assets. |
| `PLATFORM` | `SIMULATOR64COMBINED` | iOS target. Use `OS64` for physical devices. |
| `ARCHS` | `arm64` | Target architecture(s). |
| `DEPLOYMENT_VERSION` | `13.0` | Minimum iOS deployment target. |

## Troubleshooting

### `Cannot find source file: .../macos/GlistApp.storyboard`

The storyboard and `Info.plist` live in this `_apple` folder. If CMake looks for them under a plain `macos/` folder, the paths in the root `CMakeLists.txt` are out of date — they should point at `_apple/GlistApp.storyboard` and `_apple/Info.plist`.

### `The variable GLIST_IOS_DEVELOPMENT_TEAM was not supplied!`

`ios-secret.sh` is missing a value. Fill in all three variables as shown in the iOS section above.

### `mkdir: assets: File exists`

Harmless — the script pre-creates the assets folder in `_build/Debug`. It does not stop the build.
