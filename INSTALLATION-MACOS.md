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

* Apple Silicon:

  ```text
  https://github.com/GlistEngine/glistzbin-macos/releases/latest
  ```
* Intel:

  ```text
  https://github.com/GlistEngine/glistzbin-macos-x86_64/releases/latest
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


See [GETTING-STARTED.md](./GETTING-STARTED.md) for platform-specific launch instructions to get started developing.