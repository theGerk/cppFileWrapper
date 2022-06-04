#ifndef INCLUDE_GERK_FILE
#define INCLUDE_GERK_FILE

#include <optional>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace Gerk
{
	class File
	{
	public:
		enum class OpenMode : int
		{
			/*
			The file is opened in append mode.  Before  each  write(2),  the
			file  offset  is  positioned  at the end of the file, as if with
			lseek(2).  The modification of the file offset and the write op‐
			eration are performed as a single atomic step.

			O_APPEND  may lead to corrupted files on NFS filesystems if more
			than one process appends data to a file at once.   This  is  be‐
			cause  NFS  does  not support appending to a file, so the client
			kernel has to simulate it, which can't be done  without  a  race
			condition.
			*/
			Append = O_APPEND,

			/*
			Enable  signal-driven  I/O: generate a signal (SIGIO by default,
			but this can be changed via fcntl(2)) when input or  output  be‐
			comes  possible on this file descriptor.  This feature is avail‐
			able only for terminals, pseudoterminals,  sockets,  and  (since
			Linux  2.6)  pipes and FIFOs.  See fcntl(2) for further details.
			See also BUGS, below.
			*/
			Async = O_ASYNC,

			/*
			Enable the close-on-exec  flag  for  the  new  file  descriptor.
			Specifying  this  flag permits a program to avoid additional fc‐
			ntl(2) F_SETFD operations to set the FD_CLOEXEC flag.

			Note that the use of this  flag  is  essential  in  some  multi‐
			threaded programs, because using a separate fcntl(2) F_SETFD op‐
			eration to set the FD_CLOEXEC flag does  not  suffice  to  avoid
			race conditions where one thread opens a file descriptor and at‐
			tempts to set its close-on-exec flag using fcntl(2) at the  same
			time as another thread does a fork(2) plus execve(2).  Depending
			on the order of execution, the race may lead  to  the  file  de‐
			scriptor  returned by open() being unintentionally leaked to the
			program executed by the child process created by fork(2).  (This
			kind  of  race is in principle possible for any system call that
			creates a file descriptor whose  close-on-exec  flag  should  be
			set,  and various other Linux system calls provide an equivalent
			of the O_CLOEXEC flag to deal with this problem.)
			*/
			CloseOnExecute = O_CLOEXEC,

			/*
			If pathname does not exist, create it as a regular file.

			The owner (user ID) of the new file is set to the effective user
			ID of the process.

			The  group ownership (group ID) of the new file is set either to
			the effective group ID of the process (System V semantics) or to
			the group ID of the parent directory (BSD semantics).  On Linux,
			the behavior depends on whether the set-group-ID mode bit is set
			on  the parent directory: if that bit is set, then BSD semantics
			apply; otherwise, System V semantics apply.  For  some  filesys‐
			tems,  the behavior also depends on the bsdgroups and sysvgroups
			mount options described in mount(8)).

			The mode argument specifies the file mode bits be applied when a
			new  file  is  created.   This  argument  must  be supplied when
			O_CREAT or O_TMPFILE is specified in flags; if  neither  O_CREAT
			nor O_TMPFILE is specified, then mode is ignored.  The effective
			mode is modified by the process's umask in the usual way: in the
			absence  of  a  default  ACL,  the  mode  of the created file is
			(mode & ~umask).  Note that this mode applies only to future ac‐
			cesses of the newly created file; the open() call that creates a
			read-only file may well return a read/write file descriptor.
			*/
			Create = O_CREAT,

			/*
			Try  to minimize cache effects of the I/O to and from this file.
			In general this will degrade performance, but it  is  useful  in
			special  situations,  such  as  when  applications  do their own
			caching.  File I/O is done directly to/from user-space  buffers.
			The  O_DIRECT  flag  on its own makes an effort to transfer data
			synchronously, but does not give the guarantees  of  the  O_SYNC
			flag that data and necessary metadata are transferred.  To guar‐
			antee synchronous I/O, O_SYNC must be used in addition to  O_DI‐
			RECT.  See NOTES below for further discussion.

			A  semantically similar (but deprecated) interface for block de‐
			vices is described in raw(8).
			*/
			Direct = O_DIRECT,

			/*
			If pathname is not a directory, cause the open  to  fail.   This
			flag  was  added  in kernel version 2.1.126, to avoid denial-of-
			service problems if opendir(3) is called on a FIFO or  tape  de‐
			vice.
			*/
			Directory = O_DIRECTORY,

			/*
			Write  operations on the file will complete according to the re‐
			quirements of synchronized I/O data integrity completion.

			By the time write(2) (and similar) return, the output  data  has
			been transferred to the underlying hardware, along with any file
			metadata that would be required to retrieve that data (i.e.,  as
			though  each  write(2)  was followed by a call to fdatasync(2)).
			See NOTES below.
			*/
			Dsync = O_DSYNC,

			/*
			Ensure that this call creates the file: if this flag  is  speci‐
			fied  in  conjunction with O_CREAT, and pathname already exists,
			then open() fails with the error EEXIST.

			When these two flags are specified, symbolic links are not  fol‐
			lowed: if pathname is a symbolic link, then open() fails regard‐
			less of where the symbolic link points.

			In general, the behavior of O_EXCL is undefined if  it  is  used
			without  O_CREAT.   There  is  one  exception:  on Linux 2.6 and
			later, O_EXCL can be used without O_CREAT if pathname refers  to
			a  block  device.   If  the block device is in use by the system
			(e.g., mounted), open() fails with the error EBUSY.

			On NFS, O_EXCL is supported only when using NFSv3  or  later  on
			kernel  2.6  or later.  In NFS environments where O_EXCL support
			is not provided, programs that rely on it for performing locking
			tasks  will  contain  a  race condition.  Portable programs that
			want to perform atomic file locking using a lockfile,  and  need
			to avoid reliance on NFS support for O_EXCL, can create a unique
			file on the same filesystem (e.g.,  incorporating  hostname  and
			PID),  and  use  link(2)  to  make  a  link to the lockfile.  If
			link(2) returns 0,  the  lock  is  successful.   Otherwise,  use
			stat(2)  on  the  unique file to check if its link count has in‐
			creased to 2, in which case the lock is also successful.
			*/
			Exclusive = O_EXCL,

			/*
			(LFS) Allow files whose sizes cannot be represented in an  off_t
			(but  can  be  represented  in  an  off64_t)  to be opened.  The
			_LARGEFILE64_SOURCE macro must be defined (before including  any
			header  files)  in order to obtain this definition.  Setting the
			_FILE_OFFSET_BITS feature test macro to 64  (rather  than  using
			O_LARGEFILE) is the preferred method of accessing large files on
			32-bit systems (see feature_test_macros(7)).
			*/
			LargeFile = O_LARGEFILE,

			/*
			Do not update the file last access time (st_atime in the  inode)
			when the file is read(2).

			This  flag  can  be employed only if one of the following condi‐
			tions is true:

			*  The effective UID of the process matches the owner UID of the
				file.

			*  The calling process has the CAP_FOWNER capability in its user
				namespace and the owner UID of the file has a mapping in  the
				namespace.

			This  flag  is  intended for use by indexing or backup programs,
			where its use can significantly reduce the amount of disk activ‐
			ity.   This  flag  may not be effective on all filesystems.  One
			example is NFS, where the server maintains the access time.
			*/
			NoAccessTime = O_NOATIME,

			/*
			If pathname refers to a terminal device—see tty(4)—it  will  not
			become  the  process's  controlling terminal even if the process
			does not have one.
			*/
			NoControllingTerminal = O_NOCTTY,

			/*
			If pathname is a symbolic link, then the open  fails,  with  the
			error  ELOOP.  Symbolic links in earlier components of the path‐
			name will still be followed.  (Note that the  ELOOP  error  that
			can  occur in this case is indistinguishable from the case where
			an open fails because there are too many  symbolic  links  found
			while resolving components in the prefix part of the pathname.)

			This  flag  is  a FreeBSD extension, which was added to Linux in
			version 2.1.126,  and  has  subsequently  been  standardized  in
			POSIX.1-2008.

			See also O_PATH below.
			*/
			NoFollow = O_NOFOLLOW,

			/*
			When  possible, the file is opened in nonblocking mode.  Neither
			the open() nor any subsequent I/O operations  on  the  file  de‐
			scriptor  which  is  returned  will cause the calling process to
			wait.

			Note that the setting of this flag has no effect on  the  opera‐
			tion  of  poll(2), select(2), epoll(7), and similar, since those
			interfaces merely inform the caller about  whether  a  file  de‐
			scriptor  is "ready", meaning that an I/O operation performed on
			the file descriptor with the O_NONBLOCK  flag  clear  would  not
			block.

			Note  that  this  flag has no effect for regular files and block
			devices; that is, I/O operations will (briefly) block  when  de‐
			vice  activity  is required, regardless of whether O_NONBLOCK is
			set.  Since O_NONBLOCK  semantics  might  eventually  be  imple‐
			mented,  applications  should  not depend upon blocking behavior
			when specifying this flag for regular files and block devices.

			For the handling of FIFOs (named pipes), see also fifo(7).   For
			a  discussion  of  the  effect of O_NONBLOCK in conjunction with
			mandatory file locks and with file leases, see fcntl(2).
			*/
			NonBlocking = O_NONBLOCK,

			/*
			Obtain a file descriptor that can be used for two  purposes:  to
			indicate a location in the filesystem tree and to perform opera‐
			tions that act purely at the file descriptor  level.   The  file
			itself  is not opened, and other file operations (e.g., read(2),
			write(2), fchmod(2), fchown(2), fgetxattr(2), ioctl(2), mmap(2))
			fail with the error EBADF.

			The  following operations can be performed on the resulting file
			descriptor:

			*  close(2).

			*  fchdir(2), if the  file  descriptor  refers  to  a  directory
				(since Linux 3.5).

			*  fstat(2) (since Linux 3.6).

			*  fstatfs(2) (since Linux 3.12).

			*  Duplicating  the  file  descriptor (dup(2), fcntl(2) F_DUPFD,
				etc.).

			*  Getting and setting file descriptor flags  (fcntl(2)  F_GETFD
				and F_SETFD).

			*  Retrieving  open file status flags using the fcntl(2) F_GETFL
				operation: the returned flags will include the bit O_PATH.

			*  Passing the file descriptor as the dirfd argument of openat()
				and  the other "*at()" system calls.  This includes linkat(2)
				with AT_EMPTY_PATH (or via  procfs  using  AT_SYMLINK_FOLLOW)
				even if the file is not a directory.

			*  Passing the file descriptor to another process via a UNIX do‐
				main socket (see SCM_RIGHTS in unix(7)).

			When  O_PATH  is  specified  in  flags,  flag  bits  other  than
			O_CLOEXEC, O_DIRECTORY, and O_NOFOLLOW are ignored.

			Opening  a  file  or  directory with the O_PATH flag requires no
			permissions on the object itself (but does require execute  per‐
			mission  on  the  directories in the path prefix).  Depending on
			the subsequent operation, a check for suitable file  permissions
			may be performed (e.g., fchdir(2) requires execute permission on
			the directory referred to by its file descriptor argument).   By
			contrast,  obtaining a reference to a filesystem object by open‐
			ing it with the O_RDONLY flag requires that the caller have read
			permission  on  the  object,  even when the subsequent operation
			(e.g., fchdir(2), fstat(2)) does not require read permission  on
			the object.

			If  pathname  is a symbolic link and the O_NOFOLLOW flag is also
			specified, then the call returns a file descriptor referring  to
			the  symbolic  link.   This  file  descriptor can be used as the
			dirfd argument in calls to fchownat(2),  fstatat(2),  linkat(2),
			and readlinkat(2) with an empty pathname to have the calls oper‐
			ate on the symbolic link.

			If pathname refers to an automount point that has not  yet  been
			triggered,  so  no  other  filesystem is mounted on it, then the
			call returns a file descriptor referring to the automount direc‐
			tory without triggering a mount.  fstatfs(2) can then be used to
			determine if it is, in  fact,  an  untriggered  automount  point
			(.f_type == AUTOFS_SUPER_MAGIC).

			One use of O_PATH for regular files is to provide the equivalent
			of POSIX.1's O_EXEC functionality.  This permits us  to  open  a
			file  for  which we have execute permission but not read permis‐
			sion, and then execute that file, with steps something like  the
			following:

				char buf[PATH_MAX];
				fd = open("some_prog", O_PATH);
				snprintf(buf, PATH_MAX, "/proc/self/fd/%d", fd);
				execl(buf, "some_prog", (char *) NULL);

			An  O_PATH file descriptor can also be passed as the argument of
			fexecve(3).
			*/
			Path = O_PATH,

			/*
			Write operations on the file will complete according to the  re‐
			quirements  of  synchronized  I/O  file integrity completion (by
			contrast with the synchronized  I/O  data  integrity  completion
			provided by O_DSYNC.)

			By  the  time write(2) (or similar) returns, the output data and
			associated file metadata have been transferred to the underlying
			hardware  (i.e.,  as though each write(2) was followed by a call
			to fsync(2)).  See NOTES below.
			*/
			Synchronous = O_SYNC,

			/*
			Create an unnamed temporary regular file.  The pathname argument specifies a  direc‐
			tory;  an  unnamed  inode  will be created in that directory's filesystem.  Anything
			written to the resulting file will be lost when the last file descriptor is  closed,
			unless the file is given a name.

			O_TMPFILE  must be specified with one of O_RDWR or O_WRONLY and, optionally, O_EXCL.
			If O_EXCL is not specified, then linkat(2) can be used to link  the  temporary  file
			into the filesystem, making it permanent, using code like the following:

				char path[PATH_MAX];
				fd = open("/path/to/dir", O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR);

				linkat(fd, NULL, AT_FDCWD, "/path/for/file", AT_EMPTY_PATH);

			If the caller doesn't have the CAP_DAC_READ_SEARCH
			capability (needed to use AT_EMPTY_PATH with linkat(2)),
			and there is a proc(5) filesystem mounted, then the
			linkat(2) call above can be replaced with:

				snprintf(path, PATH_MAX,  "/proc/self/fd/%d", fd);
				linkat(AT_FDCWD, path, AT_FDCWD, "/path/for/file",
										AT_SYMLINK_FOLLOW);

			In  this case, the open() mode argument determines the file permission mode, as with
			O_CREAT.

			Specifying O_EXCL in conjunction with O_TMPFILE prevents a temporary file from being
			linked into the filesystem in the above manner.  (Note that the meaning of O_EXCL in
			this case is different from the meaning of O_EXCL otherwise.)

			There are two main use cases for O_TMPFILE:

			*  Improved tmpfile(3) functionality: race-free creation of temporary files that (1)
				are automatically deleted when closed; (2) can never be reached via any pathname;
				(3) are not subject to symlink attacks; and (4) do not require the caller to  de‐
				vise unique names.

			*  Creating  a  file  that is initially invisible, which is then populated with data
				and adjusted to have appropriate  filesystem  attributes  (fchown(2),  fchmod(2),
				fsetxattr(2),  etc.)   before  being  atomically  linked into the filesystem in a
				fully formed state (using linkat(2) as described above).

			O_TMPFILE requires support by the underlying filesystem;  only  a  subset  of  Linux
			filesystems  provide  that support.  In the initial implementation, support was pro‐
			vided in the ext2, ext3, ext4, UDF, Minix, and shmem filesystems.  Support for other
			filesystems  has  subsequently been added as follows: XFS (Linux 3.15); Btrfs (Linux
			3.16); F2FS (Linux 3.16); and ubifs (Linux 4.9)
			*/
			Tempfile = O_TMPFILE,

			/*
			If  the file already exists and is a regular file and the access
			mode allows writing (i.e., is O_RDWR or  O_WRONLY)  it  will  be
			truncated to length 0.  If the file is a FIFO or terminal device
			file, the O_TRUNC flag is ignored.   Otherwise,  the  effect  of
			O_TRUNC is unspecified.
			*/
			Truncate = O_TRUNC,
		};

		enum class Permissions : mode_t
		{
			UserRead = S_IRUSR,
			UserWrite = S_IWUSR,
			UserExecute = S_IXUSR,
			GroupRead = S_IRGRP,
			GroupWrite = S_IWGRP,
			GroupExecute = S_IXGRP,
			OtherRead = S_IROTH,
			OtherWrite = S_IWOTH,
			OtherExecute = S_IXOTH,

			SetUserId = S_ISUID,
			SetGroupId = S_ISGID,
			Sticky = S_ISVTX,
		};

	private:
		int fd;

	public:
		File(const std::string &path, OpenMode flags, Permissions mode = (Permissions)0);
		File(File &&other);
		~File();

		std::optional<std::string> readLine();
		File &write(const std::string &data);
		File &write(const char *const data, const size_t bytes);

		// Can be used as mutex for with std::lock_gaurd<mutex> and the like
		void lock();
		void unlock();
	};
} // namespace Gerk

#endif // INCLUDE_GERK_FILE