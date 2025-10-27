## 🤝 Contributing to OMath or other Orange's Projects

### ❕ Prerequisites

- A working up-to-date OMath installation
- C++ knowledge
- Git knowledge
- Ability to ask for help (Feel free to create empty pull-request or PM a maintainer
  in [Telegram](https://t.me/orange_cpp))

### ⏬ Setting up OMath

Please read INSTALL.md file in repository

### 🔀 Pull requests and Branches

In order to send code back to the official OMath repository, you must first create a copy of OMath on your github
account ([fork](https://help.github.com/articles/creating-a-pull-request-from-a-fork/)) and
then [create a pull request](https://help.github.com/articles/creating-a-pull-request-from-a-fork/) back to OMath.

OMath development is performed on multiple branches. Changes are then pull requested into master. By default, changes
merged into master will not roll out to stable build users unless the `stable` tag is updated.

### 📜 Code-Style

The orange code-style can be found in `.clang-format`.

### 📦 Building

OMath has already created the  `cmake-build` and `out` directories where cmake/bin files are located. By default, you
can build OMath by running `cmake --build cmake-build/build/windows-release --target omath -j 6` in the source
directory.
