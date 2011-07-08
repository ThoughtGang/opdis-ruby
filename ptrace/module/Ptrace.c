/* Ptrace.c
 * Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <string.h>
#include <signal.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>

// TODO: how to represent a 128-bit number in C and ruby
// gcc: unsigned __int128 
//  BigDecimal.new("0.0001")
//   require 'bigdecimal'

#include <ruby.h>
#include "ruby_compat.h"

#include "Ptrace.h"

#define IVAR(attr) "@" attr

// TODO: decode user data structure ala sys/user.h

static VALUE modPtrace;
static VALUE clsDebug;
static VALUE clsError;


static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, rb_intern("to_sym"), 0);
}

static VALUE pid_to_num( pid_t pid ) {
	return (sizeof(pid_t) == sizeof(unsigned int)) ? UINT2NUM(pid)
							: ULL2NUM(pid);
}

static pid_t num_to_pid( VALUE num ) {
	if ( num == Qnil ) {
		return 0;
	}

	return (sizeof(pid_t) == sizeof(unsigned int)) ? (pid_t) NUM2UINT(num)
							: (pid_t) NUM2ULL(num);
}

/* return value for key or 0 */
static VALUE hash_get_int( VALUE h, VALUE key ) {

	VALUE v = rb_hash_aref( h, key );
	return (v == Qnil) ? UINT2NUM(0) : v;
}

#define CMD_HASH_ADD(h, str, val) \
	rb_hash_aset(h, str_to_sym(str), UINT2NUM(val));

static VALUE build_cmd_hash( VALUE cls ) {
	VALUE h = rb_hash_new();

	CMD_HASH_ADD(h, SZ_PTRACE_TRACEME, PT_TRACE_ME) 
	CMD_HASH_ADD(h, SZ_PTRACE_PEEKTEXT, PT_READ_I)
	CMD_HASH_ADD(h, SZ_PTRACE_PEEKDATA, PT_READ_D) 
	CMD_HASH_ADD(h, SZ_PTRACE_POKETEXT, PT_WRITE_I)
	CMD_HASH_ADD(h, SZ_PTRACE_POKEDATA, PT_WRITE_D)
	CMD_HASH_ADD(h, SZ_PTRACE_CONT, PT_CONTINUE)
	CMD_HASH_ADD(h, SZ_PTRACE_SINGLESTEP, PT_STEP)
	CMD_HASH_ADD(h, SZ_PTRACE_KILL, PT_KILL)
	CMD_HASH_ADD(h, SZ_PTRACE_ATTACH, PT_ATTACH)
	CMD_HASH_ADD(h, SZ_PTRACE_DETACH, PT_DETACH)

	/* ====================================================== */
	/* These may not be present in all ptrace implementations */
#ifdef PT_WRITE_U
	CMD_HASH_ADD(h, SZ_PTRACE_POKEUSR, PT_WRITE_U)
#endif
#ifdef PT_READ_U
	CMD_HASH_ADD(h, SZ_PTRACE_PEEKUSR, PT_READ_U)
#endif
#ifdef PT_SYSCALL
	CMD_HASH_ADD(h, SZ_PTRACE_SYSCALL, PT_SYSCALL)
#endif
#ifdef PT_GETREGS
	CMD_HASH_ADD(h, SZ_PTRACE_GETREGS, PT_GETREGS)
#endif
#ifdef PT_GETFPREGS
	CMD_HASH_ADD(h, SZ_PTRACE_GETFPREGS, PT_GETFPREGS)
#endif
#ifdef PT_GETFPXREGS
	CMD_HASH_ADD(h, SZ_PTRACE_GETFPXREGS, PT_GETFPXREGS)
#endif
#ifdef PT_SETREGS
	CMD_HASH_ADD(h, SZ_PTRACE_SETREGS, PT_SETREGS)
#endif
#ifdef PT_SETFPREGS
	CMD_HASH_ADD(h, SZ_PTRACE_SETFPREGS, PT_SETFPREGS)
#endif
#ifdef PT_SETFPXREGS
	CMD_HASH_ADD(h, SZ_PTRACE_SETFPXREGS, PT_SETFPXREGS)
#endif
#ifdef PT_SETOPTIONS
	CMD_HASH_ADD(h, SZ_PTRACE_SETOPTIONS, PT_SETOPTIONS)
#endif
#ifdef PT_GETSIGINFO
	CMD_HASH_ADD(h, SZ_PTRACE_GETSIGINFO, PT_GETSIGINFO)
#endif
#ifdef PT_SETSIGINFO
	CMD_HASH_ADD(h, SZ_PTRACE_SETSIGINFO, PT_SETSIGINFO)
#endif
#ifdef PT_GETEVENTMSG
	CMD_HASH_ADD(h, SZ_PTRACE_GETEVENTMSG, PT_GETEVENTMSG)
#endif
#ifdef PT_SYSEMU
	CMD_HASH_ADD(h, SZ_PTRACE_SYSEMU, PT_SYSEMU)
#endif
#ifdef PT_SYSEMU_SINGLESTEP
	CMD_HASH_ADD(h, SZ_PTRACE_SYSEMU_SINGLESTEP, PT_SYSEMU_SINGLESTEP)
#endif
	/* ====================================================== */

	return h;
}

#define SYS_HASH_ADD(h, str, val) \
	rb_hash_aset(h, UINT2NUM(val), str_to_sym(str));

static VALUE build_syscall_hash( VALUE cls ) {
	VALUE h = rb_hash_new();

	/* NOTE: these have been obtained from bits/syscall.h on Linux 2.6.x */
#ifdef SYS__llseek
	SYS_HASH_ADD(h, "llseek", SYS__llseek);
#endif
#ifdef SYS__newselect
	SYS_HASH_ADD(h, "newselect", SYS__newselect);
#endif
#ifdef SYS__sysctl
	SYS_HASH_ADD(h, "sysctl", SYS__sysctl);
#endif
#ifdef SYS_accept
	SYS_HASH_ADD(h, "accept", SYS_accept);
#endif
#ifdef SYS_accept4
	SYS_HASH_ADD(h, "accept4", SYS_accept4);
#endif
#ifdef SYS_access
	SYS_HASH_ADD(h, "access", SYS_access);
#endif
#ifdef SYS_acct
	SYS_HASH_ADD(h, "acct", SYS_acct);
#endif
#ifdef SYS_add_key
	SYS_HASH_ADD(h, "add_key", SYS_add_key);
#endif
#ifdef SYS_adjtimex
	SYS_HASH_ADD(h, "adjtimex", SYS_adjtimex);
#endif
#ifdef SYS_afs_syscall
	SYS_HASH_ADD(h, "afs_syscall", SYS_afs_syscall);
#endif
#ifdef SYS_alarm
	SYS_HASH_ADD(h, "alarm", SYS_alarm);
#endif
#ifdef SYS_arch_prctl
	SYS_HASH_ADD(h, "arch_prctl", SYS_arch_prctl);
#endif
#ifdef SYS_bdflush
	SYS_HASH_ADD(h, "bdflush", SYS_bdflush);
#endif
#ifdef SYS_bind
	SYS_HASH_ADD(h, "bind", SYS_bind);
#endif
#ifdef SYS_break
	SYS_HASH_ADD(h, "break", SYS_break);
#endif
#ifdef SYS_brk
	SYS_HASH_ADD(h, "brk", SYS_brk);
#endif
#ifdef SYS_capget
	SYS_HASH_ADD(h, "capget", SYS_capget);
#endif
#ifdef SYS_capset
	SYS_HASH_ADD(h, "capset", SYS_capset);
#endif
#ifdef SYS_chdir
	SYS_HASH_ADD(h, "chdir", SYS_chdir);
#endif
#ifdef SYS_chmod
	SYS_HASH_ADD(h, "chmod", SYS_chmod);
#endif
#ifdef SYS_chown
	SYS_HASH_ADD(h, "chown", SYS_chown);
#endif
#ifdef SYS_chown32
	SYS_HASH_ADD(h, "chown32", SYS_chown32);
#endif
#ifdef SYS_chroot
	SYS_HASH_ADD(h, "chroot", SYS_chroot);
#endif
#ifdef SYS_clock_getres
	SYS_HASH_ADD(h, "clock_getres", SYS_clock_getres);
#endif
#ifdef SYS_clock_gettime
	SYS_HASH_ADD(h, "clock_gettime", SYS_clock_gettime);
#endif
#ifdef SYS_clock_nanosleep
	SYS_HASH_ADD(h, "clock_nanosleep", SYS_clock_nanosleep);
#endif
#ifdef SYS_clock_settime
	SYS_HASH_ADD(h, "clock_settime", SYS_clock_settime);
#endif
#ifdef SYS_clone
	SYS_HASH_ADD(h, "clone", SYS_clone);
#endif
#ifdef SYS_close
	SYS_HASH_ADD(h, "close", SYS_close);
#endif
#ifdef SYS_connect
	SYS_HASH_ADD(h, "connect", SYS_connect);
#endif
#ifdef SYS_creat
	SYS_HASH_ADD(h, "creat", SYS_creat);
#endif
#ifdef SYS_create_module
	SYS_HASH_ADD(h, "create_module", SYS_create_module);
#endif
#ifdef SYS_delete_module
	SYS_HASH_ADD(h, "delete_module", SYS_delete_module);
#endif
#ifdef SYS_dup
	SYS_HASH_ADD(h, "dup", SYS_dup);
#endif
#ifdef SYS_dup2
	SYS_HASH_ADD(h, "dup2", SYS_dup2);
#endif
#ifdef SYS_dup3
	SYS_HASH_ADD(h, "dup3", SYS_dup3);
#endif
#ifdef SYS_epoll_create
	SYS_HASH_ADD(h, "epoll_create", SYS_epoll_create);
#endif
#ifdef SYS_epoll_create1
	SYS_HASH_ADD(h, "epoll_create1", SYS_epoll_create1);
#endif
#ifdef SYS_epoll_ctl
	SYS_HASH_ADD(h, "epoll_ctl", SYS_epoll_ctl);
#endif
#ifdef SYS_epoll_ctl_old
	SYS_HASH_ADD(h, "epoll_ctl_old", SYS_epoll_ctl_old);
#endif
#ifdef SYS_epoll_pwait
	SYS_HASH_ADD(h, "epoll_pwait", SYS_epoll_pwait);
#endif
#ifdef SYS_epoll_wait
	SYS_HASH_ADD(h, "epoll_wait", SYS_epoll_wait);
#endif
#ifdef SYS_epoll_wait_old
	SYS_HASH_ADD(h, "epoll_wait_old", SYS_epoll_wait_old);
#endif
#ifdef SYS_eventfd
	SYS_HASH_ADD(h, "eventfd", SYS_eventfd);
#endif
#ifdef SYS_eventfd2
	SYS_HASH_ADD(h, "eventfd2", SYS_eventfd2);
#endif
#ifdef SYS_execve
	SYS_HASH_ADD(h, "execve", SYS_execve);
#endif
#ifdef SYS_exit
	SYS_HASH_ADD(h, "exit", SYS_exit);
#endif
#ifdef SYS_exit_group
	SYS_HASH_ADD(h, "exit_group", SYS_exit_group);
#endif
#ifdef SYS_faccessat
	SYS_HASH_ADD(h, "faccessat", SYS_faccessat);
#endif
#ifdef SYS_fadvise64
	SYS_HASH_ADD(h, "fadvise64", SYS_fadvise64);
#endif
#ifdef SYS_fadvise64_64
	SYS_HASH_ADD(h, "fadvise64_64", SYS_fadvise64_64);
#endif
#ifdef SYS_fallocate
	SYS_HASH_ADD(h, "fallocate", SYS_fallocate);
#endif
#ifdef SYS_fanotify_init
	SYS_HASH_ADD(h, "fanotify_init", SYS_fanotify_init);
#endif
#ifdef SYS_fanotify_mark
	SYS_HASH_ADD(h, "fanotify_mark", SYS_fanotify_mark);
#endif
#ifdef SYS_fchdir
	SYS_HASH_ADD(h, "fchdir", SYS_fchdir);
#endif
#ifdef SYS_fchmod
	SYS_HASH_ADD(h, "fchmod", SYS_fchmod);
#endif
#ifdef SYS_fchmodat
	SYS_HASH_ADD(h, "fchmodat", SYS_fchmodat);
#endif
#ifdef SYS_fchown
	SYS_HASH_ADD(h, "fchown", SYS_fchown);
#endif
#ifdef SYS_fchown32
	SYS_HASH_ADD(h, "fchown32", SYS_fchown32);
#endif
#ifdef SYS_fchownat
	SYS_HASH_ADD(h, "fchownat", SYS_fchownat);
#endif
#ifdef SYS_fcntl
	SYS_HASH_ADD(h, "fcntl", SYS_fcntl);
#endif
#ifdef SYS_fcntl64
	SYS_HASH_ADD(h, "fcntl64", SYS_fcntl64);
#endif
#ifdef SYS_fdatasync
	SYS_HASH_ADD(h, "fdatasync", SYS_fdatasync);
#endif
#ifdef SYS_fgetxattr
	SYS_HASH_ADD(h, "fgetxattr", SYS_fgetxattr);
#endif
#ifdef SYS_flistxattr
	SYS_HASH_ADD(h, "flistxattr", SYS_flistxattr);
#endif
#ifdef SYS_flock
	SYS_HASH_ADD(h, "flock", SYS_flock);
#endif
#ifdef SYS_fork
	SYS_HASH_ADD(h, "fork", SYS_fork);
#endif
#ifdef SYS_fremovexattr
	SYS_HASH_ADD(h, "fremovexattr", SYS_fremovexattr);
#endif
#ifdef SYS_fsetxattr
	SYS_HASH_ADD(h, "fsetxattr", SYS_fsetxattr);
#endif
#ifdef SYS_fstat
	SYS_HASH_ADD(h, "fstat", SYS_fstat);
#endif
#ifdef SYS_fstat64
	SYS_HASH_ADD(h, "fstat64", SYS_fstat64);
#endif
#ifdef SYS_fstatat64
	SYS_HASH_ADD(h, "fstatat64", SYS_fstatat64);
#endif
#ifdef SYS_fstatfs
	SYS_HASH_ADD(h, "fstatfs", SYS_fstatfs);
#endif
#ifdef SYS_fstatfs64
	SYS_HASH_ADD(h, "fstatfs64", SYS_fstatfs64);
#endif
#ifdef SYS_fsync
	SYS_HASH_ADD(h, "fsync", SYS_fsync);
#endif
#ifdef SYS_ftime
	SYS_HASH_ADD(h, "ftime", SYS_ftime);
#endif
#ifdef SYS_ftruncate
	SYS_HASH_ADD(h, "ftruncate", SYS_ftruncate);
#endif
#ifdef SYS_ftruncate64
	SYS_HASH_ADD(h, "ftruncate64", SYS_ftruncate64);
#endif
#ifdef SYS_futex
	SYS_HASH_ADD(h, "futex", SYS_futex);
#endif
#ifdef SYS_futimesat
	SYS_HASH_ADD(h, "futimesat", SYS_futimesat);
#endif
#ifdef SYS_get_kernel_syms
	SYS_HASH_ADD(h, "get_kernel_syms", SYS_get_kernel_syms);
#endif
#ifdef SYS_get_mempolicy
	SYS_HASH_ADD(h, "get_mempolicy", SYS_get_mempolicy);
#endif
#ifdef SYS_get_robust_list
	SYS_HASH_ADD(h, "get_robust_list", SYS_get_robust_list);
#endif
#ifdef SYS_get_thread_area
	SYS_HASH_ADD(h, "get_thread_area", SYS_get_thread_area);
#endif
#ifdef SYS_getcpu
	SYS_HASH_ADD(h, "getcpu", SYS_getcpu);
#endif
#ifdef SYS_getcwd
	SYS_HASH_ADD(h, "getcwd", SYS_getcwd);
#endif
#ifdef SYS_getdents
	SYS_HASH_ADD(h, "getdents", SYS_getdents);
#endif
#ifdef SYS_getdents64
	SYS_HASH_ADD(h, "getdents64", SYS_getdents64);
#endif
#ifdef SYS_getegid
	SYS_HASH_ADD(h, "getegid", SYS_getegid);
#endif
#ifdef SYS_getegid32
	SYS_HASH_ADD(h, "getegid32", SYS_getegid32);
#endif
#ifdef SYS_geteuid
	SYS_HASH_ADD(h, "geteuid", SYS_geteuid);
#endif
#ifdef SYS_geteuid32
	SYS_HASH_ADD(h, "geteuid32", SYS_geteuid32);
#endif
#ifdef SYS_getgid
	SYS_HASH_ADD(h, "getgid", SYS_getgid);
#endif
#ifdef SYS_getgid32
	SYS_HASH_ADD(h, "getgid32", SYS_getgid32);
#endif
#ifdef SYS_getgroups
	SYS_HASH_ADD(h, "getgroups", SYS_getgroups);
#endif
#ifdef SYS_getgroups32
	SYS_HASH_ADD(h, "getgroups32", SYS_getgroups32);
#endif
#ifdef SYS_getitimer
	SYS_HASH_ADD(h, "getitimer", SYS_getitimer);
#endif
#ifdef SYS_getpeername
	SYS_HASH_ADD(h, "getpeername", SYS_getpeername);
#endif
#ifdef SYS_getpgid
	SYS_HASH_ADD(h, "getpgid", SYS_getpgid);
#endif
#ifdef SYS_getpgrp
	SYS_HASH_ADD(h, "getpgrp", SYS_getpgrp);
#endif
#ifdef SYS_getpid
	SYS_HASH_ADD(h, "getpid", SYS_getpid);
#endif
#ifdef SYS_getpmsg
	SYS_HASH_ADD(h, "getpmsg", SYS_getpmsg);
#endif
#ifdef SYS_getppid
	SYS_HASH_ADD(h, "getppid", SYS_getppid);
#endif
#ifdef SYS_getpriority
	SYS_HASH_ADD(h, "getpriority", SYS_getpriority);
#endif
#ifdef SYS_getresgid
	SYS_HASH_ADD(h, "getresgid", SYS_getresgid);
#endif
#ifdef SYS_getresgid32
	SYS_HASH_ADD(h, "getresgid32", SYS_getresgid32);
#endif
#ifdef SYS_getresuid
	SYS_HASH_ADD(h, "getresuid", SYS_getresuid);
#endif
#ifdef SYS_getresuid32
	SYS_HASH_ADD(h, "getresuid32", SYS_getresuid32);
#endif
#ifdef SYS_getrlimit
	SYS_HASH_ADD(h, "getrlimit", SYS_getrlimit);
#endif
#ifdef SYS_getrusage
	SYS_HASH_ADD(h, "getrusage", SYS_getrusage);
#endif
#ifdef SYS_getsid
	SYS_HASH_ADD(h, "getsid", SYS_getsid);
#endif
#ifdef SYS_getsockname
	SYS_HASH_ADD(h, "getsockname", SYS_getsockname);
#endif
#ifdef SYS_getsockopt
	SYS_HASH_ADD(h, "getsockopt", SYS_getsockopt);
#endif
#ifdef SYS_gettid
	SYS_HASH_ADD(h, "gettid", SYS_gettid);
#endif
#ifdef SYS_gettimeofday
	SYS_HASH_ADD(h, "gettimeofday", SYS_gettimeofday);
#endif
#ifdef SYS_getuid
	SYS_HASH_ADD(h, "getuid", SYS_getuid);
#endif
#ifdef SYS_getuid32
	SYS_HASH_ADD(h, "getuid32", SYS_getuid32);
#endif
#ifdef SYS_getxattr
	SYS_HASH_ADD(h, "getxattr", SYS_getxattr);
#endif
#ifdef SYS_gtty
	SYS_HASH_ADD(h, "gtty", SYS_gtty);
#endif
#ifdef SYS_idle
	SYS_HASH_ADD(h, "idle", SYS_idle);
#endif
#ifdef SYS_init_module
	SYS_HASH_ADD(h, "init_module", SYS_init_module);
#endif
#ifdef SYS_inotify_add_watch
	SYS_HASH_ADD(h, "inotify_add_watch", SYS_inotify_add_watch);
#endif
#ifdef SYS_inotify_init
	SYS_HASH_ADD(h, "inotify_init", SYS_inotify_init);
#endif
#ifdef SYS_inotify_init1
	SYS_HASH_ADD(h, "inotify_init1", SYS_inotify_init1);
#endif
#ifdef SYS_inotify_rm_watch
	SYS_HASH_ADD(h, "inotify_rm_watch", SYS_inotify_rm_watch);
#endif
#ifdef SYS_io_cancel
	SYS_HASH_ADD(h, "io_cancel", SYS_io_cancel);
#endif
#ifdef SYS_io_destroy
	SYS_HASH_ADD(h, "io_destroy", SYS_io_destroy);
#endif
#ifdef SYS_io_getevents
	SYS_HASH_ADD(h, "io_getevents", SYS_io_getevents);
#endif
#ifdef SYS_io_setup
	SYS_HASH_ADD(h, "io_setup", SYS_io_setup);
#endif
#ifdef SYS_io_submit
	SYS_HASH_ADD(h, "io_submit", SYS_io_submit);
#endif
#ifdef SYS_ioctl
	SYS_HASH_ADD(h, "ioctl", SYS_ioctl);
#endif
#ifdef SYS_ioperm
	SYS_HASH_ADD(h, "ioperm", SYS_ioperm);
#endif
#ifdef SYS_iopl
	SYS_HASH_ADD(h, "iopl", SYS_iopl);
#endif
#ifdef SYS_ioprio_get
	SYS_HASH_ADD(h, "ioprio_get", SYS_ioprio_get);
#endif
#ifdef SYS_ioprio_set
	SYS_HASH_ADD(h, "ioprio_set", SYS_ioprio_set);
#endif
#ifdef SYS_ipc
	SYS_HASH_ADD(h, "ipc", SYS_ipc);
#endif
#ifdef SYS_kexec_load
	SYS_HASH_ADD(h, "kexec_load", SYS_kexec_load);
#endif
#ifdef SYS_keyctl
	SYS_HASH_ADD(h, "keyctl", SYS_keyctl);
#endif
#ifdef SYS_kill
	SYS_HASH_ADD(h, "kill", SYS_kill);
#endif
#ifdef SYS_lchown
	SYS_HASH_ADD(h, "lchown", SYS_lchown);
#endif
#ifdef SYS_lchown32
	SYS_HASH_ADD(h, "lchown32", SYS_lchown32);
#endif
#ifdef SYS_lgetxattr
	SYS_HASH_ADD(h, "lgetxattr", SYS_lgetxattr);
#endif
#ifdef SYS_link
	SYS_HASH_ADD(h, "link", SYS_link);
#endif
#ifdef SYS_linkat
	SYS_HASH_ADD(h, "linkat", SYS_linkat);
#endif
#ifdef SYS_listen
	SYS_HASH_ADD(h, "listen", SYS_listen);
#endif
#ifdef SYS_listxattr
	SYS_HASH_ADD(h, "listxattr", SYS_listxattr);
#endif
#ifdef SYS_llistxattr
	SYS_HASH_ADD(h, "llistxattr", SYS_llistxattr);
#endif
#ifdef SYS_lock
	SYS_HASH_ADD(h, "lock", SYS_lock);
#endif
#ifdef SYS_lookup_dcookie
	SYS_HASH_ADD(h, "lookup_dcookie", SYS_lookup_dcookie);
#endif
#ifdef SYS_lremovexattr
	SYS_HASH_ADD(h, "lremovexattr", SYS_lremovexattr);
#endif
#ifdef SYS_lseek
	SYS_HASH_ADD(h, "lseek", SYS_lseek);
#endif
#ifdef SYS_lsetxattr
	SYS_HASH_ADD(h, "lsetxattr", SYS_lsetxattr);
#endif
#ifdef SYS_lstat
	SYS_HASH_ADD(h, "lstat", SYS_lstat);
#endif
#ifdef SYS_lstat64
	SYS_HASH_ADD(h, "lstat64", SYS_lstat64);
#endif
#ifdef SYS_madvise
	SYS_HASH_ADD(h, "madvise", SYS_madvise);
#endif
#ifdef SYS_madvise1
	SYS_HASH_ADD(h, "madvise1", SYS_madvise1);
#endif
#ifdef SYS_mbind
	SYS_HASH_ADD(h, "mbind", SYS_mbind);
#endif
#ifdef SYS_migrate_pages
	SYS_HASH_ADD(h, "migrate_pages", SYS_migrate_pages);
#endif
#ifdef SYS_mincore
	SYS_HASH_ADD(h, "mincore", SYS_mincore);
#endif
#ifdef SYS_mkdir
	SYS_HASH_ADD(h, "mkdir", SYS_mkdir);
#endif
#ifdef SYS_mkdirat
	SYS_HASH_ADD(h, "mkdirat", SYS_mkdirat);
#endif
#ifdef SYS_mknod
	SYS_HASH_ADD(h, "mknod", SYS_mknod);
#endif
#ifdef SYS_mknodat
	SYS_HASH_ADD(h, "mknodat", SYS_mknodat);
#endif
#ifdef SYS_mlock
	SYS_HASH_ADD(h, "mlock", SYS_mlock);
#endif
#ifdef SYS_mlockall
	SYS_HASH_ADD(h, "mlockall", SYS_mlockall);
#endif
#ifdef SYS_mmap
	SYS_HASH_ADD(h, "mmap", SYS_mmap);
#endif
#ifdef SYS_mmap2
	SYS_HASH_ADD(h, "mmap2", SYS_mmap2);
#endif
#ifdef SYS_modify_ldt
	SYS_HASH_ADD(h, "modify_ldt", SYS_modify_ldt);
#endif
#ifdef SYS_mount
	SYS_HASH_ADD(h, "mount", SYS_mount);
#endif
#ifdef SYS_move_pages
	SYS_HASH_ADD(h, "move_pages", SYS_move_pages);
#endif
#ifdef SYS_mprotect
	SYS_HASH_ADD(h, "mprotect", SYS_mprotect);
#endif
#ifdef SYS_mpx
	SYS_HASH_ADD(h, "mpx", SYS_mpx);
#endif
#ifdef SYS_mq_getsetattr
	SYS_HASH_ADD(h, "mq_getsetattr", SYS_mq_getsetattr);
#endif
#ifdef SYS_mq_notify
	SYS_HASH_ADD(h, "mq_notify", SYS_mq_notify);
#endif
#ifdef SYS_mq_open
	SYS_HASH_ADD(h, "mq_open", SYS_mq_open);
#endif
#ifdef SYS_mq_timedreceive
	SYS_HASH_ADD(h, "mq_timedreceive", SYS_mq_timedreceive);
#endif
#ifdef SYS_mq_timedsend
	SYS_HASH_ADD(h, "mq_timedsend", SYS_mq_timedsend);
#endif
#ifdef SYS_mq_unlink
	SYS_HASH_ADD(h, "mq_unlink", SYS_mq_unlink);
#endif
#ifdef SYS_mremap
	SYS_HASH_ADD(h, "mremap", SYS_mremap);
#endif
#ifdef SYS_msgctl
	SYS_HASH_ADD(h, "msgctl", SYS_msgctl);
#endif
#ifdef SYS_msgget
	SYS_HASH_ADD(h, "msgget", SYS_msgget);
#endif
#ifdef SYS_msgrcv
	SYS_HASH_ADD(h, "msgrcv", SYS_msgrcv);
#endif
#ifdef SYS_msgsnd
	SYS_HASH_ADD(h, "msgsnd", SYS_msgsnd);
#endif
#ifdef SYS_msync
	SYS_HASH_ADD(h, "msync", SYS_msync);
#endif
#ifdef SYS_munlock
	SYS_HASH_ADD(h, "munlock", SYS_munlock);
#endif
#ifdef SYS_munlockall
	SYS_HASH_ADD(h, "munlockall", SYS_munlockall);
#endif
#ifdef SYS_munmap
	SYS_HASH_ADD(h, "munmap", SYS_munmap);
#endif
#ifdef SYS_nanosleep
	SYS_HASH_ADD(h, "nanosleep", SYS_nanosleep);
#endif
#ifdef SYS_newfstatat
	SYS_HASH_ADD(h, "newfstatat", SYS_newfstatat);
#endif
#ifdef SYS_nfsservctl
	SYS_HASH_ADD(h, "nfsservctl", SYS_nfsservctl);
#endif
#ifdef SYS_nice
	SYS_HASH_ADD(h, "nice", SYS_nice);
#endif
#ifdef SYS_oldfstat
	SYS_HASH_ADD(h, "oldfstat", SYS_oldfstat);
#endif
#ifdef SYS_oldlstat
	SYS_HASH_ADD(h, "oldlstat", SYS_oldlstat);
#endif
#ifdef SYS_oldolduname
	SYS_HASH_ADD(h, "oldolduname", SYS_oldolduname);
#endif
#ifdef SYS_oldstat
	SYS_HASH_ADD(h, "oldstat", SYS_oldstat);
#endif
#ifdef SYS_olduname
	SYS_HASH_ADD(h, "olduname", SYS_olduname);
#endif
#ifdef SYS_open
	SYS_HASH_ADD(h, "open", SYS_open);
#endif
#ifdef SYS_openat
	SYS_HASH_ADD(h, "openat", SYS_openat);
#endif
#ifdef SYS_pause
	SYS_HASH_ADD(h, "pause", SYS_pause);
#endif
#ifdef SYS_perf_event_open
	SYS_HASH_ADD(h, "perf_event_open", SYS_perf_event_open);
#endif
#ifdef SYS_personality
	SYS_HASH_ADD(h, "personality", SYS_personality);
#endif
#ifdef SYS_pipe
	SYS_HASH_ADD(h, "pipe", SYS_pipe);
#endif
#ifdef SYS_pipe2
	SYS_HASH_ADD(h, "pipe2", SYS_pipe2);
#endif
#ifdef SYS_pivot_root
	SYS_HASH_ADD(h, "pivot_root", SYS_pivot_root);
#endif
#ifdef SYS_poll
	SYS_HASH_ADD(h, "poll", SYS_poll);
#endif
#ifdef SYS_ppoll
	SYS_HASH_ADD(h, "ppoll", SYS_ppoll);
#endif
#ifdef SYS_prctl
	SYS_HASH_ADD(h, "prctl", SYS_prctl);
#endif
#ifdef SYS_pread64
	SYS_HASH_ADD(h, "pread64", SYS_pread64);
#endif
#ifdef SYS_preadv
	SYS_HASH_ADD(h, "preadv", SYS_preadv);
#endif
#ifdef SYS_prlimit64
	SYS_HASH_ADD(h, "prlimit64", SYS_prlimit64);
#endif
#ifdef SYS_prof
	SYS_HASH_ADD(h, "prof", SYS_prof);
#endif
#ifdef SYS_profil
	SYS_HASH_ADD(h, "profil", SYS_profil);
#endif
#ifdef SYS_pselect6
	SYS_HASH_ADD(h, "pselect6", SYS_pselect6);
#endif
#ifdef SYS_ptrace
	SYS_HASH_ADD(h, "ptrace", SYS_ptrace);
#endif
#ifdef SYS_putpmsg
	SYS_HASH_ADD(h, "putpmsg", SYS_putpmsg);
#endif
#ifdef SYS_pwrite64
	SYS_HASH_ADD(h, "pwrite64", SYS_pwrite64);
#endif
#ifdef SYS_pwritev
	SYS_HASH_ADD(h, "pwritev", SYS_pwritev);
#endif
#ifdef SYS_query_module
	SYS_HASH_ADD(h, "query_module", SYS_query_module);
#endif
#ifdef SYS_quotactl
	SYS_HASH_ADD(h, "quotactl", SYS_quotactl);
#endif
#ifdef SYS_read
	SYS_HASH_ADD(h, "read", SYS_read);
#endif
#ifdef SYS_readahead
	SYS_HASH_ADD(h, "readahead", SYS_readahead);
#endif
#ifdef SYS_readdir
	SYS_HASH_ADD(h, "readdir", SYS_readdir);
#endif
#ifdef SYS_readlink
	SYS_HASH_ADD(h, "readlink", SYS_readlink);
#endif
#ifdef SYS_readlinkat
	SYS_HASH_ADD(h, "readlinkat", SYS_readlinkat);
#endif
#ifdef SYS_readv
	SYS_HASH_ADD(h, "readv", SYS_readv);
#endif
#ifdef SYS_reboot
	SYS_HASH_ADD(h, "reboot", SYS_reboot);
#endif
#ifdef SYS_recvfrom
	SYS_HASH_ADD(h, "recvfrom", SYS_recvfrom);
#endif
#ifdef SYS_recvmmsg
	SYS_HASH_ADD(h, "recvmmsg", SYS_recvmmsg);
#endif
#ifdef SYS_recvmsg
	SYS_HASH_ADD(h, "recvmsg", SYS_recvmsg);
#endif
#ifdef SYS_remap_file_pages
	SYS_HASH_ADD(h, "remap_file_pages", SYS_remap_file_pages);
#endif
#ifdef SYS_removexattr
	SYS_HASH_ADD(h, "removexattr", SYS_removexattr);
#endif
#ifdef SYS_rename
	SYS_HASH_ADD(h, "rename", SYS_rename);
#endif
#ifdef SYS_renameat
	SYS_HASH_ADD(h, "renameat", SYS_renameat);
#endif
#ifdef SYS_request_key
	SYS_HASH_ADD(h, "request_key", SYS_request_key);
#endif
#ifdef SYS_restart_syscall
	SYS_HASH_ADD(h, "restart_syscall", SYS_restart_syscall);
#endif
#ifdef SYS_rmdir
	SYS_HASH_ADD(h, "rmdir", SYS_rmdir);
#endif
#ifdef SYS_rt_sigaction
	SYS_HASH_ADD(h, "rt_sigaction", SYS_rt_sigaction);
#endif
#ifdef SYS_rt_sigpending
	SYS_HASH_ADD(h, "rt_sigpending", SYS_rt_sigpending);
#endif
#ifdef SYS_rt_sigprocmask
	SYS_HASH_ADD(h, "rt_sigprocmask", SYS_rt_sigprocmask);
#endif
#ifdef SYS_rt_sigqueueinfo
	SYS_HASH_ADD(h, "rt_sigqueueinfo", SYS_rt_sigqueueinfo);
#endif
#ifdef SYS_rt_sigreturn
	SYS_HASH_ADD(h, "rt_sigreturn", SYS_rt_sigreturn);
#endif
#ifdef SYS_rt_sigsuspend
	SYS_HASH_ADD(h, "rt_sigsuspend", SYS_rt_sigsuspend);
#endif
#ifdef SYS_rt_sigtimedwait
	SYS_HASH_ADD(h, "rt_sigtimedwait", SYS_rt_sigtimedwait);
#endif
#ifdef SYS_rt_tgsigqueueinfo
	SYS_HASH_ADD(h, "rt_tgsigqueueinfo", SYS_rt_tgsigqueueinfo);
#endif
#ifdef SYS_sched_get_priority_max
	SYS_HASH_ADD(h, "sched_get_priority_max", SYS_sched_get_priority_max);
#endif
#ifdef SYS_sched_get_priority_min
	SYS_HASH_ADD(h, "sched_get_priority_min", SYS_sched_get_priority_min);
#endif
#ifdef SYS_sched_getaffinity
	SYS_HASH_ADD(h, "sched_getaffinity", SYS_sched_getaffinity);
#endif
#ifdef SYS_sched_getparam
	SYS_HASH_ADD(h, "sched_getparam", SYS_sched_getparam);
#endif
#ifdef SYS_sched_getscheduler
	SYS_HASH_ADD(h, "sched_getscheduler", SYS_sched_getscheduler);
#endif
#ifdef SYS_sched_rr_get_interval
	SYS_HASH_ADD(h, "sched_rr_get_interval", SYS_sched_rr_get_interval);
#endif
#ifdef SYS_sched_setaffinity
	SYS_HASH_ADD(h, "sched_setaffinity", SYS_sched_setaffinity);
#endif
#ifdef SYS_sched_setparam
	SYS_HASH_ADD(h, "sched_setparam", SYS_sched_setparam);
#endif
#ifdef SYS_sched_setscheduler
	SYS_HASH_ADD(h, "sched_setscheduler", SYS_sched_setscheduler);
#endif
#ifdef SYS_sched_yield
	SYS_HASH_ADD(h, "sched_yield", SYS_sched_yield);
#endif
#ifdef SYS_security
	SYS_HASH_ADD(h, "security", SYS_security);
#endif
#ifdef SYS_select
	SYS_HASH_ADD(h, "select", SYS_select);
#endif
#ifdef SYS_semctl
	SYS_HASH_ADD(h, "semctl", SYS_semctl);
#endif
#ifdef SYS_semget
	SYS_HASH_ADD(h, "semget", SYS_semget);
#endif
#ifdef SYS_semop
	SYS_HASH_ADD(h, "semop", SYS_semop);
#endif
#ifdef SYS_semtimedop
	SYS_HASH_ADD(h, "semtimedop", SYS_semtimedop);
#endif
#ifdef SYS_sendfile
	SYS_HASH_ADD(h, "sendfile", SYS_sendfile);
#endif
#ifdef SYS_sendfile64
	SYS_HASH_ADD(h, "sendfile64", SYS_sendfile64);
#endif
#ifdef SYS_sendmsg
	SYS_HASH_ADD(h, "sendmsg", SYS_sendmsg);
#endif
#ifdef SYS_sendto
	SYS_HASH_ADD(h, "sendto", SYS_sendto);
#endif
#ifdef SYS_set_mempolicy
	SYS_HASH_ADD(h, "set_mempolicy", SYS_set_mempolicy);
#endif
#ifdef SYS_set_robust_list
	SYS_HASH_ADD(h, "set_robust_list", SYS_set_robust_list);
#endif
#ifdef SYS_set_thread_area
	SYS_HASH_ADD(h, "set_thread_area", SYS_set_thread_area);
#endif
#ifdef SYS_set_tid_address
	SYS_HASH_ADD(h, "set_tid_address", SYS_set_tid_address);
#endif
#ifdef SYS_setdomainname
	SYS_HASH_ADD(h, "setdomainname", SYS_setdomainname);
#endif
#ifdef SYS_setfsgid
	SYS_HASH_ADD(h, "setfsgid", SYS_setfsgid);
#endif
#ifdef SYS_setfsgid32
	SYS_HASH_ADD(h, "setfsgid32", SYS_setfsgid32);
#endif
#ifdef SYS_setfsuid
	SYS_HASH_ADD(h, "setfsuid", SYS_setfsuid);
#endif
#ifdef SYS_setfsuid32
	SYS_HASH_ADD(h, "setfsuid32", SYS_setfsuid32);
#endif
#ifdef SYS_setgid
	SYS_HASH_ADD(h, "setgid", SYS_setgid);
#endif
#ifdef SYS_setgid32
	SYS_HASH_ADD(h, "setgid32", SYS_setgid32);
#endif
#ifdef SYS_setgroups
	SYS_HASH_ADD(h, "setgroups", SYS_setgroups);
#endif
#ifdef SYS_setgroups32
	SYS_HASH_ADD(h, "setgroups32", SYS_setgroups32);
#endif
#ifdef SYS_sethostname
	SYS_HASH_ADD(h, "sethostname", SYS_sethostname);
#endif
#ifdef SYS_setitimer
	SYS_HASH_ADD(h, "setitimer", SYS_setitimer);
#endif
#ifdef SYS_setpgid
	SYS_HASH_ADD(h, "setpgid", SYS_setpgid);
#endif
#ifdef SYS_setpriority
	SYS_HASH_ADD(h, "setpriority", SYS_setpriority);
#endif
#ifdef SYS_setregid
	SYS_HASH_ADD(h, "setregid", SYS_setregid);
#endif
#ifdef SYS_setregid32
	SYS_HASH_ADD(h, "setregid32", SYS_setregid32);
#endif
#ifdef SYS_setresgid
	SYS_HASH_ADD(h, "setresgid", SYS_setresgid);
#endif
#ifdef SYS_setresgid32
	SYS_HASH_ADD(h, "setresgid32", SYS_setresgid32);
#endif
#ifdef SYS_setresuid
	SYS_HASH_ADD(h, "setresuid", SYS_setresuid);
#endif
#ifdef SYS_setresuid32
	SYS_HASH_ADD(h, "setresuid32", SYS_setresuid32);
#endif
#ifdef SYS_setreuid
	SYS_HASH_ADD(h, "setreuid", SYS_setreuid);
#endif
#ifdef SYS_setreuid32
	SYS_HASH_ADD(h, "setreuid32", SYS_setreuid32);
#endif
#ifdef SYS_setrlimit
	SYS_HASH_ADD(h, "setrlimit", SYS_setrlimit);
#endif
#ifdef SYS_setsid
	SYS_HASH_ADD(h, "setsid", SYS_setsid);
#endif
#ifdef SYS_setsockopt
	SYS_HASH_ADD(h, "setsockopt", SYS_setsockopt);
#endif
#ifdef SYS_settimeofday
	SYS_HASH_ADD(h, "settimeofday", SYS_settimeofday);
#endif
#ifdef SYS_setuid
	SYS_HASH_ADD(h, "setuid", SYS_setuid);
#endif
#ifdef SYS_setuid32
	SYS_HASH_ADD(h, "setuid32", SYS_setuid32);
#endif
#ifdef SYS_setxattr
	SYS_HASH_ADD(h, "setxattr", SYS_setxattr);
#endif
#ifdef SYS_sgetmask
	SYS_HASH_ADD(h, "sgetmask", SYS_sgetmask);
#endif
#ifdef SYS_shmat
	SYS_HASH_ADD(h, "shmat", SYS_shmat);
#endif
#ifdef SYS_shmctl
	SYS_HASH_ADD(h, "shmctl", SYS_shmctl);
#endif
#ifdef SYS_shmdt
	SYS_HASH_ADD(h, "shmdt", SYS_shmdt);
#endif
#ifdef SYS_shmget
	SYS_HASH_ADD(h, "shmget", SYS_shmget);
#endif
#ifdef SYS_shutdown
	SYS_HASH_ADD(h, "shutdown", SYS_shutdown);
#endif
#ifdef SYS_sigaction
	SYS_HASH_ADD(h, "sigaction", SYS_sigaction);
#endif
#ifdef SYS_sigaltstack
	SYS_HASH_ADD(h, "sigaltstack", SYS_sigaltstack);
#endif
#ifdef SYS_signal
	SYS_HASH_ADD(h, "signal", SYS_signal);
#endif
#ifdef SYS_signalfd
	SYS_HASH_ADD(h, "signalfd", SYS_signalfd);
#endif
#ifdef SYS_signalfd4
	SYS_HASH_ADD(h, "signalfd4", SYS_signalfd4);
#endif
#ifdef SYS_sigpending
	SYS_HASH_ADD(h, "sigpending", SYS_sigpending);
#endif
#ifdef SYS_sigprocmask
	SYS_HASH_ADD(h, "sigprocmask", SYS_sigprocmask);
#endif
#ifdef SYS_sigreturn
	SYS_HASH_ADD(h, "sigreturn", SYS_sigreturn);
#endif
#ifdef SYS_sigsuspend
	SYS_HASH_ADD(h, "sigsuspend", SYS_sigsuspend);
#endif
#ifdef SYS_socket
	SYS_HASH_ADD(h, "socket", SYS_socket);
#endif
#ifdef SYS_socketcall
	SYS_HASH_ADD(h, "socketcall", SYS_socketcall);
#endif
#ifdef SYS_socketpair
	SYS_HASH_ADD(h, "socketpair", SYS_socketpair);
#endif
#ifdef SYS_splice
	SYS_HASH_ADD(h, "splice", SYS_splice);
#endif
#ifdef SYS_ssetmask
	SYS_HASH_ADD(h, "ssetmask", SYS_ssetmask);
#endif
#ifdef SYS_stat
	SYS_HASH_ADD(h, "stat", SYS_stat);
#endif
#ifdef SYS_stat64
	SYS_HASH_ADD(h, "stat64", SYS_stat64);
#endif
#ifdef SYS_statfs
	SYS_HASH_ADD(h, "statfs", SYS_statfs);
#endif
#ifdef SYS_statfs64
	SYS_HASH_ADD(h, "statfs64", SYS_statfs64);
#endif
#ifdef SYS_stime
	SYS_HASH_ADD(h, "stime", SYS_stime);
#endif
#ifdef SYS_stty
	SYS_HASH_ADD(h, "stty", SYS_stty);
#endif
#ifdef SYS_swapoff
	SYS_HASH_ADD(h, "swapoff", SYS_swapoff);
#endif
#ifdef SYS_swapon
	SYS_HASH_ADD(h, "swapon", SYS_swapon);
#endif
#ifdef SYS_symlink
	SYS_HASH_ADD(h, "symlink", SYS_symlink);
#endif
#ifdef SYS_symlinkat
	SYS_HASH_ADD(h, "symlinkat", SYS_symlinkat);
#endif
#ifdef SYS_sync
	SYS_HASH_ADD(h, "sync", SYS_sync);
#endif
#ifdef SYS_sync_file_range
	SYS_HASH_ADD(h, "sync_file_range", SYS_sync_file_range);
#endif
#ifdef SYS_sysfs
	SYS_HASH_ADD(h, "sysfs", SYS_sysfs);
#endif
#ifdef SYS_sysinfo
	SYS_HASH_ADD(h, "sysinfo", SYS_sysinfo);
#endif
#ifdef SYS_syslog
	SYS_HASH_ADD(h, "syslog", SYS_syslog);
#endif
#ifdef SYS_tee
	SYS_HASH_ADD(h, "tee", SYS_tee);
#endif
#ifdef SYS_tgkill
	SYS_HASH_ADD(h, "tgkill", SYS_tgkill);
#endif
#ifdef SYS_time
	SYS_HASH_ADD(h, "time", SYS_time);
#endif
#ifdef SYS_timer_create
	SYS_HASH_ADD(h, "timer_create", SYS_timer_create);
#endif
#ifdef SYS_timer_delete
	SYS_HASH_ADD(h, "timer_delete", SYS_timer_delete);
#endif
#ifdef SYS_timer_getoverrun
	SYS_HASH_ADD(h, "timer_getoverrun", SYS_timer_getoverrun);
#endif
#ifdef SYS_timer_gettime
	SYS_HASH_ADD(h, "timer_gettime", SYS_timer_gettime);
#endif
#ifdef SYS_timer_settime
	SYS_HASH_ADD(h, "timer_settime", SYS_timer_settime);
#endif
#ifdef SYS_timerfd_create
	SYS_HASH_ADD(h, "timerfd_create", SYS_timerfd_create);
#endif
#ifdef SYS_timerfd_gettime
	SYS_HASH_ADD(h, "timerfd_gettime", SYS_timerfd_gettime);
#endif
#ifdef SYS_timerfd_settime
	SYS_HASH_ADD(h, "timerfd_settime", SYS_timerfd_settime);
#endif
#ifdef SYS_times
	SYS_HASH_ADD(h, "times", SYS_times);
#endif
#ifdef SYS_tkill
	SYS_HASH_ADD(h, "tkill", SYS_tkill);
#endif
#ifdef SYS_truncate
	SYS_HASH_ADD(h, "truncate", SYS_truncate);
#endif
#ifdef SYS_truncate64
	SYS_HASH_ADD(h, "truncate64", SYS_truncate64);
#endif
#ifdef SYS_tuxcall
	SYS_HASH_ADD(h, "tuxcall", SYS_tuxcall);
#endif
#ifdef SYS_ugetrlimit
	SYS_HASH_ADD(h, "ugetrlimit", SYS_ugetrlimit);
#endif
#ifdef SYS_ulimit
	SYS_HASH_ADD(h, "ulimit", SYS_ulimit);
#endif
#ifdef SYS_umask
	SYS_HASH_ADD(h, "umask", SYS_umask);
#endif
#ifdef SYS_umount
	SYS_HASH_ADD(h, "umount", SYS_umount);
#endif
#ifdef SYS_umount2
	SYS_HASH_ADD(h, "umount2", SYS_umount2);
#endif
#ifdef SYS_uname
	SYS_HASH_ADD(h, "uname", SYS_uname);
#endif
#ifdef SYS_unlink
	SYS_HASH_ADD(h, "unlink", SYS_unlink);
#endif
#ifdef SYS_unlinkat
	SYS_HASH_ADD(h, "unlinkat", SYS_unlinkat);
#endif
#ifdef SYS_unshare
	SYS_HASH_ADD(h, "unshare", SYS_unshare);
#endif
#ifdef SYS_uselib
	SYS_HASH_ADD(h, "uselib", SYS_uselib);
#endif
#ifdef SYS_ustat
	SYS_HASH_ADD(h, "ustat", SYS_ustat);
#endif
#ifdef SYS_utime
	SYS_HASH_ADD(h, "utime", SYS_utime);
#endif
#ifdef SYS_utimensat
	SYS_HASH_ADD(h, "utimensat", SYS_utimensat);
#endif
#ifdef SYS_utimes
	SYS_HASH_ADD(h, "utimes", SYS_utimes);
#endif
#ifdef SYS_vfork
	SYS_HASH_ADD(h, "vfork", SYS_vfork);
#endif
#ifdef SYS_vhangup
	SYS_HASH_ADD(h, "vhangup", SYS_vhangup);
#endif
#ifdef SYS_vm86
	SYS_HASH_ADD(h, "vm86", SYS_vm86);
#endif
#ifdef SYS_vm86old
	SYS_HASH_ADD(h, "vm86old", SYS_vm86old);
#endif
#ifdef SYS_vmsplice
	SYS_HASH_ADD(h, "vmsplice", SYS_vmsplice);
#endif
#ifdef SYS_vserver
	SYS_HASH_ADD(h, "vserver", SYS_vserver);
#endif
#ifdef SYS_wait4
	SYS_HASH_ADD(h, "wait4", SYS_wait4);
#endif
#ifdef SYS_waitid
	SYS_HASH_ADD(h, "waitid", SYS_waitid);
#endif
#ifdef SYS_waitpid
	SYS_HASH_ADD(h, "waitpid", SYS_waitpid);
#endif
#ifdef SYS_write
	SYS_HASH_ADD(h, "write", SYS_write);
#endif
#ifdef SYS_writev
	SYS_HASH_ADD(h, "writev", SYS_writev);
#endif

	return h;
}

/* This is a hash of user structure members to offsets */
static VALUE build_user_hash( void ) {
	VALUE h = rb_hash_new();
	unsigned int offset;

	//u_fp_valid 
	offset = sizeof(struct user_regs_struct);
	//u_tsize = 
	offset += sizeof(int) + sizeof(struct user_fpregs_struct);
	//u_dsize = 
	offset += sizeof(unsigned long int);
	//u_ssize = 
	offset += sizeof(unsigned long int);
	//start_code = 
	offset += sizeof(unsigned long int);
	//start_stack = 
	offset += sizeof(unsigned long);
	//signal = 
	offset += sizeof(unsigned long);
	// magic
	offset += sizeof(long int) + sizeof(void *) + sizeof(void *);
	// comm
	offset += sizeof(unsigned long int);
	// debug_r0 - r7
	offset += sizeof(char) * 32;
	offset += sizeof(int);
	//CMD_HASH_ADD(h, SZ_PTRACE_TRACEME, PT_TRACE_ME) 

	return h;
}

static VALUE build_option_hash( void ) {
	VALUE h = rb_hash_new();

	// TODO : Change this to if linux as these are not #defines!
#ifdef PTRACE_O_TRACESYSGOOD
	CMD_HASH_ADD(h, SZ_PTRACE_O_TRACESYSGOOD, PTRACE_O_TRACESYSGOOD)
#endif
#ifdef PTRACE_O_TRACEFORK
	CMD_HASH_ADD(h, SZ_PTRACE_O_TRACEFORK, PTRACE_O_TRACEFORK)
#endif
#ifdef PTRACE_O_TRACEVFORK
	CMD_HASH_ADD(h, SZ_PTRACE_O_TRACEVFORK, PTRACE_O_TRACEVFORK)
#endif
#ifdef PTRACE_O_TRACEVFORKDONE
	CMD_HASH_ADD(h, SZ_PTRACE_O_TRACEVFORKDONE, PTRACE_O_TRACEVFORKDONE)
#endif
#ifdef PTRACE_O_TRACECLONE
	CMD_HASH_ADD(h, SZ_PTRACE_O_TRACECLONE, PTRACE_O_TRACECLONE)
#endif
#ifdef PTRACE_O_TRACEEXEC
	CMD_HASH_ADD(h, SZ_PTRACE_O_TRACEEXEC, PTRACE_O_TRACEEXEC)
#endif
#ifdef PTRACE_O_TRACEEXIT
	CMD_HASH_ADD(h, SZ_PTRACE_O_TRACEEXIT, PTRACE_O_TRACEEXIT)
#endif
	return h;
}

/* ---------------------------------------------------------------------- */

static long int_ptrace_raw( enum __ptrace_request req, VALUE pid, void * addr, 
			    void * data )  {
	pid_t tgt;
	long rv;

	tgt = num_to_pid(pid);
	rv = ptrace(req, tgt, addr, data);
	if ( rv == -1l ) {
		 rb_raise(rb_eRuntimeError, "PTRACE: %s", strerror(errno));
	}

	return rv;
}

/* internal wrapper for ptrace that converts PID to a pid_t, and converts
 * return value to a Fixnum */
static VALUE int_ptrace( enum __ptrace_request req, VALUE pid, void * addr, 
			void * data )  {
	long rv = int_ptrace_raw(req, pid, addr, data);
	return LONG2NUM(rv);
}

static VALUE int_ptrace_data( VALUE req, VALUE pid, VALUE addr, void * data ) {
	enum __ptrace_request cmd = (req == Qnil) ? 0 :
				    (enum __ptrace_request) NUM2UINT(req);
	void * tgt_addr = (addr == Qnil) ? NULL : (void *) NUM2OFFT(addr);

	return int_ptrace(cmd, pid, tgt_addr, data);
}


/* ---------------------------------------------------------------------- */
/* PTRACE API */

static VALUE ptrace_send( VALUE cls, VALUE req, VALUE pid, VALUE addr) {
	return int_ptrace_data( req, pid, addr, NULL );
}

/* NOTE only use this for data that is NOT a memory address (pointer)! */
static VALUE ptrace_send_data( VALUE cls, VALUE req, VALUE pid, VALUE addr, 
			       VALUE data ) {
	void * the_data = (data == Qnil) ? NULL : (void *) NUM2ULONG(data);

	return int_ptrace_data(req, pid, addr, the_data);
}

/* peek_text, peek_data, peek_user */
static VALUE ptrace_peek( VALUE cls, VALUE req, VALUE pid, VALUE addr ) {
	return int_ptrace_data(req, pid, addr, NULL);
}

/* poke_text, poke_data, poke_user */
static VALUE ptrace_poke( VALUE cls, VALUE req, VALUE pid, VALUE addr, 
			  VALUE data ) {
	void * the_data = (void *) NUM2ULONG(data);

	return int_ptrace_data(req, pid, addr, the_data);
}

static VALUE ptrace_get_regs( VALUE cls, VALUE pid ) {
	VALUE h = rb_hash_new();

#ifdef __linux
	long rv = 0;
	struct user_regs_struct regs = {0};

	rv = int_ptrace_raw( PT_GETREGS, pid, NULL, &regs);

#  ifdef __x86_64__
	rb_hash_aset( h, rb_str_new_cstr("r15"), ULONG2NUM(regs.r15) );
	rb_hash_aset( h, rb_str_new_cstr("r14"), ULONG2NUM(regs.r14) );
	rb_hash_aset( h, rb_str_new_cstr("r13"), ULONG2NUM(regs.r13) );
	rb_hash_aset( h, rb_str_new_cstr("r12"), ULONG2NUM(regs.r12) );
	rb_hash_aset( h, rb_str_new_cstr("rbp"), ULONG2NUM(regs.rbp) );
	rb_hash_aset( h, rb_str_new_cstr("rbx"), ULONG2NUM(regs.rbx) );
	rb_hash_aset( h, rb_str_new_cstr("r11"), ULONG2NUM(regs.r11) );
	rb_hash_aset( h, rb_str_new_cstr("r10"), ULONG2NUM(regs.r10) );
	rb_hash_aset( h, rb_str_new_cstr("r9"), ULONG2NUM(regs.r9) );
	rb_hash_aset( h, rb_str_new_cstr("r8"), ULONG2NUM(regs.r8) );
	rb_hash_aset( h, rb_str_new_cstr("rax"), ULONG2NUM(regs.rax) );
	rb_hash_aset( h, rb_str_new_cstr("rcx"), ULONG2NUM(regs.rcx) );
	rb_hash_aset( h, rb_str_new_cstr("rdx"), ULONG2NUM(regs.rdx) );
	rb_hash_aset( h, rb_str_new_cstr("rsi"), ULONG2NUM(regs.rsi) );
	rb_hash_aset( h, rb_str_new_cstr("rdi"), ULONG2NUM(regs.rdi) );
	rb_hash_aset( h, rb_str_new_cstr("orig_rax"), ULONG2NUM(regs.orig_rax));
	rb_hash_aset( h, rb_str_new_cstr("rip"), ULONG2NUM(regs.rip) );
	rb_hash_aset( h, rb_str_new_cstr("cs"), ULONG2NUM(regs.cs) );
	rb_hash_aset( h, rb_str_new_cstr("eflags"), ULONG2NUM(regs.eflags) );
	rb_hash_aset( h, rb_str_new_cstr("rsp"), ULONG2NUM(regs.rsp) );
	rb_hash_aset( h, rb_str_new_cstr("ss"), ULONG2NUM(regs.ss) );
	rb_hash_aset( h, rb_str_new_cstr("fs_base"), ULONG2NUM(regs.fs_base) );
	rb_hash_aset( h, rb_str_new_cstr("gs_base"), ULONG2NUM(regs.gs_base) );
	rb_hash_aset( h, rb_str_new_cstr("ds"), ULONG2NUM(regs.ds) );
	rb_hash_aset( h, rb_str_new_cstr("es"), ULONG2NUM(regs.es) );
	rb_hash_aset( h, rb_str_new_cstr("fs"), ULONG2NUM(regs.fs) );
	rb_hash_aset( h, rb_str_new_cstr("gs"), ULONG2NUM(regs.gs) );
#  else
	rb_hash_aset( h, rb_str_new_cstr("ebx"), ULONG2NUM(regs.ebx) );
	rb_hash_aset( h, rb_str_new_cstr("ecx"), ULONG2NUM(regs.ecx) );
	rb_hash_aset( h, rb_str_new_cstr("edx"), ULONG2NUM(regs.edx) );
	rb_hash_aset( h, rb_str_new_cstr("esi"), ULONG2NUM(regs.esi) );
	rb_hash_aset( h, rb_str_new_cstr("edi"), ULONG2NUM(regs.edi) );
	rb_hash_aset( h, rb_str_new_cstr("ebp"), ULONG2NUM(regs.ebp) );
	rb_hash_aset( h, rb_str_new_cstr("eax"), ULONG2NUM(regs.eax) );
	rb_hash_aset( h, rb_str_new_cstr("ds"), ULONG2NUM(regs.xds) );
	rb_hash_aset( h, rb_str_new_cstr("es"), ULONG2NUM(regs.xes) );
	rb_hash_aset( h, rb_str_new_cstr("fs"), ULONG2NUM(regs.xfs) );
	rb_hash_aset( h, rb_str_new_cstr("gs"), ULONG2NUM(regs.xgs) );
	rb_hash_aset( h, rb_str_new_cstr("orig_eax"), ULONG2NUM(regs.orig_eax));
	rb_hash_aset( h, rb_str_new_cstr("eip"), ULONG2NUM(regs.eip) );
	rb_hash_aset( h, rb_str_new_cstr("cs"), ULONG2NUM(regs.xcs) );
	rb_hash_aset( h, rb_str_new_cstr("eflags"), ULONG2NUM(regs.eflags) );
	rb_hash_aset( h, rb_str_new_cstr("esp"), ULONG2NUM(regs.esp) );
	rb_hash_aset( h, rb_str_new_cstr("ss"), ULONG2NUM(regs.xss) );
#  endif
#endif
	return h;
}

static VALUE ptrace_set_regs( VALUE cls, VALUE pid, VALUE h ) {

#ifdef __linux
	long rv = 0;
	struct user_regs_struct regs = {0};
	int_ptrace_raw( PT_GETREGS, pid, NULL, &regs);

#  ifdef __x86_64__
	regs.r15 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r15") ));
	regs.r14 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r14") ));
	regs.r13 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r13") ));
	regs.r12 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r12") ));
	regs.rbp = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rbp") ));
	regs.rbx = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rbx") ));
	regs.r11 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r11") ));
	regs.r10 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r10") ));
	regs.r9 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r9") ));
	regs.r8 = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("r8") ));
	regs.rax = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rax") ));
	regs.rcx = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rcx") ));
	regs.rdx = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rdx") ));
	regs.rsi = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rsi") ));
	regs.rdi = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rdi") ));
	/* Ptrace does not allow modification of these registers:
	regs.orig_rax = NUM2ULONG(hash_get_int(h, rb_str_new_cstr("orig_rax")));
	regs.cs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("cs") ));
	regs.ss = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ss") ));
	regs.fs_base = NUM2ULONG(hash_get_int(h, rb_str_new_cstr("fs_base") ));
	regs.gs_base = NUM2ULONG(hash_get_int(h, rb_str_new_cstr("gs_base") ));
	*/
	regs.rip = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rip") ));
	regs.eflags = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("eflags") ));
	regs.rsp = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rsp") ));
	regs.ds = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ds") ));
	regs.es = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("es") ));
	regs.fs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fs") ));
	regs.gs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("gs") ));
#  else
	regs.ebx = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ebx") ));
	regs.ecx = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ecx") ));
	regs.edx = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("edx") ));
	regs.esi = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("esi") ));
	regs.edi = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("edi") ));
	regs.ebp = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ebp") ));
	regs.eax = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("eax") ));
	regs.xds = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ds") ));
	regs.xes = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("es") ));
	regs.xfs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fs") ));
	regs.xgs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("gs") ));
	/* Ptrace does not allow modification of these registers:
	regs.orig_rax = NUM2ULONG(hash_get_int(h, rb_str_new_cstr("orig_rax")));
	regs.xcs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("cs") ));
	regs.xss = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ss") ));
	*/
	regs.eip = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("eip") ));
	regs.eflags = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("eflags") ));
	regs.esp = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("esp") ));
#  endif
	rv = int_ptrace_raw( PT_SETREGS, pid, NULL, &regs);
#endif
	return Qnil;
}

static VALUE ptrace_get_fpregs( VALUE cls, VALUE pid ) {
	VALUE h = rb_hash_new();
#ifdef __linux
	long rv = 0;
	int i;
	struct user_fpregs_struct regs = {0};

	rv = int_ptrace_raw( PT_GETFPREGS, pid, NULL, &regs);

#  ifdef __x86_64__
	rb_hash_aset( h, rb_str_new_cstr("cwd"), UINT2NUM(regs.cwd) );
	rb_hash_aset( h, rb_str_new_cstr("swd"), UINT2NUM(regs.swd) );
	rb_hash_aset( h, rb_str_new_cstr("ftw"), UINT2NUM(regs.ftw) );
	rb_hash_aset( h, rb_str_new_cstr("fop"), UINT2NUM(regs.fop) );
	rb_hash_aset( h, rb_str_new_cstr("rip"), ULONG2NUM(regs.rip) );
	rb_hash_aset( h, rb_str_new_cstr("rdp"), ULONG2NUM(regs.rdp) );
	rb_hash_aset( h, rb_str_new_cstr("mxcsr"), UINT2NUM(regs.mxcsr) );
	rb_hash_aset( h, rb_str_new_cstr("mxcr_mask"), 
			 UINT2NUM(regs.mxcr_mask) );
	for ( i = 0; i < 32; i++ ) {
		char buf[8];
		sprintf(buf, "ST(%d)", i);
		// TODO: 8 x 16-byte regs
		//rb_hash_aset( h, rb_str_new_cstr(buf), 
		//	      UINT2NUM(regs.st_space[i]) );
	}
	for ( i = 0; i < 64; i++ ) {
		char buf[8];
		sprintf(buf, "xmm%d", i);
		// TODO: 16 x 16-byte regs
		//rb_hash_aset( h, rb_str_new_cstr(buf), 
		//	      UINT2NUM(regs.xmm_space[i]) );
	}
#  else
	rb_hash_aset( h, rb_str_new_cstr("cwd"), UINT2NUM(regs.cwd) );
	rb_hash_aset( h, rb_str_new_cstr("swd"), UINT2NUM(regs.swd) );
	rb_hash_aset( h, rb_str_new_cstr("twd"), UINT2NUM(regs.twd) );
	rb_hash_aset( h, rb_str_new_cstr("fip"), UINT2NUM(regs.fip) );
	rb_hash_aset( h, rb_str_new_cstr("fcs"), UINT2NUM(regs.fcs) );
	rb_hash_aset( h, rb_str_new_cstr("foo"), UINT2NUM(regs.foo) );
	rb_hash_aset( h, rb_str_new_cstr("fos"), UINT2NUM(regs.fos) );
	for ( i = 0; i < 20; i++ ) {
		// 20x long int st_space
		char buf[8];
		sprintf(buf, "ST(%d)", i);
		// TODO: 8 x 16-byte regs
		//rb_hash_aset( h, rb_str_new_cstr(buf), 
		//	      UINT2NUM(regs.st_space[i]) );
	}
#  endif
#elif defined(__APPLE__)
#  ifdef __x86_64__
#  else
#  endif
#endif
	return h;
}

static VALUE ptrace_set_fpregs( VALUE cls, VALUE pid, VALUE h ) {

#ifdef __linux
	long rv = 0;
	int i;
	struct user_fpregs_struct regs = {0};
#  ifdef __x86_64__
	regs.cwd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("cwd") ));
	regs.swd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("swd") ));
	regs.ftw = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ftw") ));
	regs.fop = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fop") ));
	regs.rip = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rip") ));
	regs.rdp = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("rdp") ));
	regs.mxcsr = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("mxcsr") ));
	regs.mxcr_mask = NUM2ULONG(hash_get_int( h, 
					rb_str_new_cstr("mxcr_mask") ));
	for ( i = 0; i < 32; i++ ) {
		char buf[8];
		sprintf(buf, "ST(%d)", i);
		// TODO: 8 * 16-byte regs
		//regs.st_space[i] = NUM2UINT(hash_get_int( h, 
		//				rb_str_new_cstr(buf) ));
	}
	for ( i = 0; i < 64; i++ ) {
		char buf[8];
		sprintf(buf, "xmm%d", i);
		// TODO : 16 x 16-byte regs
		//regs.xmm_space[i] = NUM2UINT(hash_get_int( h, 
		//				rb_str_new_cstr(buf) ));
	}
#  else
	regs.cwd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("cwd") ));
	regs.swd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("swd") ));
	regs.twd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("twd") ));
	regs.fip = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fip") ));
	regs.fcs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fcs") ));
	regs.foo = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("foo") ));
	regs.fos = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fos") ));
	for ( i = 0; i < 20; i++ ) {
		char buf[8];
		sprintf(buf, "ST(%d)", i);
		// TODO: 8 * 16-byte regs
		//regs.st_space[i] = NUM2UINT(hash_get_int( h, 
		//				rb_str_new_cstr(buf) ));
	}
#  endif
	rv = int_ptrace_raw( PT_SETFPREGS, pid, NULL, &regs);
#elif defined(__APPLE__)
#  ifdef __x86_64__
#  else
#  endif
#endif
	return Qnil;
}

static VALUE ptrace_get_fpxregs( VALUE cls, VALUE pid ) {
	VALUE h = rb_hash_new();
#ifdef __linux
	long rv = 0;
	int i;

#  ifdef __x86_64__
	// Nothing to do: x86-64 has no fpxregs structure
#  else
	struct user_fpxregs_struct regs = {0};

	rv = int_ptrace_raw( PT_GETFPREGS, pid, NULL, &regs);

	rb_hash_aset( h, rb_str_new_cstr("cwd"), UINT2NUM(regs.cwd) );
	rb_hash_aset( h, rb_str_new_cstr("swd"), UINT2NUM(regs.swd) );
	rb_hash_aset( h, rb_str_new_cstr("twd"), UINT2NUM(regs.twd) );
	rb_hash_aset( h, rb_str_new_cstr("fop"), UINT2NUM(regs.fop) );
	rb_hash_aset( h, rb_str_new_cstr("fip"), ULONG2NUM(regs.fip) );
	rb_hash_aset( h, rb_str_new_cstr("fcs"), UINT2NUM(regs.fcs) );
	rb_hash_aset( h, rb_str_new_cstr("foo"), UINT2NUM(regs.foo) );
	rb_hash_aset( h, rb_str_new_cstr("fos"), UINT2NUM(regs.fos) );
	rb_hash_aset( h, rb_str_new_cstr("mxcsr"), UINT2NUM(regs.mxcsr) );
	for ( i = 0; i < 32; i++ ) {
		char buf[8];
		sprintf(buf, "ST(%d)", i);
		// TODO: 8 x 16-byte regs
		//rb_hash_aset( h, rb_str_new_cstr(buf), 
		//	      UINT2NUM(regs.st_space[i]) );
	}
	for ( i = 0; i < 32; i++ ) {
		char buf[8];
		sprintf(buf, "xmm%d", i);
		// TODO: 16 x 16-byte regs
		//rb_hash_aset( h, rb_str_new_cstr(buf), 
		//	      UINT2NUM(regs.xmm_space[i]) );
	}
#  endif
#elif defined(__APPLE__)
#  ifdef __x86_64__
#  else
#  endif
#endif
	return h;
}

static VALUE ptrace_set_fpxregs( VALUE cls, VALUE pid, VALUE h ) {

#ifdef __linux
	long rv = 0;
	int i;
#  ifdef __x86_64__
	// Nothing to do: x86-64 has no fpxregs structure
#  else
	struct user_fpxregs_struct regs = {0};

	regs.cwd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("cwd") ));
	regs.swd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("swd") ));
	regs.twd = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("twd") ));
	regs.fop = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fop") ));
	regs.fip = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fip") ));
	regs.fcs = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fcs") ));
	regs.foo = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("foo") ));
	regs.fos = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("fos") ));
	regs.mxcsr = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("mxcsr") ));
	for ( i = 0; i < 32; i++ ) {
		char buf[8];
		sprintf(buf, "ST(%d)", i);
		// TODO: 8 * 16-byte regs
		//regs.st_space[i] = NUM2UINT(hash_get_int( h, 
		//				rb_str_new_cstr(buf) ));
	}
	for ( i = 0; i < 32; i++ ) {
		char buf[8];
		sprintf(buf, "xmm%d", i);
		// TODO : 16 x 16-byte regs
		//regs.xmm_space[i] = NUM2UINT(hash_get_int( h, 
		//				rb_str_new_cstr(buf) ));
	}
	rv = int_ptrace_raw( PT_SETFPXREGS, pid, NULL, &regs);
#  endif
#elif defined(__APPLE__)
#  ifdef __x86_64__
#  else
#  endif
#endif
	return Qnil;
}

static VALUE ptrace_get_siginfo( VALUE cls, VALUE pid ) {
	VALUE h = rb_hash_new();
#ifdef PT_GETSIGINFO
#  ifdef __linux
	siginfo_t sig = {0};

	int rv = int_ptrace_raw( PT_GETSIGINFO, pid, NULL, &sig);

	rb_hash_aset( h, rb_str_new_cstr("signo"), UINT2NUM(sig.si_signo) );
	rb_hash_aset( h, rb_str_new_cstr("errno"), UINT2NUM(sig.si_errno) );
	rb_hash_aset( h, rb_str_new_cstr("code"), UINT2NUM(sig.si_code) );
	//rb_hash_aset( h, rb_str_new_cstr("trapno"), UINT2NUM(sig.si_trapno) );
	rb_hash_aset( h, rb_str_new_cstr("pid"), pid_to_num(sig.si_pid) );
	rb_hash_aset( h, rb_str_new_cstr("uid"), UIDTNUM(sig.si_uid) );
	rb_hash_aset( h, rb_str_new_cstr("status"), UINT2NUM(sig.si_status) );
	rb_hash_aset( h, rb_str_new_cstr("utime"), UINT2NUM(sig.si_utime) );
	rb_hash_aset( h, rb_str_new_cstr("stime"), UINT2NUM(sig.si_stime) );
	//rb_hash_aset( h, rb_str_new_cstr("value"), UINT2NUM(sig.si_value) );
	rb_hash_aset( h, rb_str_new_cstr("int"), UINT2NUM(sig.si_int) );
	//rb_hash_aset( h, rb_str_new_cstr("ptr"), ULONG2NUM(sig.si_ptr) );
	rb_hash_aset( h, rb_str_new_cstr("overrun"), UINT2NUM(sig.si_overrun) );
	rb_hash_aset( h, rb_str_new_cstr("timerid"), UINT2NUM(sig.si_timerid) );
	//rb_hash_aset( h, rb_str_new_cstr("addr"), ULONG2NUM(sig.si_addr) );
	rb_hash_aset( h, rb_str_new_cstr("band"), UINT2NUM(sig.si_band) );
	rb_hash_aset( h, rb_str_new_cstr("fd"), UINT2NUM(sig.si_fd) );
#  elif defined(__APPLE__)
#  endif
#endif
	return h;
}
static VALUE ptrace_set_siginfo( VALUE cls, VALUE pid, VALUE hash ) {
	VALUE rv = Qnil;
#ifdef PT_SET_SIGINFO
#  ifdef __linux
	siginfo_t sig = {0};

	sig.si_signo = NUM2UINT(hash_get_int( h, rb_str_new_cstr("signo") ));
	sig.si_errno = NUM2UINT(hash_get_int( h, rb_str_new_cstr("errno") ));
	sig.si_code = NUM2UINT(hash_get_int( h, rb_str_new_cstr("code") ));
	sig.si_trapno = NUM2UINT(hash_get_int( h, rb_str_new_cstr("trapno") ));
	sig.si_pid = num_to_pid(hash_get_int( h, rb_str_new_cstr("pid") ));
	sig.si_uid = NUM2UIDT(hash_get_int( h, rb_str_new_cstr("uid") ));
	sig.si_status = NUM2UINT(hash_get_int( h, rb_str_new_cstr("status") ));
	sig.si_utime = NUM2UINT(hash_get_int( h, rb_str_new_cstr("utime") ));
	sig.si_stime = NUM2UINT(hash_get_int( h, rb_str_new_cstr("stime") ));
	sig.si_value = NUM2UINT(hash_get_int( h, rb_str_new_cstr("value") ));
	sig.si_int = NUM2UINT(hash_get_int( h, rb_str_new_cstr("int") ));
	sig.si_ptr = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("ptr") ));
	sig.si_overrun = NUM2UINT(hash_get_int(h, rb_str_new_cstr("overrun")));
	sig.si_timerid = NUM2UINT(hash_get_int(h, rb_str_new_cstr("timerid")));
	sig.si_addr = NUM2ULONG(hash_get_int( h, rb_str_new_cstr("addr") ));
	sig.si_band = NUM2UINT(hash_get_int( h, rb_str_new_cstr("band") ));
	sig.si_fd = NUM2UINT(hash_get_int( h, rb_str_new_cstr("fd") ));

	int_ptrace_raw( PT_SETSIGINFO, pid, NULL, &sig);
#  elif defined(__APPLE__)
#  endif
#endif
	return rv;
}

static VALUE ptrace_eventmsg( VALUE cls, VALUE pid ) {
	VALUE rv = Qnil;
#ifdef PT_GETEVENTMSG
	unsigned long long msg;

	int_ptrace_raw( PT_GETEVENTMSG, pid, NULL, &msg);
	rv = ULL2NUM(msg);
#endif
	return rv;
}

/* ---------------------------------------------------------------------- */
/* Debugger Class */

static void init_debugger_class( VALUE modPtrace ) {
	clsDebug = rb_define_class_under(modPtrace, DEBUGGER_CLASS_NAME, 
					    rb_cObject);

	rb_define_singleton_method(clsDebug, "commands", build_cmd_hash, 0);
	rb_define_singleton_method(clsDebug, "syscalls", build_syscall_hash, 0);

	rb_define_singleton_method(clsDebug, "send_cmd", ptrace_send, 3);
	rb_define_singleton_method(clsDebug, "send_data", ptrace_send_data, 4);
	rb_define_singleton_method(clsDebug, "peek", ptrace_peek, 3);
	rb_define_singleton_method(clsDebug, "poke", ptrace_poke, 4);
	rb_define_singleton_method(clsDebug, "regs", ptrace_get_regs, 1);
	rb_define_singleton_method(clsDebug, "regs=", ptrace_set_regs, 2);
	rb_define_singleton_method(clsDebug, "fpregs", ptrace_get_fpregs, 1);
	rb_define_singleton_method(clsDebug, "fpregs=", ptrace_set_fpregs, 2);
	rb_define_singleton_method(clsDebug, "fpxregs", ptrace_get_fpxregs, 1);
	rb_define_singleton_method(clsDebug, "fpxregs=", ptrace_set_fpxregs, 2);
	rb_define_singleton_method(clsDebug, "signal", ptrace_get_siginfo, 1);
	rb_define_singleton_method(clsDebug, "signal=", ptrace_set_siginfo, 2);
	rb_define_singleton_method(clsDebug, "event_msg", ptrace_eventmsg, 1);
}

/* ---------------------------------------------------------------------- */
/* USER */

/* ---------------------------------------------------------------------- */
/* Ptrace Module */

void Init_Ptrace_ext() {
	modPtrace = rb_define_module(PTRACE_MODULE_NAME);
	clsError = rb_define_class_under(rb_eRuntimeError, 
					 PTRACE_ERROR_CLASS_NAME, 
					 rb_cObject);

	init_debugger_class(modPtrace);
}
