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
- Mac Install Script (Intel and Apple Silicon)
    - Open up a terminal and run this command to install GlistEngine:
    ```bash
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/GlistEngine/InstallScripts/main/scripts/macos/install-glist.sh)"
    ```
- Linux Install Script
    - Open up a terminal and run this command to install GlistEngine:
    ```bash
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/GlistEngine/InstallScripts/main/scripts/linux/install-glist.sh)"
    ```

If you'd rather install manually instead of using the script, follow the instructions for your platform:
 - [Windows](./INSTALLATION-WIN.md)
 - [macOS](./INSTALLATION-MACOS.md)
 - [Linux](./INSTALLATION-LINUX.md)

## Getting Started
See [GETTING-STARTED.md](./GETTING-STARTED.md) for platform-specific launch instructions.

## Troubleshooting

### Unresolved Inclusions

If you see "Unresolved Inclusion" errors on the editor, that means the ide lost the pre-determined include path entries.

- Open Project->Properties->C/C++ General->Preprocessor Include Paths->Providers. Go to the bottom of the list. Select CMAKE_EXPORT_COMPILE_COMMANDS Compiler and Parser selections. Click Apply and close button.
- Click Project->Clean Project to clean previous build
- Click hammer button to rebuild the project.
- Click Project->C/C++ Index->Rebuild to invoke indexer.

Then the "Unresolved Inclusion" errors should be gone.


## Contributing

We're a welcoming community. You can follow the steps below to send a PR. Please keep in mind that you cannot create pull requests if you installed the engine using the MSI installer for Windows, see [below](#if-you-installed-glistengine-using-the-installer) for more information.

1. **Make your changes** in the workspace.  
4. **Commit locally & push:**  
   - Go to the Git Staging window, if it's not open, you can open it from **Window -> Show View -> Other** and select Git Staging  
   - Here, stage the files you changed, write a clear commit message. Try to keep the first line short, you can explain as much as you want after the first line.  
   - Click **Commit and Push**.  
5. **Open the PR on GitHub:**  
   - After the push completes, go to your fork on GitHub.  
   - Click **Compare & pull request**, choose our main repo as the base, fill in the description, and submit. Make sure to explain what's the purpose of your pull request, even if your commit explains it.

That's it, your changes will be reviewed and if approved, will be merged. You might get a feedback and be required to make some changes before your PR is merged, make sure to check your PR for feedbacks. To update your PR, simply commit to your branch and those changes will be included. Thanks for contributing!  

### If you installed GlistEngine using the installer

If you installed GlistEngine with the Windows MSI Installer (student installation above) and want to contribute the repo, you can do the following configuration:

1. Right click on the GlistApp and GlistEngine on the left column and select Delete from the drop down menu. (Unselect delete project content on disk)
2. Right click on an empty area of the left column. Select Import...->Git->Projects From Git->Next->Existing Local Repository. Import first GlistEngine, then import GlistApp.


## Licence

This project is under heavy development and has a Apache License 2.0.


## Version

GlistEngine follows the [semantic versioning system](https://semver.org/). The current version is 0.1.2
