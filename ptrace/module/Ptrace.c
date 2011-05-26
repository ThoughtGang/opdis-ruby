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
static VALUE clsDebugger;


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

// internal wrapper for ptrace that converts PID to a pid_t, and converts
// return value to a Fixnum
static VALUE int_ptrace( enum __ptrace_request req, VALUE pid, void * addr, 
			void * data )  {
}

/* ---------------------------------------------------------------------- */
/* PTRACE API */

// generic (non-arg) use:
//static VALUE ptrace_send( enum __ptrace_request req,  pid_t pid, void * addr) 
static VALUE ptrace_send( VALUE req, VALUE pid, VALUE addr) {
	// convert req to __ptrace_request
	// convert addr to void * (from Fixnum)
	// int_ptrace
}

// for internal use: send with a data param
// includes external ptrace_set_options
// NOTE only use this for data that is NOT a memory address!
//static VALUE ptrace_send_data( enum __ptrace_request req, pid_t pid, 
//			       void * addr, void * data ) {
static VALUE ptrace_send_data( VALUE req, VALUE pid, VALUE addr, VALUE data ) {
	// convert req to __ptrace_request
	// convert addr to void * (from Fixnum)
	// convert data to void * (from Fixnum)
	// int_ptrace
}

//peek_text, peek_data, peek_user
//static VALUE ptrace_peek( enum __ptrace_request req, pid_t pid, void * addr ) 
static VALUE ptrace_peek( VALUE type, VALUE pid, VALUE addr ) {
	// convert req to __ptrace_request
	// convert addr to void * (from Fixnum)
	// alloc a word
	// int_ptrace
	// convert word to binary string
}

//poke_text, poke_data, poke_user
//static VALUE ptrace_poke( enum __ptrace_request req, pid_t pid, void * addr,
//			  void * data ) {
static VALUE ptrace_poke( VALUE req, VALUE pid, VALUE addr, VALUE data ) {
	// convert req to __ptrace_request
	// convert addr to void * (from Fixnum)
	// alloc a word
	// convert data (Fixnum or Binary String) to word
	// int_ptrace
}

static VALUE ptrace_get_regs( pid_t * pid ) {
	// int_ptrace
	// data to Hash
}

static VALUE ptrace_set_regs( pid_t * pid, VALUE hash ) {
	// hash to data
	// int_ptrace
}

static VALUE ptrace_get_fpregs( pid_t * pid ) {
	// int_ptrace
	// data to Hash
}

static VALUE ptrace_set_fpregs( pid_t * pid, VALUE hash ) {
	// hash to data
	// int_ptrace
}

static VALUE ptrace_get_siginfo( pid_t * pid ) {
	// int_ptrace
	// data to Hash
}
static VALUE ptrace_set_siginfo( pid_t * pid, VALUE hash ) {
	// hash to data
	// int_ptrace
}

static VALUE ptrace_get_eventmsg( pid_t * pid ) {
	// int_ptrace
	// data to Hash
}

/* ---------------------------------------------------------------------- */
/* Debugger Class */

static void init_debugger_class( VALUE modPtrace ) {
	clsDebugger = rb_define_class_under(modPtrace, DEBUGGER_CLASS_NAME, 
					    rb_cObject);

	rb_define_singleton_method(clsDebugger, "", ptrace_send, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_send_data, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_peek, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_poke, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_get_regs, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_set_regs, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_get_fpregs, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_set_fpregs, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_get_siginfo, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_set_siginfo, 2);
	rb_define_singleton_method(clsDebugger, "", ptrace_get_eventmsg, 2);
}

/* ---------------------------------------------------------------------- */
/* BFD Module */

void Init_Ptrace_ext() {
	modPtrace = rb_define_module(PTRACE_MODULE_NAME);

	init_debugger_class(modPtrace);
}
