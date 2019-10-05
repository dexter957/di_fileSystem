# di_fileSystem

## A custom file system, based on the unix inodes.
###### This project has been developed as part of the Operating Systems class, NKUA, winter semester 2016-2017.

Mydiz is a system utility that flattens a given hierarchy of files (flat ones or directories). The resulting file has the postfix .di.
One should be able to retrieve the files they' ve stored in the .di file, that may or may not have been stored in a compressed form (using gzip utility).
Mydiz should be able to handle, not only files and directories, but soft links as well (not implemented).

Command Line
Flags may be used to determine what should the utility do in a specific call:
./mydiz {-c|-a|-x|-m|-d|-p|-j|-q} <archive-file> <list-of_files/dirs>
  
 




