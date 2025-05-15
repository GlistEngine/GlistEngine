# Manual Installation on Windows

**1. Install Git**  
Download the standalone installer from https://git-scm.com/download/win, then install it.

**2. Create folders**

Open up a command terminal, then run the following commands.

```powershell
mkdir C:\dev\glist\myglistapps
mkdir C:\dev\glist\zbin
```

**3. Go to zbin**

```powershell
cd C:\dev\glist\zbin
```

**4. Download zbin**
Download the .zip file from https://github.com/GlistEngine/glistzbin-win64/releases/latest, and then extract it into `C:\dev\glist\zbin`

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
