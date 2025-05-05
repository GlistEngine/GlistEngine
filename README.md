# GLIST ENGINE

Glist Engine is a cross-platform OpenGL game engine written in C++.
Supports OpenGL 3.3 & OpenGL ES 3.0 on Windows, macOS, Linux, Android & iOS (WebGL coming soon).

**Repositories**  
- **GlistApp** – starter template for your new project  
- **GlistEngine** – core engine, plugins & samples (this repo)  
- **GlistZbin** – prebuilt libs, headers & preconfigured IDE  


More information:  
- Website: https://www.glistengine.com  
- Docs: https://glistengine.github.io/GlistEngine/  

## Installation

### I. Installation For Students

Glist Engine has one-click installer for Windows, you can download it from [https://www.glistengine.com/download](https://www.glistengine.com/download)
Please keep in mind that this installer might not always have the latest version of the engine. For students, this is usually enough.

### II. Installation For Developers

We suggest using the installation scripts instead of manually installing the engine, here are the links for automated install scripts.
These scripts might not always work, if you have issues installing please feel free to create an issue via https://github.com/GlistEngine/GlistEngine/issues.

- Windows Install Script
    - Open up a powershell terminal and run this command to install GlistEngine:
    ```bash
    irm "https://raw.githubusercontent.com/GlistEngine/InstallScripts/main/scripts/windows/install-glist.ps1" | iex
    ```
- Mac Install Script (Universal)
    - Open up a terminal and run this command to install GlistEngine:
    ```bash
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/GlistEngine/InstallScripts/main/scripts/macos/install-glist.sh)"
    ```
- Linux Install Script
    - Open up a terminal and run this command to install GlistEngine:
    ```bash
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/GlistEngine/InstallScripts/main/scripts/linux/install-glist.sh)"
    ```

If you want to install manually instead, you can follow the steps for your platform.

<details>
<summary>Windows Setup</summary>

**1. Install Git**  
Visit:  
```text
https://git-scm.com/download/win
````

**2. Create folders**

```powershell
mkdir C:\dev\glist\myglistapps
mkdir C:\dev\glist\zbin
```

**3. Go to zbin**

```powershell
cd C:\dev\glist\zbin
```

**4. Download zbin**
Visit:

```text
https://github.com/GlistEngine/glistzbin-win64/releases/latest
```

Extract into `C:\dev\glist\zbin`

**5. Back up one level**

```powershell
cd C:\dev\glist
```

**6. Clone GlistEngine**

```bash
git clone https://github.com/yourgithubusername/glistengine.git
```

**7. Clone GlistApp**

```powershell
cd C:\dev\glist\myglistapps
git clone https://github.com/yourgithubusername/glistapp.git
```

</details>

<details>
<summary>macOS Setup</summary>

**1. Install Xcode**

You can install Xcode via App Store, after the installation complete, open it and accept the end user agreement. You might need accept this agreement after some macOS updates. 

**2. Install Homebrew**

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**3. Install libs & config**

```bash
brew install llvm gcc@11 cmake glew glfw glm freetype assimp curl openssl pkg-config
export PATH=$PATH:~/usr/local/bin/cmake
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

</details>

<details>
<summary>Linux Setup</summary>

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

</details>

## Getting Started
After installation, you are very close to start developing your first application using GlistEngine.

<details>
<summary>On Windows</summary>

If you used the installation script, you can use the shortcut created on your desktop to start GlistEngine Eclipse. 

If not, open your file explorer, go to `C:\dev\glist\zbin\glistzbin-win64` and double click **GlistEngine** to start the GlistEngine Eclipse.

</details>

<details>
<summary>On macOS</summary>

You can start GlistEngine Eclipse from Launchpad or Applications via Finder.
If you see "app is damaged...", open Applications via Finder and find GlistEngine Eclipse, then while holding option, right click to it. Then while still holding option, click Open.

</details>

<details>
<summary>On Linux</summary>

If you used the installation script, you can use the shortcut created on your desktop to start GlistEngine Eclipse. 

If not, open your file explorer, go to `~/dev/glist/zbin/glistzbin-linux` and double click **GlistEngine** to start the GlistEngine Eclipse.

</details>

### First screen
You will see GlistApp and GlistEngine on the left column. And GameCanvas src/h files will be opened in the code editor. These are the source code of GlistApp.


### Launch the GlistApp
On the toolbar of the IDE, there is a dropdown button just next to Run button. Dropdown this list. Click "GlistApp Release" on the list. The GlistApp and the engine will be built and run for the first time.


### Start coding
After closing the running window, you can start coding your new gorgeous game on GameCanvas.


## Troubleshooting

1- Unresolved Inclusions

If you see "Unresolved Inclusion" errors on the editor, that means the ide lost the pre-determined include path entries.

- Open Project->Properties->C/C++ General->Preprocessor Include Paths->Providers. Go to the bottom of the list. Select CMAKE_EXPORT_COMPILE_COMMANDS Compiler and Parser selections. Click Apply and close button.
- Click Project->Clean Project to clean previous build
- Click hammer button to rebuild the project.
- Click Project->C/C++ Index->Rebuild to invoke indexer.

Then the "Unresolved Inclusion" errors should be gone.


## Contributing

We're a welcoming community. You can follow the steps below to send a PR. Please keep in mind that you cannot create pull requests using the MSI installer for Windows, see below for more information.

1. **Make your changes** in the workspace.  
4. **Commit locally & push:**  
   - Right-click your project -> **Team -> Commit...**  
   - Select the files you changed, write a clear commit message. Try to keep the first line short, you can explain as much as you want after the first line.  
   - Check **"Push changes to upstream"** at the bottom and click **Commit and Push**.  
5. **Open the PR on GitHub:**  
   - After the push completes, go to your fork on GitHub.  
   - Click **Compare & pull request**, choose our main repo as the base, fill in the description, and submit. Make sure to explain what's the purpose of your pull request, even if your commit explains it.

That's it, your changes will be reviewed and if approved, will be merged. You might get a feedback and be required to make some changes before your PR is merged, make sure to check your PR for feedbacks. To update your PR, simply commit to your branch and those changes will be included. Thanks for contributing!  

***If you installed GlistEngine using the installer***

If you installed GlistEngine with the Windows MSI Installer (student installation above) and want to contribute the repo, you can do the following configuration:

1. Right click on the GlistApp and GlistEngine on the left column and select Delete from the drop down menu. (Unselect delete project content on disk)
2. Right click on an empty area of the left column. Select Import...->Git->Projects From Git->Next->Existing Local Repository. Import first GlistEngine, then import GlistApp.


## Licence

This project is under heavy development and has a Apache License 2.0.


## Version

GlistEngine has a [semantic versioning system](https://semver.org/). The current version is 0.1.2
