# prodosfs

A mountable read-only filesystem for Apple II ProDOS 8 disk images.

# Description

prodosfs is a [FUSE](https://github.com/libfuse) filesystem that allows mounting images of Apple II ProDOS 8 disks in Linux for read-only access. Once mounted, the ProDOS files and directories can be viewed with standard GNU/Linux tools (ls, cat, etc).

## Limitations

This sofware was written to allow to recovery of the author's 30+-year-old files. As such, it is by no means intended to be a full, polished implementation of a ProDOS filesystem. Only functionality required to read one or more of the author's 5&#188;&#8243; floppy disk images is implemented.

Since it is intended mainly for personal use on small disks, there has been little effort spent on optimization or performance.

# Build

This is a cmake-based project. Building it should be as simple as:

```
$ mkdir build
$ cd build
$ cmake ..
```

# Usage

The only arguments currently supported are an `-f` option to specify the mount directory and pathname to the disk image file:

```
$ build/prodosfs -f /mnt/prodos /path/to/image/file
```

Informational and error messages are written to `stderr`. The files are accessible until the `prodosfs` program exits due to an error or the mount directory is unmounted with `umount`.
