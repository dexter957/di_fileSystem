# di_fileSystem

## A custom file system, based on the unix inodes.
###### This project has been developed as part of the Operating Systems class, NKUA, winter semester 2016-2017.

Mydiz is a system utility that flattens a given hierarchy of files (flat ones or directories). The resulting file has the postfix .di.
One should be able to retrieve the files they' ve stored in the .di file, that may or may not have been stored in a compressed form (using gzip utility).
Mydiz should be able to handle, not only files and directories, but soft links as well (not implemented).

**Command Line**
Flags may be used to determine what should the utility do in a specific call:
./mydiz {-c|-a|-x|-m|-d|-p|-j|-q} <archive-file> <list-of_files/dirs>
  
 
* -c: Archiving of the files in <list-of_files/dirs> (recursive in case of nested directories)
* -a: Adds the files in <list-of_files/dirs> in the (may) already existent <archive-file>
* -x: Exports everything that has been stored in <archive-file>
* -m: Prints metadata (owner, group, access rights) about the entities stored in <archive-file>.
* -d: Delete the entities within <list-of_files/dirs>, that have been stored in <archive-file>. (not implemented)
* -p: Pretty print of the hierarchy that has been stored in <archive-file>.
* -j: Compress before archiving
* -q: Queries about whether or not the entities within <list-of_files/dirs> have been stored in <archive-file>.
    There should be a positive or negative answer for each entity in <list-of_files/dirs>.



