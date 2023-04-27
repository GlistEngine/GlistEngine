# GLIST ENGINE

Glist Engine is a cross platform OpenGL game engine written in C++.

The engine supports OpenGL 3.3. It can be used to develop Windows, macOS and Linux games. Mobile platforms coming soon.

Glist Engine consists of 3 repositories:
- Glist App
- Glist Engine (this repo) and
- Glist Zbin

Glist Engine repo contains the core engine, plugins, samples to use in the development. Glist Zbin contains libs, includes and a preconfigured ide. Glist App is the repo to clone when starting each new game project.

More information about GlistEngine can be found at [https://www.glistengine.com](https://www.glistengine.com)


## Installation

### I. Installation For Students

Glist Engine has one-click installers for Windows, Linux and Mac. You can download the installers from the below links:

- Windows Installer - [https://www.glistengine.com/download](https://www.glistengine.com/download)
- Linux installer (coming soon)
- Mac installer (coming soon)


### II. Installation For Developers

#### 1- Fork our 2 repos into your Github account:

- GlistEngine/GlistEngine repo
- GlistEngine/GlistApp repo

#### 2- You need to install some programs into your computer in order to clone and run GlistEngine.

<ins>On Windows:</ins>
- We provide all libraries except GIT. You should first download and install GIT from https://git-scm.com/download/win

<ins>On macOS:</ins>
- Please note that the installation is tested on a clean Monterey. If you have an older OS version, consider upgrading your Mac

- First of all, you should install Xcode because OpenGL library comes with Xcode. Download Xcode via App Store

- Open Terminal and install homebrew
> /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

- Install libraries via home-brew and make necessary configurations
> brew install llvm gcc@11 cmake glew glfw glm freetype assimp curl openssl

> export PATH=$PATH:~/usr/local/bin/cmake

> sudo spctl --master-disable

<ins>On Linux:</ins>
- Install necessary libraries on Ubuntu and other Debian based Linux
> sudo apt-get install git cmake clang-14 libstdc++-12-dev libglew-dev curl libssl-dev

- Install necessary libraries rpm based system Linux
> sudo dnf install glm-devel glfw glfw-decel libglew-devel freetype-devel freetype assimp assimp-devel curl curl-devel libssl libssl-devel

#### 3- Open file explorer and create necessary folders:

On Windows:
- C:\dev\glist\myglistapps
- C:\dev\glist\zbin

On macOS:
- ~/dev/glist/myglistapps
- ~/dev/glist/zbin

On Linux:
- ~/dev/glist/myglistapps
- ~/dev/glist/zbin


4- Open command line(cmd.exe or bash) and go to the zbin folder by the command

On Windows:
> cd C:\dev\glist\zbin

On macOS:
> cd ~/dev/glist/zbin

On Linux:
> cd ~/dev/glist/zbin


5- First of all, you need to download the zbin repo. Download the zbin repo by visiting one of these pages and extract the zip:

On Windows:
>  https://github.com/GlistEngine/glistzbin-win64/releases/latest

On MacOS Arm64 (M1, M2):
>  https://github.com/GlistEngine/glistzbin-macos/releases/latest

On MacOS X86_64:
>  https://github.com/GlistEngine/glistzbin-macos-x86_64/releases/latest

On Linux:
>  https://github.com/GlistEngine/glistzbin-linux/releases/latest


6- Go one folder up

On Windows:
> cd C:\dev\glist

On macOS:
> cd ~/dev/glist

On Linux:
> cd ~/dev/glist


7- Clone the GlistEngine repo

> git clone https://github.com/yourgithubusername/glistengine.git


8- Go to myglistapps folder

On Windows:
> cd C:\dev\glist\myglistapps

On macOS:
> cd ~/dev/glist/myglistapps

On Linux:
> cd ~/dev/glist/myglistapps


9- Clone the GlistApp repo

> git clone https://github.com/yourgithubusername/glistapp.git


10- Run the GlistEngine

Start the engine.

On Windows:
- Open file explorer, go to C:\dev\glist\zbin\glistzbin-win64 and double click GlistEngine

On macOS M1:
- Never start the engine with clicking Eclipse icon on Launchpad! Instead, you should open the Terminal and enter this command:
> open ~/dev/glist/zbin/glistzbin-macos/eclipse/eclipsecpp-2020-09/Eclipse.app 
- (If you get an error message "app is damaged and can’t be opened. You should move it to the Trash.", enter this command on Terminal:
> xattr -cr ~/dev/glist/zbin/glistzbin-macos/eclipse/eclipsecpp-2020-09/Eclipse.app
- and restart your computer.

On macOS X86_64:
- Never start the engine with clicking Eclipse icon on Launchpad! Instead, you should open the Terminal and enter this command:
> open ~/dev/glist/zbin/glistzbin-macos-x86_64/eclipse/eclipsecpp-2020-09/Eclipse.app 
- (If you get an error message "app is damaged and can’t be opened. You should move it to the Trash.", enter this command on Terminal:
> xattr -cr ~/dev/glist/zbin/glistzbin-macos-x86_64/eclipse/eclipsecpp-2020-09/Eclipse.app
- and restart your computer.

On Linux:
- Open file explorer, go to ~/dev/glist/zbin/glistzbin-linux and double click GlistEngine


11- You will see GlistApp and GlistEngine on the left column. And GameCanvas src/h files will be opened in the code editor. These are the source code of GlistApp.


12- On the toolbar of the IDE, there is a dropdown button just next to Run button. Dropdown this list. Click "GlistApp Release" on the list. The GlistApp and the engine will be built and run for the first time.


13- After closing the running window, you can start coding your new gorgeous game on GameCanvas.


## Troubleshooting

1- Unresolved Inclusions

If you see "Unresolved Inclusion" errors on the editor, that means the ide lost the pre-determined include path entries.

- Open Project->Properties->C/C++ General->Preprocessor Include Paths->Providers. Go to the bottom of the list. Select CMAKE_EXPORT_COMPILE_COMMANDS Compiler and Parser selections. Click Apply and close button.
- Click Project->Clean Project to clean previous build
- Click hammer button to rebuild the project.
- Click Project->C/C++ Index->Rebuild to invoke indexer.

Then the "Unresolved Inclusion" errors should be gone.


## Contributing

We are a warm community. We encourage the game developers to contribute into the engine by sending commits, opening bug reports and issues.

If you want to send PR, please open the staging view via 

> Window->Show View->Other..->Git->Git Staging.

On the Staged Changes list, remove unnecessasy files by selecting them and clicking the - (Remove selected files from the index) button on up-right corner of the list. Add your commit message and send the commit to your fork repo. Then login Github and send PR to our repos.

***If you installed GlistEngine using the installer***

One can contribute into the code when working on the cloned versions of the repos. If you installed GlistEngine by the installer (student installation above) and want to contribute the repo, you can do the following configuration:

1- Right click on the GlistApp and GlistEngine on the left column and select Delete from the drop down menu. (Unselect delete project content on disk)

2- Right click on an empty area of the left column. Select Import...->Git->Projects From Git->Next->Existing Local Repository. Import first GlistEngine, then import GlistApp.


## Licence

This project is under heavy development and has a Apache Licence 2.0.


## Version

GlistEngine has a semantic versioning system. Current version is 0.1.2
