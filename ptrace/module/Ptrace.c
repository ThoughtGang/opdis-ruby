/* Ptrace.c
 * Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <sys/ptrace.h>

#include <ruby.h>
#include "ruby_compat.h"

#include "Ptrace.h"

#define IVAR(attr) "@" attr

static VALUE modPtrace;
static VALUE clsDebug;


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
	CMD_HASH_ADD(h, SZ_PTRACE_PEEKUSR, PTRACE_PEEKUSR)
	CMD_HASH_ADD(h, SZ_PTRACE_POKETEXT, PTRACE_POKETEXT)
	CMD_HASH_ADD(h, SZ_PTRACE_POKEDATA, PTRACE_POKEDATA)
	CMD_HASH_ADD(h, SZ_PTRACE_POKEUSR, PTRACE_POKEUSR)
	CMD_HASH_ADD(h, SZ_PTRACE_GETREGS, PTRACE_GETREGS)
	CMD_HASH_ADD(h, SZ_PTRACE_GETFPREGS, PTRACE_GETFPREGS)
	CMD_HASH_ADD(h, SZ_PTRACE_SETREGS, PTRACE_SETREGS)
	CMD_HASH_ADD(h, SZ_PTRACE_SETFPREGS, PTRACE_SETFPREGS)
	CMD_HASH_ADD(h, SZ_PTRACE_CONT, PTRACE_CONT)
	CMD_HASH_ADD(h, SZ_PTRACE_SYSCALL, PTRACE_SYSCALL)
	CMD_HASH_ADD(h, SZ_PTRACE_SINGLESTEP, PTRACE_SINGLESTEP)
	CMD_HASH_ADD(h, SZ_PTRACE_KILL, PTRACE_KILL)
	CMD_HASH_ADD(h, SZ_PTRACE_ATTACH, PTRACE_ATTACH)
	CMD_HASH_ADD(h, SZ_PTRACE_DETACH, PTRACE_DETACH)
// #ifdef linux
	CMD_HASH_ADD(h, SZ_PTRACE_SETOPTIONS, PTRACE_SETOPTIONS)
	CMD_HASH_ADD(h, SZ_PTRACE_GETSIGINFO, PTRACE_GETSIGINFO)
	CMD_HASH_ADD(h, SZ_PTRACE_SETSIGINFO, PTRACE_SETSIGINFO)
	CMD_HASH_ADD(h, SZ_PTRACE_GETEVENTMSG, PTRACE_GETEVENTMSG)
	CMD_HASH_ADD(h, SZ_PTRACE_SYSEMU, PTRACE_SYSEMU)
	CMD_HASH_ADD(h, SZ_PTRACE_SYSEMU_SINGLESTEP, PTRACE_SYSEMU_SINGLESTEP)

	return h;
}

/* ---------------------------------------------------------------------- */

static long int_ptrace_raw( enum __ptrace_request req, VALUE pid, void * addr, 
			    void * data )  {
	pid_t tgt;
	long rv;

	tgt = PIDT2NUM(pid);
	return ptrace(req, tgt, addr, data);
	//if (rv == -1) strerror(err)
}

// internal wrapper for ptrace that converts PID to a pid_t, and converts
// return value to a Fixnum
static VALUE int_ptrace( enum __ptrace_request req, VALUE pid, void * addr, 
			void * data )  {
	long rv = int_ptrace_raw(req, tgt, addr, data);
	return LONG2NUM(rv);
}

static VALUE int_ptrace_data( VALUE req, VALUE pid, VALUE addr, void * data ) {
	enum __ptrace_request cmd = (enum __ptrace_request_cmd) NUM2UINT(req);
	void * tgt_addr = (void *) NUM2ULONG(addr);

	return int_ptrace(cmd, pid, tgt_addr, data);
}


/* ---------------------------------------------------------------------- */
/* PTRACE API */


// generic (non-arg) use:
//static VALUE ptrace_send( enum __ptrace_request req,  pid_t pid, void * addr) 
static VALUE ptrace_send( VALUE req, VALUE pid, VALUE addr) {
	return int_ptrace_data( req, pid, addr, NULL );
}

// includes external ptrace_set_options
// NOTE only use this for data that is NOT a memory address!
//static VALUE ptrace_send_data( enum __ptrace_request req, pid_t pid, 
//			       void * addr, void * data ) {
static VALUE ptrace_send_data( VALUE req, VALUE pid, VALUE addr, VALUE data ) {
	void * the_data;
	// convert data to void * (from Fixnum)

	return int_ptrace_send(req, pid, addr, the_data);
}

//peek_text, peek_data, peek_user
//static VALUE ptrace_peek( enum __ptrace_request req, pid_t pid, void * addr ) 
static VALUE ptrace_peek( VALUE type, VALUE pid, VALUE addr ) {
	enum __ptrace_request cmd;
	void * tgt_addr = (void *) NUM2ULONG(addr); 
	long rv;
	VALUE word;

	rv = int_ptrace_raw(type, pid, tgt_addr);

	// convert rv to binary string

	return word;
}

//poke_text, poke_data, poke_user
//static VALUE ptrace_poke( enum __ptrace_request req, pid_t pid, void * addr,
//			  void * data ) {
static VALUE ptrace_poke( VALUE req, VALUE pid, VALUE addr, VALUE data ) {
	void * the_data;

	// convert data (Fixnum or Binary String) to word
	return int_ptrace_send(req, pid, addr, the_data);
}

static VALUE ptrace_get_regs( pid_t * pid ) {
	// alloc reg struct
	// int_ptrace
	// data to Hash
	return Qnil;
}

static VALUE ptrace_set_regs( pid_t * pid, VALUE hash ) {
	// alloc reg struct
	// hash to data
	// int_ptrace
	return Qnil;
}

static VALUE ptrace_get_fpregs( pid_t * pid ) {
	// alloc reg struct
	// int_ptrace
	// data to Hash
	return Qnil;
}

static VALUE ptrace_set_fpregs( pid_t * pid, VALUE hash ) {
	// alloc reg struct
	// hash to data
	// int_ptrace
	return Qnil;
}

static VALUE ptrace_get_siginfo( pid_t * pid ) {
	// alloc sig struct
	// int_ptrace
	// data to Hash
	return Qnil;
}
static VALUE ptrace_set_siginfo( pid_t * pid, VALUE hash ) {
	// alloc sig struct
	// hash to data
	// int_ptrace
	return Qnil;
}

static VALUE ptrace_eventmsg( pid_t * pid ) {
	// alloc msg struct
	// int_ptrace
	// data to Hash
	return Qnil;
}

/* ---------------------------------------------------------------------- */
/* Debugger Class */

static void init_debugger_class( VALUE modPtrace ) {
	clsDebug = rb_define_class_under(modPtrace, DEBUGGER_CLASS_NAME, 
					    rb_cObject);

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
/* BFD Module */

void Init_Ptrace_ext() {
	modPtrace = rb_define_module(PTRACE_MODULE_NAME);

	init_debugger_class(modPtrace);
}
