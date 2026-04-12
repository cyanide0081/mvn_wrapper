Wraps maven command line script setting JAVA_HOME and other relevant variables
based on the target version of your current project's pom.xml file(s).

## Building

Currently supported platforms:
* Windows with
  [Clang](https://releases.llvm.org/download.html) or
  [MSVC](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026)
* Linux with
  [Clang](https://releases.llvm.org/download.html) or
  [GCC](https://gcc.gnu.org)

Once you have at least one installed and your environment variables set up, run:
* Windows:
```
.\build.cmd
```
* Linux:
```
./build.sh
```
