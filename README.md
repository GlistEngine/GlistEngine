# GLIST ENGINE

Glist Engine is a cross platform OpenGL game engine written in C++.

The engine supports OpenGL 3.3. It can be used to develop Windows and Linux games. Macintosh and mobile platforms coming soon.

Glist Engine consists of 3 repositories:
- Glist App
- Glist Engine (this repo) and
- Glist Zbin

Glist Engine repo contains the core engine, plugins, samples to use in the development. Glist Zbin contains libs, includes and a preconfigured ide. Glist App is the repo to clone when starting each new game project.

More information about GlistEngine can be found at [https://glistengine.gamelab.istanbul/](https://glistengine.gamelab.istanbul/)


## Installation

### I. Installation For Students

Glist Engine has one-click installers for Windows, Linux and Mac. You can download the installers from the below links:

- Windows Installer - [https://glistengine.gamelab.istanbul/documentation/tutorials/glistengine-setup-tutorial.html](https://glistengine.gamelab.istanbul/documentation/tutorials/glistengine-setup-tutorial.html)
- Linux installer (coming soon)
- Mac installer (coming soon)


### II. Installation For Developers

1- Fork one of the GlistZbin-Win64 or GlistZbin-Linux repo intos your account, according to your computer's operating system


2- Fork the GlistEngine and GlistApp repos into your account.


3- Open file explorer and create necessary folders:

On Windows:
- C:\dev\glist\myglistapps
- C:\dev\glist\zbin

On Linux:
- ~/dev/glist/myglistapps
- ~/dev/glist/zbin


4- Open command line(cmd.exe or bash) and go to the zbin folder by the command

On Windows:
> cd C:\dev\glist\zbin

On Linux:
> cd ~/dev/glist/zbin


5- First of all, you need to clone the zbin repo. Clone the zbin repo by writing one of these commands on command line:

On Windows:
>  git clone https://github.com/Yavuz1234567890/glistzbin-win64.git

On Linux:
>  git clone https://github.com/Yavuz1234567890/glistzbin-linux.git


6- Go one folder up

On Windows:
> cd C:\dev\glist

On Linux:
> cd ~/dev/glist


7- Clone the GlistEngine repo

> git clone https://github.com/Yavuz1234567890/glistengine.git


8- Go to myglistapps folder

On Windows:
> cd C:\dev\glist\myglistapps

On Linux:
> cd ~/dev/glist/myglistapps


9- Clone the GlistApp repo

> git clone git clone https://github.com/Yavuz1234567890/glistapp.git


10- On file explorer, go to;

On Windows:
> cd C:\dev\glist\zbin\glistzbin-win64

On Linux:
> cd ~/dev/glist/zbin/glistzbin-linux

directory. You will see a file named "GlistEngine". Double click this file to start the ide.


11- You will see GlistApp and GlistEngine on the left column. And GameCanvas src/h files will be opened in the code editor. These are the source code of GlistApp.


12- On the toolbar of the IDE, there is a dropdown button just next to Run button. Dropdown this list. Click "GlistApp Release" on the list. The GlistApp and the engine will be built and run for the first time.


13- After closing the running window, you can start coding your gorgeous game on GameCanvas.


## Troubleshooting

1- Unresolved Inclusions

If you see "Unresolved Inclusion" errors on the editor, that means the ide lost the pre-determined include path entries.

- If so, click GlistApp on the left column, then open Project->Properties->C/C++ General->Paths and symbols->Includes. Select Debug from the Configuration drop down and click Languages->GNU C++. Copy the paths here. Select Release from the Configuration dropdown and click Languages->GNU C++. Click Add. and paste the paths here. After copying all paths to Release, click Apply and close button.
- Click GlistEngine on the left column and do the same copy-paste jobs here too. (This repo has more lines)
- Select GlistApp again and click hammer button to rebuild the project.
- Click Project->C/C++ Index->Rebuild to invode indexer.

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
