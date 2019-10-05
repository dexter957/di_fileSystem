# di_fileSystem

## A custom file system, based on the unix inodes.
###### This project has been developed as part of the Operating Systems class, NKUA, Department of Informatics and Telecommunications, fall semester 2016-2017.

Mydiz is a system utility that flattens a given hierarchy of files (flat ones or directories). The resulting file has the postfix .di.
One should be able to retrieve the files they' ve stored in the .di file, that may or may not have been stored in a compressed form (using gzip utility).
Mydiz should be able to handle, not only files and directories, but soft links as well (not implemented).

###### **Command Line**
Flags may be used to determine what should the utility do in a specific call:
###### ./mydiz {-c|-a|-x|-m|-d|-p|-j|-q} <archive-file> <list-of_files/dirs>
  
 
* -c: Archiving of the files in <list-of_files/dirs> (recursive in case of nested directories)
* -a: Adds the files in <list-of_files/dirs> in the (may) already existent <archive-file>
* -x: Exports everything that has been stored in <archive-file>
* -m: Prints metadata (owner, group, access rights) about the entities stored in <archive-file>.
* -d: Delete the entities within <list-of_files/dirs>, that have been stored in <archive-file>. (not implemented)
* -p: Pretty print of the hierarchy that has been stored in <archive-file>.
* -j: Compress before archiving
* -q: Queries about whether or not the entities within <list-of_files/dirs> have been stored in <archive-file>.
    There should be a positive or negative answer for each entity in <list-of_files/dirs>.

##### **Technical**

It can be derived from the above that we need a way to store, in a flat form, information about files: actual data, as well as metadata about size, ownership, hierarchy, exact storage position, etc. We also need to be able to retrieve all this information. So we are actually implementing some very basic functions of a file system, and therefore we can think of the di file as our hard drive. Thus, much like the hard drive, the di file consists of (memory) blocks (here of variable length) on which is stored everything about a file (data and metadata).
The basic entity of the di file system, is the _dinode_ . A dinode (much like an inode) describes a file entity:
owner, group, directory, access rights, size (in bytes), access timestamps, and the starting block of the file. Each dinode has a unique
number that identifies it. So we basically count the entities stored in our file, by the number of allocated dinodes.
The file entities of the di file system are the plain file, and the directory.
###### _File_ 
A file is an entity, described by its name, the number of blocks and bytes (these two may differ because of fragmentation) it occupies.

###### _Directory_ 
A directory is a file that contains entries about its contents, that may be other directories, or files. For each of these entries, we need to know
the name of the contained file, the length of that name, and the dinode for that file.

###### _Header_ 
The di file has a header, that contains its size (in blocks and in bytes), the first block of the data section, the last block in the data section
(directories hierarchy is stored afterwards), the first block of metadata, the total number of dinodes, and the number of files sto

##### **Authors**
* [Tsuyomaru](https://github.com/Tsuyomaru) 
* [dexter957](https://github.com/dexter957)
