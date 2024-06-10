#
# prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
#
# Copyright 2024 by Javier Alvarado.
#

cmake_minimum_required(VERSION 3.28)
project(prodosfs)

set(CMAKE_CXX_STANDARD 17)

include_directories("/usr/include/fuse3")
link_libraries("/usr/lib64/libfuse3.so")

set(CMAKE_CXX_FLAGS "-Wno-pointer-arith")

include_directories("include")
add_executable(
    prodosfs

    main.cxx
    include/prodos.hxx
    include/prodos/block.hxx
    include/prodos/directory.hxx
    include/prodos/disk.hxx
    include/prodos/entry.hxx
    include/prodos/file.hxx
    include/prodos/filetype.hxx
    include/prodos/util.hxx
    include/prodos/volume.hxx
    source/directory.cxx
    source/disk.cxx
    source/entry.cxx
    source/file.cxx
    source/filetype.cxx
    source/util.cxx
    source/volume.cxx
)
