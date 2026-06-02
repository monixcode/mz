# MZ File Archiver

<img width="320" height="180" alt="mz" src="https://github.com/user-attachments/assets/de077a1b-6450-44cb-bb21-de39342a85d5" />

## Features
1) Archiving Many Files in a Single File.
2) Extracting an archived file safely.

## Limitations
1) No Path Sanitization
2) Only For Windows Till Now.

## Compile
```bash
gcc mz.c -o mz.exe
```

## Commands
### Archive
```bash
mz -a/-archive file.txt file2.txt .... -o/-output output.mz
```
### Extract
```bash
mz -x/-extract file.mz file2.mz ....
```
## License
MIT LICENSE @2026
