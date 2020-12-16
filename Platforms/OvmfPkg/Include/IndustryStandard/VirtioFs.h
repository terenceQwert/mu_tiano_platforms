/** @file
  Type and macro definitions specific to the Virtio Filesystem device.

  At the time of this writing, the latest released Virtio specification (v1.1)
  does not include the virtio-fs device. The development version of the
  specification defines it however; see the latest version at
  <https://github.com/oasis-tcs/virtio-spec/blob/87fa6b5d8155/virtio-fs.tex>.

  This header file is minimal, and only defines the types and macros that are
  necessary for the OvmfPkg implementation.

  Copyright (C) 2020, Red Hat, Inc.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef VIRTIO_FS_H_
#define VIRTIO_FS_H_

#include <IndustryStandard/Virtio.h>

//
// Lowest numbered queue for sending normal priority requests.
//
#define VIRTIO_FS_REQUEST_QUEUE 1

//
// Number of bytes in the "VIRTIO_FS_CONFIG.Tag" field.
//
#define VIRTIO_FS_TAG_BYTES 36

//
// Device configuration layout.
//
#pragma pack (1)
typedef struct {
  //
  // The Tag field can be considered the filesystem label, or a mount point
  // hint. It is UTF-8 encoded, and padded to full size with NUL bytes. If the
  // encoded bytes take up the entire Tag field, then there is no NUL
  // terminator.
  //
  UINT8 Tag[VIRTIO_FS_TAG_BYTES];
  //
  // The total number of request virtqueues exposed by the device (i.e.,
  // excluding the "hiprio" queue).
  //
  UINT32 NumReqQueues;
} VIRTIO_FS_CONFIG;
#pragma pack ()

//
// FUSE-related definitions follow.
//
// From virtio-v1.1-cs01-87fa6b5d8155, 5.11 File System Device: "[...] The
// driver acts as the FUSE client mounting the file system. The virtio file
// system device provides the mechanism for transporting FUSE requests [...]"
//
// Unfortunately, the documentation of the FUSE wire protocol is lacking. The
// Virtio spec (as of this writing) simply defers to
// "include/uapi/linux/fuse.h" in the Linux kernel source -- see the reference
// in virtio spec file "introduction.tex", at commit 87fa6b5d8155.
//
// Of course, "include/uapi/linux/fuse.h" is a moving target (the virtio spec
// does not specify a particular FUSE interface version). The OvmfPkg code
// targets version 7.31, because that's the lowest version that the QEMU
// virtio-fs daemon supports at this time -- see QEMU commit 72c42e2d6551
// ("virtiofsd: Trim out compatibility code", 2020-01-23).
//
// Correspondingly, Linux's "include/uapi/linux/fuse.h" is consulted as checked
// out at commit (c6ff213fe5b8^) = d78092e4937d ("fuse: fix page dereference
// after free", 2020-09-18); that is, right before commit c6ff213fe5b8 ("fuse:
// add submount support to <uapi/linux/fuse.h>", 2020-09-18) introduces FUSE
// interface version 7.32.
//
#define VIRTIO_FS_FUSE_MAJOR  7
#define VIRTIO_FS_FUSE_MINOR 31

//
// The inode number of the root directory.
//
#define VIRTIO_FS_FUSE_ROOT_DIR_NODE_ID 1

//
// Distinguished errno values.
//
#define VIRTIO_FS_FUSE_ERRNO_ENOENT (-2)

//
// File mode bitmasks.
//
#define VIRTIO_FS_FUSE_MODE_TYPE_MASK 0170000u
#define VIRTIO_FS_FUSE_MODE_TYPE_REG  0100000u
#define VIRTIO_FS_FUSE_MODE_TYPE_DIR  0040000u
#define VIRTIO_FS_FUSE_MODE_PERM_RWXU 0000700u
#define VIRTIO_FS_FUSE_MODE_PERM_RUSR 0000400u
#define VIRTIO_FS_FUSE_MODE_PERM_WUSR 0000200u
#define VIRTIO_FS_FUSE_MODE_PERM_RWXG 0000070u
#define VIRTIO_FS_FUSE_MODE_PERM_RGRP 0000040u
#define VIRTIO_FS_FUSE_MODE_PERM_WGRP 0000020u
#define VIRTIO_FS_FUSE_MODE_PERM_RWXO 0000007u
#define VIRTIO_FS_FUSE_MODE_PERM_ROTH 0000004u
#define VIRTIO_FS_FUSE_MODE_PERM_WOTH 0000002u

//
// Flags for VirtioFsFuseOpOpen.
//
#define VIRTIO_FS_FUSE_OPEN_REQ_F_RDONLY 0
#define VIRTIO_FS_FUSE_OPEN_REQ_F_RDWR   2

//
// FUSE operation codes.
//
typedef enum {
  VirtioFsFuseOpLookup      =  1,
  VirtioFsFuseOpForget      =  2,
  VirtioFsFuseOpMkDir       =  9,
  VirtioFsFuseOpUnlink      = 10,
  VirtioFsFuseOpRmDir       = 11,
  VirtioFsFuseOpOpen        = 14,
  VirtioFsFuseOpRelease     = 18,
  VirtioFsFuseOpFsync       = 20,
  VirtioFsFuseOpFlush       = 25,
  VirtioFsFuseOpInit        = 26,
  VirtioFsFuseOpOpenDir     = 27,
  VirtioFsFuseOpReleaseDir  = 29,
  VirtioFsFuseOpFsyncDir    = 30,
  VirtioFsFuseOpCreate      = 35,
} VIRTIO_FS_FUSE_OPCODE;

#pragma pack (1)
//
// Request-response headers common to all request types.
//
typedef struct {
  UINT32 Len;
  UINT32 Opcode;
  UINT64 Unique;
  UINT64 NodeId;
  UINT32 Uid;
  UINT32 Gid;
  UINT32 Pid;
  UINT32 Padding;
} VIRTIO_FS_FUSE_REQUEST;

typedef struct {
  UINT32 Len;
  INT32  Error;
  UINT64 Unique;
} VIRTIO_FS_FUSE_RESPONSE;

//
// Structure with which the Virtio Filesystem device reports a NodeId to the
// FUSE client (i.e., to the Virtio Filesystem driver). This structure is a
// part of the response headers for operations that inform the FUSE client of
// an inode.
//
typedef struct {
  UINT64 NodeId;
  UINT64 Generation;
  UINT64 EntryValid;
  UINT64 AttrValid;
  UINT32 EntryValidNsec;
  UINT32 AttrValidNsec;
} VIRTIO_FS_FUSE_NODE_RESPONSE;

//
// Structure describing the host-side attributes of an inode. This structure is
// a part of the response headers for operations that inform the FUSE client of
// an inode.
//
typedef struct {
  UINT64 Ino;
  UINT64 Size;
  UINT64 Blocks;
  UINT64 Atime;
  UINT64 Mtime;
  UINT64 Ctime;
  UINT32 AtimeNsec;
  UINT32 MtimeNsec;
  UINT32 CtimeNsec;
  UINT32 Mode;
  UINT32 Nlink;
  UINT32 Uid;
  UINT32 Gid;
  UINT32 Rdev;
  UINT32 Blksize;
  UINT32 Padding;
} VIRTIO_FS_FUSE_ATTRIBUTES_RESPONSE;

//
// Header for VirtioFsFuseOpForget.
//
typedef struct {
  UINT64 NumberOfLookups;
} VIRTIO_FS_FUSE_FORGET_REQUEST;

//
// Header for VirtioFsFuseOpMkDir.
//
typedef struct {
  UINT32 Mode;
  UINT32 Umask;
} VIRTIO_FS_FUSE_MKDIR_REQUEST;

//
// Headers for VirtioFsFuseOpOpen and VirtioFsFuseOpOpenDir.
//
typedef struct {
  UINT32 Flags;
  UINT32 Unused;
} VIRTIO_FS_FUSE_OPEN_REQUEST;

typedef struct {
  UINT64 FileHandle;
  UINT32 OpenFlags;
  UINT32 Padding;
} VIRTIO_FS_FUSE_OPEN_RESPONSE;

//
// Header for VirtioFsFuseOpRelease and VirtioFsFuseOpReleaseDir.
//
typedef struct {
  UINT64 FileHandle;
  UINT32 Flags;
  UINT32 ReleaseFlags;
  UINT64 LockOwner;
} VIRTIO_FS_FUSE_RELEASE_REQUEST;

//
// Header for VirtioFsFuseOpFsync and VirtioFsFuseOpFsyncDir.
//
typedef struct {
  UINT64 FileHandle;
  UINT32 FsyncFlags;
  UINT32 Padding;
} VIRTIO_FS_FUSE_FSYNC_REQUEST;

//
// Header for VirtioFsFuseOpFlush.
//
typedef struct {
  UINT64 FileHandle;
  UINT32 Unused;
  UINT32 Padding;
  UINT64 LockOwner;
} VIRTIO_FS_FUSE_FLUSH_REQUEST;

//
// Headers for VirtioFsFuseOpInit.
//
typedef struct {
  UINT32 Major;
  UINT32 Minor;
  UINT32 MaxReadahead;
  UINT32 Flags;
} VIRTIO_FS_FUSE_INIT_REQUEST;

typedef struct {
  UINT32 Major;
  UINT32 Minor;
  UINT32 MaxReadahead;
  UINT32 Flags;
  UINT16 MaxBackground;
  UINT16 CongestionThreshold;
  UINT32 MaxWrite;
  UINT32 TimeGran;
  UINT16 MaxPages;
  UINT16 MapAlignment;
  UINT32 Unused[8];
} VIRTIO_FS_FUSE_INIT_RESPONSE;

//
// Header for VirtioFsFuseOpCreate.
//
typedef struct {
  UINT32 Flags;
  UINT32 Mode;
  UINT32 Umask;
  UINT32 Padding;
} VIRTIO_FS_FUSE_CREATE_REQUEST;
#pragma pack ()

#endif // VIRTIO_FS_H_
