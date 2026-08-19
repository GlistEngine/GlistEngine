# Manual Installation on macOS

**1. Install Xcode**

You can install Xcode via App Store, after the installation complete, open it and accept the end user agreement. You might need accept this agreement after some macOS updates.

**2. Install Homebrew**

Homebrew is a package-manager that helps you manage packages/libraries, GlistEngine uses brew for some of it's dependencies. Run the following command in a terminal to install it.

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**3. Install libs & config**


```bash
brew install git openssl@1.1 libomp llvm cmake glew glfw glm freetype assimp curl git wget pkg-config
sudo spctl --master-disable
```

**4. Create folders**

```bash
mkdir -p ~/dev/glist/{myglistapps,zbin}
```

**5. Go to zbin**

```bash
cd ~/dev/glist/zbin
```

**6. Download zbin**

Apple Silicon or Intel:

```text
https://github.com/GlistEngine/glistzbin-macos/releases/latest
```

Extract into `~/dev/glist/zbin`

**7. Back up one level**

```bash
cd ~/dev/glist
```

**8. Clone GlistEngine**

```bash
git clone https://github.com/yourgithubusername/glistengine.git
```

**9. Clone GlistApp**

```bash
cd ~/dev/glist/myglistapps
git clone https://github.com/yourgithubusername/glistapp.git
```


## Vulkan renderer (optional)

The engine renders through OpenGL unless an app asks for Vulkan, and the steps
above are all an OpenGL app needs. To build the Vulkan backend as well:

```bash
brew install vulkan-loader molten-vk
```

macOS has no Vulkan driver of its own - the loader hands the calls to MoltenVK,
which translates them to Metal. If they are missing CMake says so during the
configure step and builds the engine without the backend:

```text
-- Vulkan development files not found; Vulkan backend disabled
```

Two more packages are worth having while developing, neither of which is needed
to build or run:

```bash
brew install vulkan-validationlayers shaderc
```

`vulkan-validationlayers` is loaded by Debug builds only and reports API misuse
that otherwise shows up as a blank screen or a driver crash. `shaderc` provides
`glslc`, which regenerates the compiled shaders whenever you edit a file under
`engine/graphics/shaders/vk`; without it the committed shaders are used as they
are, so it only matters if you change them.

An app selects the renderer where it passes the render engine in `main.cpp`:
`G_RENDERER_VK` instead of `G_RENDERER_GL`. A build without Vulkan falls back to
OpenGL by itself, so the same source works either way.

For mobile: Android takes its Vulkan loader from the NDK and needs nothing
installed, and iOS builds against a MoltenVK that ships inside the gipIOS plugin
- see that plugin's README for the details of both.

See [GETTING-STARTED.md](./GETTING-STARTED.md) for platform-specific launch instructions to get started developing.
