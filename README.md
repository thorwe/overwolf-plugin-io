simple-io-plugin
================
Used to read files from local disk.
Includes an Overwolf sample WebApp to show all features.

Constants:
==========
```
plugin().PROGRAMFILES
plugin().COMMONFILES
plugin().COMMONAPPDATA
plugin().DESKTOP
plugin().WINDIR
plugin().SYSDIR
plugin().SYSDIRX86
```

Functions:
==========
1. fileExists - check if a file exists locally (notice the way we use /, otherwise you need \\)

```
plugin().fileExists(
  plugin().PROGRAMFILES + "/overwolf/Overwolf.exe.config", 
  function(status) {
  
  if (status === true) {
  } else {
  }
});
```

2. isDirectory - check if a given path is a directory (false if not or doesn't exist)

```
plugin().isDirectory(
  plugin().PROGRAMFILES + "/overwolf", 
  function(status) {
  
    if (status === true) {
    } else {
    }
});
```
 
3. getTextFile - reads a file's contents and returns as text.
Use the second parameter to indicate if the file is in UCS-2 (2 bytes per char) and
it will automatically do the UTF8 conversion.  Otherwise, returns in UTF8

```
plugin().getTextFile(
  plugin().PROGRAMFILES + 
  "/overwolf/Overwolf.exe.config", 
  false, // not a UCS-2 file
  function(status, data) {
          
  if (!status) {
    console.log("failed to get Overwolf.exe.config contents");
  } else {
    console.log(data);
  }
});
```
        
4. getBinaryFile - reads a file's contents and returns as an array of byte values.
NOTE: this function is extremly slow! Use only for small files or to get file header
info using the second parameter (limit) to limit amount of data to fetch

```
plugin().getBinaryFile(
  plugin().PROGRAMFILES + 
  "/overwolf/Overwolf.exe.config",
  -1, // no limits
  function(status, data) {
    if (!status) {
      console.log("failed to get Overwolf.exe.config");
    } else {
      var arr = data.split(",");
      console.log(arr);
    }
});
```