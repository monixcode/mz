## MZ File Archiver
A File Archiving CLI Tool written in pure C. No external third-party dependencies required except the C standard libraries.

## Version
2.0.0

## Changelog
1) Added new tags: -I for input and -P for output control.
2) Added -Idir to recursively read all files within a directory.
3) Added -Ifile to explicitly specify an input file when automatic file detection is not suitable.
4) Added -Pverbose for detailed output.
5) Added -Pprocess for basic progress output.
6) Added -Perror to enable error reporting.
7) Added -Pfilesin to display all files stored in an archive.    
8) Fixed numerous bugs and improved stability.

## Features
- Archiving Many Files into a Single File.
- Safely extract archived files.
- Basic Path Sanitization on Both Extracting and Archiving
- Cross Compatible on Windows and Linux
- Supports archiving an unlimited number of files, constrained only by available memory
- View the names and count of files stored within an archive using -Pfilesin.
- Proper error handling and reporting with meaningful exit codes through -Perror.
- Recursively process directories and files using -Idir and -Ifile for archiving (-a), extraction (-x), and archive inspection (-Pfilesin).
- Flexible output control through -Perror, -Pverbose, and -Pprocess.

## Compile
```
gcc mz.c -o mz.exe
```

## Downloads or Compiled Binaries
### Windows
[mz-win64.zip](https://github.com/monixcode/mz/releases/download/2.0.0/mz-win64.zip)
### Linux
[mz-linux-x86_64.tar.gz](https://github.com/monixcode/mz/releases/download/2.0.0/mz-linux-x86_64.tar.gz)

## Commands
### Archive with error printing and process printing
```
mz -a file.txt file2.txt .... -o/-output output.mz -Perror -Pprocess
```

### Extract with error printing and verbose printing
```
mz -x -Perror -Pverbose file.mz file2.mz ....
```

### Check Files
```
mz -Pfilesin file.mz
```

### Read Folder and archive
```
mz -a -Idir folder -o out.mz
```

### Read Folder and extract
```
mz -x -Idir folder
```

### Read Folder and check files
```
mz -Pfilesin -Idir folder
```

## Optionals
```
mz --help
```
```
mz --version
```

## Credits
This software's header style was inspired by the STB family of C libraries created by [Sean Barrett](https://github.com/nothings). The implementation is original, but the single-header framework concept and overall design philosophy draw inspiration from the STB approach.

## License
MIT LICENSE @2026
