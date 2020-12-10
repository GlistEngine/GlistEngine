# GLIST ENGINE

Glist Engine is a cross platform OpenGL game engine written in C++.

The engine supports OpenGL 3.3. It can be used to develop Windows, Linux and Macintosh games. Mobile platforms coming soon.

Glist Engine consists of 3 repositories:
- Glist App
- Glist Engine (this repo) and
- Glist Zbin

Glist Engine repo contains the core engine, plugins, samples to use in the development. Glist Zbin contains libs, includes and a preconfigured ide. Glist App is the repo to clone when starting each new game project.


## Installation

### I. Installation For Students

Glist Engine has one-click installers for Windows, Linux and Mac. You can download the installers from the below links:

- Windows Installer (coming soon)
- Linux installer (coming soon)
- Mac installer (coming soon)


### II. Installation For Developers

1- Fork the GlistEngine, GlistApp and GlistZbin-Win64 repos into your account.

2- Open file explorer and create necessary folders:

- C:\dev\glist\myglistapps
- C:\dev\glist\zbin

3- Open command line(cmd.exe) and go to the zbin folder by the command

> cd C:\dev\glist\zbin

4- First of all, you need to clone the zbin repo. Clone the zbin repo by writing this command on command line:

>  git clone https://github.com/yourusername/glistzbin-win64.git

5- Go one folder up to C:\dev\glist

> cd C:\dev\glist

5- Clone the GlistEngine repo

> git clone https://github.com/yourusername/glistengine.git

6- Go to myglistapps folder

> cd C:\dev\glist\myglistapps

7- Clone the GlistApp repo

> git clone git clone https://github.com/yourusername/glistapp.git

8- On file explorer, go to C:\dev\glist\zbin\glistzbin-win64 directory. You will see a file named "GlistEngine_Win64". Double click this file to start the ide.

9- You will see GlistApp and GlistEngine on the left column. And GameCanvas src/h files will be opened in the code editor. Click somewhere on GameCanvas.cpp. Then click the hammer button located on the toolbar. The GlistApp and the engine will be built for the first time.

10- After the build process, you can start coding your game on GameCanvas.


## Troubleshooting

1- If you see "Unresolved Inclusion" errors on the editor, that means the ide lost the pre-determined include path entries.

2- If so, click GlistApp on the left column, then open Project->Properties->C/C++ General->Paths and symbols->Includes. Select Debug from the Configuration drop down and click Languages->GNU C++. Copy the paths here. Select Release from the Configuration dropdown and click Languages->GNU C++. Click Add. and paste the paths here. After copying all paths to Release, click Apply and close button.

3- Click GlistEngine on the left column and do the same copy-paste jobs here too. (This repo has more lines)

4- Select GlistApp again and click hammer button to rebuild the project.

5- Click Project->C/C++ Index->Rebuild to invode indexer.

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
