# prodosfs

A mountable read-only filesystem for Apple II ProDOS 8 disk images.

> [!NOTE]
> This project is under development and incomplete.

# Description

prodosfs is a [FUSE](https://github.com/libfuse/libfuse) filesystem that allows mounting images of Apple II ProDOS 8 disks in Linux for read-only access. Once mounted, the ProDOS files and directories can be viewed with standard GNU/Linux tools (ls, cat, etc).

## Limitations

This software was written to allow to recovery of the author's 30+-year-old files. As such, it is by no means intended to be a full, polished implementation of a ProDOS filesystem. Only functionality required to read one or more of the author's 5&#188;&#8243; floppy disk images is implemented.

This is only intended for the ProDOS disks used on the 8-bit Apple II series (i.e. IIe, IIc, etc) _not_ ones used on the 16-bit Apple IIgs (ProDOS 16).

Since it is intended mainly for personal use on small disks, there has been little effort spent on performance or security. There are few protections against misuse, unintentional or otherwise.

# Build

This is a cmake-based project. Building it should be as simple as:

```
$ mkdir build
$ cd build
$ cmake ..
```

# Usage

Two arguments are required: a mount directory and an image file path.

A few options are supported:

* `-h` to output a usage message
* `-f` to run in the foreground instead of backgrounding itself
* `-d` to enable FUSE debugging (implies `-f`)
* `-n` to mount in `<mount dir>/<volume name>` instead of in `<mount dir>`
* `-lN` to set the log level to N (0 = least, 9 = most)

For example:

```
$ build/prodosfs -f -l3 -n /mnt/prodos /path/to/image/file
```

Log messages are written to `stderr` when in the foreground, `/tmp/<image name>.log` when in the background.

The image contents are accessible until the `prodosfs` program exits, either due to an unexpected error, a signal, or the directory being unmounted with `umount`.

## Extended attributes

Many ProDOS filesystem properties that do not obviously map to similar POSIX properties (e.g. file creation date and time) are accessible as extended attributes in a `prodos` namespace. For example:

```
$ getfattr -d -m "prodos.*" IMPORTANT.ADRS
# file: IMPORTANT.ADRS
prodos.access="READ | WRITE | BACKUP | RENAME | DESTROY"
prodos.appleworks_filename="Important Adrs"
prodos.aux_type="$DC7F"
prodos.creation_timestamp="12-JAN-92 00:00"
prodos.file_type="$1A"
prodos.file_type_description="AppleWorks word processor file"
prodos.file_type_name="AWP"
prodos.min_version="0"
prodos.version="8"
```

## Pseudo-files

The file system includes support for fake or synthetic files that don't actually exist in the disk image but are generated dynamically. The only such file currently is `.CATALOG`, which can be read in any directory to view a directory listing in a similar format to the output of the ProdDOS `CATALOG` command:

```
$ cat APPLEWORKS/.CATALOG

/APPLEWORKS

 NAME            TYPE  BLOCKS  MODIFIED         CREATED          ENDFILE  SUBTYPE

 PRODOS           SYS      34  02-NOV-90 12:31  21-MAR-92 00:00    16595         
 APLWORKS.SYSTEM  SYS      26  09-FEB-92 00:00  21-MAR-92 00:00    12683         
 SEG.00           BIN       9  03-AUG-89 15:59  21-MAR-92 00:00     3923         
 SEG.AW           BIN      98  03-AUG-89 16:01  21-MAR-92 00:00    49220         
 SEG.EL           BIN      12  27-DEC-88 09:52  21-MAR-92 00:00     5632         
 SEG.ER           BIN       4  16-OCT-23 00:00  21-MAR-92 00:00     2134         
 SEG.WP           BIN      88  03-AUG-89 16:01  21-MAR-92 00:00    44542         

BLOCKS FREE:    2          BLOCKS USED:  278          TOTAL BLOCKS:  280

```

## Utilities

The `util/` directory contains small programs that may be useful for working with files on the mounted disks or the disk images themselves. Three currently exist.

* `awp2txt`: Convert an AppleWorks word processor file to text.
* `wpf2txt`: Convert a MultiScribe word processor file to text.
* `prodosify`: Write disk image to disk if it required modifications (e.g. converting from track-and-sector to block-oriented) in order to mount.

# To Do

- [ ] Comment code more.
- [ ] ~~Mount as read-only~~ Seems FUSE does not support this.
- [ ] Add option to use file type as file extension.
- [ ] Add option to disable ProDOS-to-Unix text file translation.
- [ ] Add option to control pseudo-files support.
- [ ] Write utility to de-tokenize Applesoft BASIC programs to text
