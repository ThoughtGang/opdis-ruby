/* Ptrace.h
 * Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef PTRACE_RUBY_EXTENSION_H
#define PTRACE_RUBY_EXTENSION_H

#define SZ_PTRACE_TRACEME "traceme"
#define SZ_PTRACE_PEEKTEXT "peektext"
#define SZ_PTRACE_PEEKDATA "peekdata"
#define SZ_PTRACE_PEEKUSR "peekusr"
#define SZ_PTRACE_POKETEXT "poketext"
#define SZ_PTRACE_POKEDATA "pokedata"
#define SZ_PTRACE_POKEUSR "pokeusr"
#define SZ_PTRACE_GETREGS "getregs"
#define SZ_PTRACE_GETFPREGS "getfpregs"
#define SZ_PTRACE_GETFPXREGS "getfpxregs"
#define SZ_PTRACE_SETREGS "setregs"
#define SZ_PTRACE_SETFPREGS "setfpregs"
#define SZ_PTRACE_SETFPXREGS "setfpxregs"
#define SZ_PTRACE_CONT "cont"
#define SZ_PTRACE_SYSCALL "syscall"
#define SZ_PTRACE_SINGLESTEP "singlestep"
#define SZ_PTRACE_KILL "kill"
#define SZ_PTRACE_ATTACH "attach"
#define SZ_PTRACE_DETACH  "detach"
#define SZ_PTRACE_GETSIGINFO "getsiginfo"
#define SZ_PTRACE_SETSIGINFO "setsiginfo"
#define SZ_PTRACE_SETOPTIONS "setoptions"
#define SZ_PTRACE_GETEVENTMSG "geteventmsg"
#define SZ_PTRACE_SYSEMU "sysemu"
#define SZ_PTRACE_SYSEMU_SINGLESTEP  "sysemu_singlestep"

#define SZ_PTRACE_O_TRACESYSGOOD "trace_sys_good"
#define SZ_PTRACE_O_TRACEFORK "trace_fork"
#define SZ_PTRACE_O_TRACEVFORK "trace_vfork"
#define SZ_PTRACE_O_TRACEVFORKDONE "trace_vfork_done"
#define SZ_PTRACE_O_TRACECLONE "trace_clone"
#define SZ_PTRACE_O_TRACEEXEC "trace_exec"
#define SZ_PTRACE_O_TRACEEXIT "trace_exit"

/* module and class names */
#define PTRACE_MODULE_NAME "Ptrace"
#define PTRACE_ERROR_CLASS_NAME "PtraceError"
#define DEBUGGER_CLASS_NAME "Debugger"

void Init_Ptrace_ext();

#endif
