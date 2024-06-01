# prodosfs

A mountable read-only filesystem for Apple II ProDOS 8 disk images.

> [!NOTE]
> This project is under development and incomplete.

# Description

prodosfs is a [FUSE](https://github.com/libfuse/libfuse) filesystem that allows mounting images of Apple II ProDOS 8 disks in Linux for read-only access. Once mounted, the ProDOS files and directories can be viewed with standard GNU/Linux tools (ls, cat, etc).

## Limitations

This software was written to allow to recovery of the author's 30+-year-old files. As such, it is by no means intended to be a full, polished implementation of a ProDOS filesystem. Only functionality required to read one or more of the author's 5&#188;&#8243; floppy disk images is implemented.

Since it is intended mainly for personal use on small disks, there has been no effort spent on performance or security. There are currently little protections against misuse, unintentional or otherwise.

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

Informational and error messages are written to `stderr`. The files are accessible until the `prodosfs` program exits due to an unexpected error or the mount directory is unmounted with `umount`.

## Extended attributes

Many ProDOS filesystem properties that do not obviously map to similar POSIX properties (e.g. file creation date and time) are accessible as extended attributes in the `prodos` namespace. For example:

```
$ getfattr -d -m "prodos.*" IMPORTANT.ADRS
# file: IMPORTANT.ADRS
prodos.access="READ WRITE BACKUP RENAME DESTROY"
prodos.appleworks_filename="Important Adrs"
prodos.aux_type="$DC7F"
prodos.creation_timestamp="12-JAN-92 00:00 AM"
prodos.file_type="$1A"
prodos.file_type_description="AppleWorks word processor file"
prodos.file_type_name="AWP"
prodos.min_version="0"
prodos.version="8"
```
# To Do

- [ ] Comment code
- [ ] Mount as read-only
- [ ] Add option to background self
- [ ] Add option to mount under volume name
- [ ] Add option to disable ProDOS-to-Unix text file translation
