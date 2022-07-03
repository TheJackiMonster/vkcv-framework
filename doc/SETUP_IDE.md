# How to setup your IDE

## VSCodium

1. Install [VSCodium](https://github.com/VSCodium/vscodium)
2. Run `scripts/setup-codium.sh` in your local repository
3. Run VSCodium and open your local repository as project

### VSCode

If you should prefer to use the official binaries from Microsoft including their own telemetry, you can follow these steps:

1. Download and install [VSCode](https://code.visualstudio.com/Download)
2. Run `scripts/setup-codium.sh` in your local repository or in case of a Windows host run following line from PowerShell:
```
Get-Content resources\extensions.txt | ForEach-Object {code --install-extension $_}
```
3. Run VSCode and open your local repository as project

## Eclipse CDT

1. Download and install [Eclipse CDT](https://www.eclipse.org/cdt/downloads.php)
2. Run Eclipse CDT and open your local repository as project

## CLion

1. Download and install [CLion](https://www.jetbrains.com/clion/download/)
2. Run CLion and open your local repository as project

