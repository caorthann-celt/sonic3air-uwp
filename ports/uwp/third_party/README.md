This folder is the UWP build's little supply shed.

The main game code still lives in the repo root. This directory only keeps the extra bits the UWP build actually needs, so a fresh checkout can configure and build without outside setup.

Current layout:

- `uwp-deps/`
- `glad-compat/`

The goal here is simple: if somebody opens this port months later, they should still be able to build it without doing archaeology.

## Credits

A big shout goes to Daniel Worley for the UWP/OpenGL/Xbox groundwork and documentation that helped make this port much less painful to figure out:

- [worleydl/uwp-dep](https://github.com/worleydl/uwp-dep)
- [worleydl/libuwp](https://github.com/worleydl/libuwp)
- [worleydl/mesa-uwp](https://github.com/worleydl/mesa-uwp)
- [worleydl/SDL-uwp-gl](https://github.com/worleydl/SDL-uwp-gl)
- [worleydl/uwp_gl_sample](https://github.com/worleydl/uwp_gl_sample)
- [WorleyDL's UWP porting notes](https://wiki.sternserv.xyz/docs/helpful-links/worleydls-uwp-porting-notes)

And of course, credit is also due to the original upstream projects that are actually doing the heavy lifting:

- SDL2
- Mesa
- GLAD
- zlib
