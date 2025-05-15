# Manual Installation on Linux

**1. Install prerequisites**

* Debian/Ubuntu:

  ```bash
  sudo apt-get install git cmake clang-14 libstdc++-12-dev libglew-dev \
    curl libcurl4-openssl-dev libssl-dev build-essential
  ```
* Fedora/RPM:

  ```bash
  sudo dnf install glm-devel glfw glfw-devel glew-devel freetype-devel \
    assimp assimp-devel curl curl-devel openssl-devel
  ```

**2. Create folders**

```bash
mkdir -p ~/dev/glist/{myglistapps,zbin}
```

**3. Go to zbin**

```bash
cd ~/dev/glist/zbin
```

**4. Download zbin**

```text
https://github.com/GlistEngine/glistzbin-linux/releases/latest
```

Extract into `~/dev/glist/zbin`

**5. Back up one level**

```bash
cd ~/dev/glist
```

**6. Clone GlistEngine**

```bash
git clone https://github.com/yourgithubusername/glistengine.git
```

**7. Clone GlistApp**

```bash
cd ~/dev/glist/myglistapps
git clone https://github.com/yourgithubusername/glistapp.git
```
