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

/* ---------------------------------------------------------------------- */
//symFileno = rb_intern("fileno");
// rb_raise(rb_eRuntimeError, "BFD error (%d): %s", err, bfd_errmsg(err) );
// VALUE instance, var;
// if ( rb_respond_to( tgt, symFileno ) ) {
// } else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString) ) {
// char * path = StringValuePtr(tgt);
// VALUE rb_path = rb_funcall(tgt, symPath, 0);
// if ( Qnil == fd_val ) {
// fd = NUM2INT(fd_val);
//	rb_hash_aset( *hash, str_to_sym(AINFO_MEMBER_ARCH), 
//		      rb_str_new_cstr(bfd_printable_arch_mach(info->arch,
//							      info->mach)) );
// var = rb_hash_new();
/* ---------------------------------------------------------------------- */

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
#ifdef PT_SETREGS
	CMD_HASH_ADD(h, SZ_PTRACE_SETREGS, PT_SETREGS)
#endif
#ifdef PT_SETFPREGS
	CMD_HASH_ADD(h, SZ_PTRACE_SETFPREGS, PT_SETFPREGS)
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

	// int_ptrace
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
	// TODO
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
	// x86 hash to data
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
#  endif
	rv = int_ptrace_raw( PT_SETFPREGS, pid, NULL, &regs);
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

	rb_define_singleton_method(clsDebug, "send_cmd", ptrace_send, 3);
	rb_define_singleton_method(clsDebug, "send_data", ptrace_send_data, 4);
	rb_define_singleton_method(clsDebug, "peek", ptrace_peek, 3);
	rb_define_singleton_method(clsDebug, "poke", ptrace_poke, 4);
	rb_define_singleton_method(clsDebug, "regs", ptrace_get_regs, 1);
	rb_define_singleton_method(clsDebug, "regs=", ptrace_set_regs, 2);
	rb_define_singleton_method(clsDebug, "fpregs", ptrace_get_fpregs, 1);
	rb_define_singleton_method(clsDebug, "fpregs=", ptrace_set_fpregs, 2);
	rb_define_singleton_method(clsDebug, "signal", ptrace_get_siginfo, 1);
	rb_define_singleton_method(clsDebug, "signal=", ptrace_set_siginfo, 2);
	rb_define_singleton_method(clsDebug, "event_msg", ptrace_eventmsg, 1);
}

/* ---------------------------------------------------------------------- */
/* USER */
/* ---------------------------------------------------------------------- */
/* BFD Module */

void Init_Ptrace_ext() {
	modPtrace = rb_define_module(PTRACE_MODULE_NAME);
	clsError = rb_define_class_under(rb_eRuntimeError, 
					 PTRACE_ERROR_CLASS_NAME, 
					 rb_cObject);

	init_debugger_class(modPtrace);
}
