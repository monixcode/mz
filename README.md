# MZ File Archiver

<img width="320" height="180" alt="mz" src="https://github.com/user-attachments/assets/de077a1b-6450-44cb-bb21-de39342a85d5" />

## Version
1.1.1

## Changelog
1) Proper --help section
2) Added Basic Path Sanitization on mz_archive function

## Features
1) Archiving Many Files into a Single File.
2) Extracting an archived file safely.
3) Basic Path Sanitization
4) Cross Compatible on Windows and Linux

## Compile
```bash
gcc mz.c -o mz.exe
```
## Builds
[MZ Builds](https://github.com/monixcode/mz/releases)

## Commands
### Archive
```bash
mz -a/-archive file.txt file2.txt .... -o/-output output.mz
```
### Extract
```bash
mz -x/-extract file.mz file2.mz ....
```

## Optionals
```bash
mz --help/--h
mz --version/--v
```

## License
MIT LICENSE @2026
