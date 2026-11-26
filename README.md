BWXML-unboost
=====

A tool for unpacking BigWorld compressed data sections to plain XML.
Also supports packing XML back to its compressed form. Does not use Boost for the core `libbwxml` library.

## How to build

Prerequisites are:
- [pugixml](https://github.com/zeux/pugixml) by Arseny Kapoulkine, tested and developed against v1.16
- [Base64 encoding/decoding header-only library](https://github.com/tobiaslocker/base64) by Tobias Locker
- [Python](https://www.python.org/downloads/) 2.6.4 or newer for `bwxml-pyd` (Python binding for `libbwxml`)
- [Boost](https://www.boost.org/releases/latest/) 1.89 or newer (required only by `bwxml` CLI tool; subject to change in future releases)
- [CMake](https://cmake.org/download/) 3.19 or newer
- A sane C++17 conformant compiler, tested and developed against Microsoft Visual C++ v145 (Visual Studio "18")

### Configuring

- `BUILD_BWXML_TOOL` (default: off)
  `bwxml` CLI tool target
- `BUILD_PYTHON_MODULE` (default: on)
  `bwxml-pyd` Python binding target, exported as `bwxml.pyd`
- `pugixml_DIR` (unassigned)
   Path to look for `pugixml-config.cmake` file
- `TOBIASLOCKER_BASE64_DIR` (unassigned)
   Path to the source tree of Tobias Locker's Base64 encoding/decoding header-only library. Set it to the root folder, e.g. `tobiaslocker/base64`. The `include` directory is handled by the project.

## Support

Feel free to post on the official EI Forums [topic](https://forum.k2mg.net/?showtopic=2737) or use [issues](https://github.com/SigmaTel71/BWXML/issues) to report a bug. **These are the only communication ways we can provide.**