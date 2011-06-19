/* Ptrace.c
 * Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>

// TODO: how to represent a 128-bit number in C and ruby
// gcc: unsigned __int128 
//  BigDecimal.new("0.0001")
//   require 'bigdecimal'


/* yup, ptrace is not actually POSIX... */
#ifdef __APPLE__
#  define PTRACE_TRACE_ME PT_TRACE_ME
#  define PTRACE_PEEK_TEXT PT_READ_I
#  define PTRACE_PEEK_DATA PT_READ_D
#  define PTRACE_PEEK_USER PT_READ_U
#  define PTRACE_POKE_TEXT PT_WRITE_I
#  define PTRACE_POKE_DATA PT_WRITE_D
#  define PTRACE_POKE_USER PT_WRITE_U
#  define PTRACE_CONT PT_CONTINUE
#  define PTRACE_SINGLESTEP PT_STEP
#  define PTRACE_KILL PT_KILL
#  define PTRACE_ATTACH PT_ATTACH
#  define PTRACE_DETACH PT_DETACH
#endif

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

static VALUE build_cmd_hash( void ) {
	VALUE h = rb_hash_new();

	CMD_HASH_ADD(h, SZ_PTRACE_TRACEME, PTRACE_TRACEME) 
	CMD_HASH_ADD(h, SZ_PTRACE_PEEKTEXT, PTRACE_PEEKTEXT)
	CMD_HASH_ADD(h, SZ_PTRACE_PEEKDATA, PTRACE_PEEKDATA) 
	CMD_HASH_ADD(h, SZ_PTRACE_POKETEXT, PTRACE_POKETEXT)
	CMD_HASH_ADD(h, SZ_PTRACE_POKEDATA, PTRACE_POKEDATA)
	CMD_HASH_ADD(h, SZ_PTRACE_CONT, PTRACE_CONT)
	CMD_HASH_ADD(h, SZ_PTRACE_SINGLESTEP, PTRACE_SINGLESTEP)
	CMD_HASH_ADD(h, SZ_PTRACE_KILL, PTRACE_KILL)
	CMD_HASH_ADD(h, SZ_PTRACE_ATTACH, PTRACE_ATTACH)
	CMD_HASH_ADD(h, SZ_PTRACE_DETACH, PTRACE_DETACH)

	/* ====================================================== */
	/* These may not be present in all ptrace implementations */
#ifdef PTRACE_POKEUSR
	CMD_HASH_ADD(h, SZ_PTRACE_POKEUSR, PTRACE_POKEUSR)
#endif
#ifdef PTRACE_PEEKUSR
	CMD_HASH_ADD(h, SZ_PTRACE_PEEKUSR, PTRACE_PEEKUSR)
#endif
#ifdef PTRACE_SYSCALL
	CMD_HASH_ADD(h, SZ_PTRACE_SYSCALL, PTRACE_SYSCALL)
#endif
#ifdef PTRACE_GETREGS
	CMD_HASH_ADD(h, SZ_PTRACE_GETREGS, PTRACE_GETREGS)
#endif
#ifdef PTRACE_GETFPREGS
	CMD_HASH_ADD(h, SZ_PTRACE_GETFPREGS, PTRACE_GETFPREGS)
#endif
#ifdef PTRACE_SETREGS
	CMD_HASH_ADD(h, SZ_PTRACE_SETREGS, PTRACE_SETREGS)
#endif
#ifdef PTRACE_SETFPREGS
	CMD_HASH_ADD(h, SZ_PTRACE_SETFPREGS, PTRACE_SETFPREGS)
#endif
#ifdef PTRACE_SETOPTIONS
	CMD_HASH_ADD(h, SZ_PTRACE_SETOPTIONS, PTRACE_SETOPTIONS)
#endif
#ifdef PTRACE_GETSIGINFO
	CMD_HASH_ADD(h, SZ_PTRACE_GETSIGINFO, PTRACE_GETSIGINFO)
#endif
#ifdef PTRACE_SETSIGINFO
	CMD_HASH_ADD(h, SZ_PTRACE_SETSIGINFO, PTRACE_SETSIGINFO)
#endif
#ifdef PTRACE_GETEVENTMSG
	CMD_HASH_ADD(h, SZ_PTRACE_GETEVENTMSG, PTRACE_GETEVENTMSG)
#endif
#ifdef PTRACE_SYSEMU
	CMD_HASH_ADD(h, SZ_PTRACE_SYSEMU, PTRACE_SYSEMU)
#endif
#ifdef PTRACE_SYSEMU_SINGLESTEP
	CMD_HASH_ADD(h, SZ_PTRACE_SYSEMU_SINGLESTEP, PTRACE_SYSEMU_SINGLESTEP)
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
	CMD_HASH_ADD(h, SZ_PTRACE_TRACEME, PTRACE_TRACEME) 
}

/* ---------------------------------------------------------------------- */

static long int_ptrace_raw( enum __ptrace_request req, VALUE pid, void * addr, 
			    void * data )  {
	pid_t tgt;
	long rv;

	tgt = PIDT2NUM(pid);
	rv = ptrace(req, tgt, addr, data);
	if (rv == -1) {
		 rb_raise(rb_eRuntimeError, "%s", strerror(errno));
	}

	rv;
}

/* internal wrapper for ptrace that converts PID to a pid_t, and converts
 * return value to a Fixnum */
static VALUE int_ptrace( enum __ptrace_request req, VALUE pid, void * addr, 
			void * data )  {
	long rv = int_ptrace_raw(req, pid, addr, data);
	return LONG2NUM(rv);
}

static VALUE int_ptrace_data( VALUE req, VALUE pid, VALUE addr, void * data ) {
	enum __ptrace_request cmd = (enum __ptrace_request) NUM2UINT(req);
	void * tgt_addr = (void *) NUM2ULONG(addr);

	return int_ptrace(cmd, pid, tgt_addr, data);
}


/* ---------------------------------------------------------------------- */
/* PTRACE API */

static VALUE ptrace_send( VALUE req, VALUE pid, VALUE addr) {
	return int_ptrace_data( req, pid, addr, NULL );
}

/* NOTE only use this for data that is NOT a memory address (pointer)! */
static VALUE ptrace_send_data( VALUE req, VALUE pid, VALUE addr, VALUE data ) {
	void * the_data = (void *) NUM2ULONG(data);

	return int_ptrace_data(req, pid, addr, the_data);
}

/* peek_text, peek_data, peek_user */
static VALUE ptrace_peek( VALUE type, VALUE pid, VALUE addr ) {
	void * tgt_addr = (void *) NUM2ULONG(addr); 
	long rv;

	rv = int_ptrace_raw(type, pid, tgt_addr, NULL);

	return ULONG2NUM(rv);
}

/* poke_text, poke_data, poke_user */
static VALUE ptrace_poke( VALUE req, VALUE pid, VALUE addr, VALUE data ) {
	void * the_data = (void *) NUM2ULONG(data);

	return int_ptrace_data(req, pid, addr, the_data);
}

static VALUE ptrace_get_regs( VALUE pid ) {
	VALUE h = rb_hash_new();

#ifdef __linux
	long rv = 0;
	struct user_regs_struct regs = {0};

	rv = int_ptrace_raw( PTRACE_GETREGS, pid, NULL, &regs);

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

static VALUE ptrace_set_regs( VALUE pid, VALUE hash ) {
	VALUE h = rb_hash_new();

#ifdef __linux
	long rv = 0;
	struct user_regs_struct regs = {0};

#  ifdef __x86_64__
	regs.r15 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r15") ));
	regs.r14 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r14") ));
	regs.r13 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r13") ));
	regs.r12 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r12") ));
	regs.rbp = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rbp") ));
	regs.rbx = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rbx") ));
	regs.r11 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r11") ));
	regs.r10 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r10") ));
	regs.r9 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r9") ));
	regs.r8 = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("r8") ));
	regs.rax = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rax") ));
	regs.rcx = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rcx") ));
	regs.rdx = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rdx") ));
	regs.rsi = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rsi") ));
	regs.rdi = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rdi") ));
	regs.orig_rax = NUM2ULONG(rb_hash_fetch( h, 
						rb_str_new_cstr("orig_rax") ));
	regs.rip = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rip") ));
	regs.cs = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("cs") ));
	regs.eflags = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("eflags") ));
	regs.rsp = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rsp") ));
	regs.ss = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("ss") ));
	regs.fs_base = NUM2ULONG(rb_hash_fetch( h, 
						rb_str_new_cstr("fs_base") ));
	regs.gs_base = NUM2ULONG(rb_hash_fetch( h, 
						rb_str_new_cstr("gs_base") ));
	regs.ds = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("ds") ));
	regs.es = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("es") ));
	regs.fs = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("fs") ));
	regs.gs = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("gs") ));
#  else
	// hash to data
#  endif
	rv = int_ptrace_raw( PTRACE_SETREGS, pid, NULL, &regs);
#endif
	return Qnil;
}

static VALUE ptrace_get_fpregs( VALUE pid ) {
	VALUE h = rb_hash_new();
#ifdef __linux
	long rv = 0;
	int i;
	struct user_fpregs_struct regs = {0};

	rv = int_ptrace_raw( PTRACE_GETFPREGS, pid, NULL, &regs);

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

static VALUE ptrace_set_fpregs( VALUE pid, VALUE hash ) {
	VALUE h = rb_hash_new();

#ifdef __linux
	long rv = 0;
	int i;
	struct user_fpregs_struct regs = {0};
#  ifdef __x86_64__
	regs.cwd = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("cwd") ));
	regs.swd = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("swd") ));
	regs.ftw = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("ftw") ));
	regs.fop = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("fop") ));
	regs.rip = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rip") ));
	regs.rdp = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("rdp") ));
	regs.mxcsr = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("mxcsr") ));
	regs.mxcr_mask = NUM2ULONG(rb_hash_fetch( h, 
					rb_str_new_cstr("mxcr_mask") ));
	for ( i = 0; i < 32; i++ ) {
		char buf[8];
		sprintf(buf, "ST(%d)", i);
		// TODO: 8 * 16-byte regs
		//regs.st_space[i] = NUM2UINT(rb_hash_fetch( h, 
		//				rb_str_new_cstr(buf) ));
	}
	for ( i = 0; i < 64; i++ ) {
		char buf[8];
		sprintf(buf, "xmm%d", i);
		// TODO : 16 x 16-byte regs
		//regs.xmm_space[i] = NUM2UINT(rb_hash_fetch( h, 
		//				rb_str_new_cstr(buf) ));
	}
#  else
#  endif
	rv = int_ptrace_raw( PTRACE_SETFPREGS, pid, NULL, &regs);
#elif defined(__APPLE__)
#  ifdef __x86_64__
#  else
#  endif
#endif
	return Qnil;
}

static VALUE ptrace_get_siginfo( VALUE pid ) {
	VALUE h = rb_hash_new();
#ifdef PTRACE_GETSIGINFO
#  ifdef __linux
	siginfo_t sig = {0};

	rv = int_ptrace_raw( PTRACE_GETSIGINFO, pid, NULL, &sig);

	rb_hash_aset( h, rb_str_new_cstr("signo"), UINT2NUM(sig.si_signo) );
	rb_hash_aset( h, rb_str_new_cstr("errno"), UINT2NUM(sig.si_errno) );
	rb_hash_aset( h, rb_str_new_cstr("code"), UINT2NUM(sig.si_code) );
	rb_hash_aset( h, rb_str_new_cstr("trapno"), UINT2NUM(sig.si_trapno) );
	rb_hash_aset( h, rb_str_new_cstr("pid"), PIDTNUM(sig.si_pid) );
	rb_hash_aset( h, rb_str_new_cstr("uid"), UIDTNUM(sig.si_uid) );
	rb_hash_aset( h, rb_str_new_cstr("status"), UINT2NUM(sig.si_status) );
	rb_hash_aset( h, rb_str_new_cstr("utime"), UINT2NUM(sig.si_utime) );
	rb_hash_aset( h, rb_str_new_cstr("stime"), UINT2NUM(sig.si_stime) );
	rb_hash_aset( h, rb_str_new_cstr("value"), UINT2NUM(sig.si_value) );
	rb_hash_aset( h, rb_str_new_cstr("int"), UINT2NUM(sig.si_int) );
	rb_hash_aset( h, rb_str_new_cstr("ptr"), ULONG2NUM(sig.si_ptr) );
	rb_hash_aset( h, rb_str_new_cstr("overrun"), UINT2NUM(sig.si_overrun) );
	rb_hash_aset( h, rb_str_new_cstr("timerid"), UINT2NUM(sig.si_timerid) );
	rb_hash_aset( h, rb_str_new_cstr("addr"), ULONG2NUM(sig.si_addr) );
	rb_hash_aset( h, rb_str_new_cstr("band"), UINT2NUM(sig.si_band) );
	rb_hash_aset( h, rb_str_new_cstr("fd"), UINT2NUM(sig.si_fd) );
#  elif defined(__APPLE__)
#  endif
#endif
	return h;
}
static VALUE ptrace_set_siginfo( VALUE pid, VALUE hash ) {
	VALUE rv = Qnil;
#ifdef PTRACE_SET_SIGINFO
#  ifdef __linux
	siginfo_t sig = {0};

	sig.si_signo = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("signo") ));
	sig.si_errno = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("errno") ));
	sig.si_code = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("code") ));
	sig.si_trapno = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("trapno") ));
	sig.si_pid = NUM2PIDT(rb_hash_fetch( h, rb_str_new_cstr("pid") ));
	sig.si_uid = NUM2UIDT(rb_hash_fetch( h, rb_str_new_cstr("uid") ));
	sig.si_status = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("status") ));
	sig.si_utime = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("utime") ));
	sig.si_stime = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("stime") ));
	sig.si_value = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("value") ));
	sig.si_int = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("int") ));
	sig.si_ptr = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("ptr") ));
	sig.si_overrun = NUM2UINT(rb_hash_fetch(h, rb_str_new_cstr("overrun")));
	sig.si_timerid = NUM2UINT(rb_hash_fetch(h, rb_str_new_cstr("timerid")));
	sig.si_addr = NUM2ULONG(rb_hash_fetch( h, rb_str_new_cstr("addr") ));
	sig.si_band = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("band") ));
	sig.si_fd = NUM2UINT(rb_hash_fetch( h, rb_str_new_cstr("fd") ));

	int_ptrace_raw( PTRACE_SETSIGINFO, pid, NULL, &sig);
#  elif defined(__APPLE__)
#  endif
#endif
	return rv;
}

static VALUE ptrace_eventmsg( VALUE pid ) {
	VALUE rv = Qnil;
#ifdef PTRACE_GETEVENTMSG
	unsigned long long msg;

	int_ptrace_raw( PTRACE_GETEVENTMSG, pid, NULL, &msg);
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

	rb_define_singleton_method(clsDebug, "send", ptrace_send, 3);
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
