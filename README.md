# OpenGL Triangle

A small PS4 homebrew sample: a triangle drawn with OpenGL ES 2 through EGL
(Mesa), over a pulsing background, for 30 seconds. It shows the least a
program needs to put OpenGL on the screen with the
[ps4-homebrew-dev SDK](https://github.com/ps4-homebrew-dev/sdk), and how
to package it with Mesa's libraries.

## Building
With the SDK and its libraries from
[pacbrew](https://github.com/ps4-homebrew-dev/pacbrew/releases):

```sh
/opt/ps4-homebrew-sdk/bin/orbis-cmake -S . -B build
make -C build
```

That gives `build/IV0000-BREW95439_00-GLTRIANGLE000000.pkg` to install.
Prebuilt pkgs are on the [releases](../../releases) page.
