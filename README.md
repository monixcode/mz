# MZ File Archiver

A lightweight, cross-platform **file archiver** written entirely in **ISO C**.

Unlike ZIP or 7z, **MZ does not compress data**. It simply stores multiple files inside a single archive, making archiving and extraction extremely fast while preserving the original file contents.

The project is written using only the **C Standard Library**, requiring **no external dependencies**.

---

## Features

- 📦 Archive multiple files into a single `.mz` archive
- 📁 Recursively archive entire directories
- ⚡ Fast archiving and extraction (no compression)
- 🔒 Safe extraction from valid archives
- 🎯 Extract selected files without unpacking everything
- 📄 List files stored inside an archive
- ℹ️ Display archive metadata
- 🖥️ Optional verbose and error logging
- 🌍 Cross-platform
  - Windows
  - Linux
- 🧩 Pure ISO C implementation
- 📚 No third-party runtime dependencies

---

## Version

**v0.1.0**

---

# Building

Compile using any C compiler.

### GCC

```bash
gcc -O2 -o mz mz.c
```

### MSVC

```cmd
cl mz.c
```

---

# Downloads

Precompiled binaries are available from the GitHub Releases page.

| Platform | Download |
|----------|----------|
| Windows (x64) | [mz-win64.zip](https://github.com/monixcode/mz/releases/tag/v0.1.0) |
| Linux (x86_64) | [mz-linux_x86_64.tar.gz](https://github.com/monixcode/mz/releases/tag/v0.1.0) |

---

# Usage

```
mz [command] [options]
```

---

## Create an Archive

Archive individual files.

```bash
mz -a image.png index.html -o archive.mz --verbose --error
```

Archive an entire directory recursively.

```bash
mz -a -Idir docs -o archive.mz --verbose --error
```

---

## Extract an Archive

Extract all files.

```bash
mz -x archive.mz
```

Extract all files of a specific directory.

```bash
mz -x archive.mz -Idir extracted --verbose --error
```

---

## Extract Specific Files

```bash
mz -xs archive.mz image.png index.html --verbose --error
```

---

## List Archive Contents

```bash
mz -fi archive.mz
```

---

## Display Archive Information

```bash
mz -info archive.mz
```

---

## Show Help

```bash
mz --help
```

---

## Show Version

```bash
mz --version
```

---

# Project Goals

- Simple archive format
- Zero external dependencies
- High performance
- Low memory usage
- Portable across operating systems
- Easy-to-read source code
- Suitable for learning systems programming in C

---

# Credits

MZ follows the **STB-style single-header library design** popularized by **Sean Barrett**.

https://github.com/nothings

---

# License

This project is licensed under the **MIT License**.

See the `LICENSE` file for details.

Copyright © 2026 MZ Project.
