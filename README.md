## <img src="NSpring/res/notepad.png" width="64"> NSpring
MS Notepad Extension Project.

* Support Undo/Redo up to 256 times.

### Install

Run below command on Windows CMD.

```
powershell "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12;(New-Object System.Net.WebClient).DownloadFile('https://github.com/springkim/NSpring/releases/download/bin/NSpring.exe','%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\mspaintex.exe');" & pushd %cd% & cd "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup" & start mspaintex.exe & popd
```