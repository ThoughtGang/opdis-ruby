/* Opdis.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <errno.h>
#include <string.h>

#include <ruby.h>
#ifndef RUBY_19
#include <ruby1.8_compat.h>
#endif

#include <opdis/opdis.h>

#include "Opdis.h"
#include "Callbacks.h"
#include "Model.h"

#define IVAR(attr) "@" attr
#define SETTER(attr) attr "="

static VALUE symToSym, symRead, symCall, symSize, symPath;
static VALUE symDecode, symVisited, symResolve;

static VALUE clsDisasm, clsOutput;

static VALUE modOpdis;

static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, symToSym, 0);
}

/* BFD Support (requires BFD gem) */
static VALUE clsBfdTgt = Qnil;
static VALUE clsBfdSec = Qnil;
static VALUE clsBfdSym = Qnil;

#define GET_BFD_CLASS(cls,name) (cls = cls == Qnil ? path2class(name) : cls)

#define ALLOC_FIXED_INSN opdis_insn_alloc_fixed(128, 32, 16, 32)

/* ---------------------------------------------------------------------- */
/* rb_path2class from variable.c reimplemented to NOT throw exceptions
 * when a class isn't found. */
static VALUE path2class(const char * path) {
	const char *pbeg, *p;
	ID id;
	VALUE c = rb_cObject;

	pbeg = p = path;

	if (path[0] == '#') {
		rb_raise(rb_eArgError, "can't retrieve anonymous class %s", 
			 path);
	}

	while (*p) {
		while (*p && *p != ':') p++;
		id = rb_intern2(pbeg, p-pbeg);
		if (p[0] == ':') {
			if (p[1] != ':') {
				rb_raise(rb_eArgError, 
					 "undefined class/module %.*s", 
					 (int)(p-path), path);
			}
			p += 2;
			pbeg = p;
		}

		if (!rb_const_defined(c, id)) {
			return Qnil;
		}

		c = rb_const_get_at(c, id);
		switch (TYPE(c)) {
			case T_MODULE:
			case T_CLASS:
			break;
			default:
			rb_raise(rb_eTypeError, 
				 "%s does not refer to class/module", path);
			}
		}

	return c;
}

/* ---------------------------------------------------------------------- */
/* Disasm Output Class */
/* This is basically a Hash of VMA => Instruction entries, with an @errors
 * attribute that gets filled with error messages from the disassembler */

/* insn containing vma */
static VALUE cls_output_contain( VALUE instance, VALUE vma ) {
	unsigned long long addr = NUM2ULL(vma);
	/* NOTE: '32 bytes is the largest insn' may not be valid */
	unsigned long long orig = addr, min = addr - 32;
	int cont = 1;

	/* iterate backwards from vma looking for insn containing vma */
	for ( cont = 1; cont > 0 && addr >= min; addr -= 1 ) {
		VALUE rb_size;
		VALUE val = rb_hash_lookup2( instance, ULL2NUM(addr), Qfalse ); 

		if ( val == Qfalse ) {
			if ( addr == 0 ) {
				cont = 0;
			}
			continue;
		}

		/* requested vma exists; return it */
		if ( addr == orig ) {
			return val;
		}

		/* does insn contain (insn.size + addr) requested vma? */
		rb_size = rb_funcall(val, rb_intern(IVAR(INSN_ATTR_SIZE)), 0);
		if ( rb_size == Qnil || addr + NUM2UINT(rb_size) < orig ) {
			/* nope - no insn contains requested vma */
			cont = 0;
		} else {
			/* yup - found it */
			return val;
		}
	}

	return Qfalse;
}

static VALUE cls_output_init( VALUE instance ) {
	rb_iv_set(instance, IVAR(OUT_ATTR_ERRORS), rb_ary_new() );
	return instance;
}

static void init_output_class( VALUE modOpdis ) {
	clsOutput = rb_define_class_under(modOpdis, OPDIS_OUTPUT_CLASS_NAME, 
					  rb_cHash);
	rb_define_method(clsOutput, "initialize", cls_output_init, 0);

	/* read-only attribute for error list */
	rb_define_attr(clsOutput, OUT_ATTR_ERRORS, 1, 0);

	/* setters */
	rb_define_method(clsOutput, OUT_METHOD_CONTAIN, cls_output_contain, 1);
}


/* ---------------------------------------------------------------------- */
/* Disassembler Architectures (internal) */
struct arch_def { 
	const char * name; 
	enum bfd_architecture arch;	/* BFD CPU architecture */
	unsigned long mach;		/* (default) BFD machine type */
	disassembler_ftype fn;		/* libopcodes print_insn callback */
};

/* ---------------------------------------------------------------------- */
/* Supported architectures */

static struct arch_def arch_definitions[] = {
	// TODO: arm, ia64, ppc, sparc, etc

	{"8086", bfd_arch_i386, bfd_mach_i386_i8086, print_insn_i386},
	{"x86", bfd_arch_i386, bfd_mach_i386_i386, print_insn_i386},
	{"x86_att", bfd_arch_i386, bfd_mach_i386_i386, print_insn_i386_att},
	{"x86_intel", bfd_arch_i386, bfd_mach_i386_i386_intel_syntax, 
		      print_insn_i386_intel},
	{"x86_64", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
	{"x86_64_att", bfd_arch_i386, bfd_mach_x86_64, print_insn_i386},
	{"x86_64_intel", bfd_arch_i386, bfd_mach_x86_64_intel_syntax, 
			         print_insn_i386}
};


/* ---------------------------------------------------------------------- */
/* Disassembler Class */

/* list all recognized syntaxes */
static VALUE cls_disasm_syntaxes( VALUE class ) {
	VALUE ary = rb_ary_new();
	rb_ary_push( ary, rb_str_new_cstr(DIS_SYNTAX_ATT) );
	rb_ary_push( ary, rb_str_new_cstr(DIS_SYNTAX_INTEL) );
	return ary;
}

/* list all recognized architectures */
static VALUE cls_disasm_architectures( VALUE class ) {
	VALUE ary = rb_ary_new();
	int i;
	int num_defs = sizeof(arch_definitions) / sizeof(struct arch_def);

	for ( i = 0; i < num_defs; i++ ) {
		struct arch_def *def = &arch_definitions[i];
		rb_ary_push( ary, rb_str_new_cstr(def->name) );
	}

	return ary;
}

/* list all recognized disassembly algorithms */
static VALUE cls_disasm_strategies( VALUE class ) {
	VALUE ary = rb_ary_new();
	rb_ary_push( ary, rb_str_new_cstr(DIS_STRAT_SINGLE) );
	rb_ary_push( ary, rb_str_new_cstr(DIS_STRAT_LINEAR) );
	rb_ary_push( ary, rb_str_new_cstr(DIS_STRAT_CFLOW) );
	rb_ary_push( ary, rb_str_new_cstr(DIS_STRAT_SYMBOL) );
	rb_ary_push( ary, rb_str_new_cstr(DIS_STRAT_SECTION) );
	rb_ary_push( ary, rb_str_new_cstr(DIS_STRAT_ENTRY) );
	return ary;
}

/* local decoder callback: this calls the decode method in the object provided
 * by the user. */
static int local_decoder( const opdis_insn_buf_t in, opdis_insn_t * out,
                          const opdis_byte_t * buf, opdis_off_t offset,
                          opdis_vma_t vma, opdis_off_t length, void * arg ) {
	/* Build a hash containing the arguments passed to the decoder */
	VALUE hash = Opdis_decoderHash( in, buf, offset, vma, length );
	/* Create a Ruby Opdis::Instruction object based on the C object */
	VALUE insn = Opdis_insnFromC(out);
	VALUE obj = (VALUE) arg;

	/* invoke decode method in Decoder object */
	VALUE var = rb_funcall(obj, symDecode, 2, insn, hash);

	/* Move info back to C domain */
	Opdis_insnToC( insn, out );

	return (Qfalse == var || Qnil == var) ? 0 : 1;
}

static VALUE cls_disasm_set_decoder(VALUE instance, VALUE obj) {
	opdis_t  opdis;
	Data_Get_Struct(instance, opdis_info_t, opdis);

	/* 'nil' causes opdis to revert to default decoder */
	if ( Qnil == obj ) {
		opdis_set_decoder( opdis, opdis_default_decoder, NULL );
		return Qtrue;
	}

	/* objects without a 'decode' method cannot be decoders */
	if (! rb_respond_to(obj, rb_intern(DECODER_METHOD)) ) {
		return Qfalse;
	}
	
	opdis_set_decoder( opdis, local_decoder, (void *) obj );
	rb_iv_set(instance, IVAR(DIS_ATTR_DECODER), obj );

	return Qtrue;
}

/* local insn handler object: this invokes the visited? method in the handler
 * object provided by the user. */
static int local_handler( const opdis_insn_t * i, void * arg ) {
	VALUE obj = (VALUE) arg;
	VALUE insn = Opdis_insnFromC(i);

	/* invoke visited? method in Handler object */
	VALUE var = rb_funcall(obj, symVisited, 1, insn);

	/* True means already visited, so continue = 0 */
	return (Qtrue == var) ? 0 : 1;
}

static VALUE cls_disasm_set_handler(VALUE instance, VALUE obj) {
	opdis_t  opdis;
	Data_Get_Struct(instance, opdis_info_t, opdis);

	/* nil causes opdis to revert to default decoder */
	if ( Qnil == obj ) {
		opdis_set_handler( opdis, opdis_default_handler, opdis );
		return Qtrue;
	}

	/* objects without a visited? method cannot be handlers */
	if (! rb_respond_to(obj, rb_intern(HANDLER_METHOD)) ) {
		return Qfalse;
	}

	opdis_set_handler( opdis, local_handler, (void *) obj );
	rb_iv_set(instance, IVAR(DIS_ATTR_HANDLER), obj );

	return Qtrue;
}

/* local resolver callback: this invokes the ruby resolve method in the object
 * provided by the user */
static opdis_vma_t local_resolver ( const opdis_insn_t * i, void * arg ) {
	VALUE obj = (VALUE) arg;
	VALUE insn = Opdis_insnFromC(i);

	/* invoke resolve method in Resolver object */
	VALUE vma = rb_funcall(obj, symResolve, 1, insn);

	return (Qnil == vma) ? OPDIS_INVALID_ADDR : (opdis_vma_t) NUM2UINT(vma);
}

static VALUE cls_disasm_set_resolver(VALUE instance, VALUE obj) {
	opdis_t  opdis;
	Data_Get_Struct(instance, opdis_info_t, opdis);

	/* nil causes opdis to revert to default resolver */
	if ( Qnil == obj ) {
		opdis_set_resolver( opdis, opdis_default_resolver, NULL );
		return Qtrue;
	}

	/* objects without a resolve method cannot be Resolvers */
	if (! rb_respond_to(obj, rb_intern(RESOLVER_METHOD)) ) {
		return Qfalse;
	}
	
	opdis_set_resolver( opdis, local_resolver, (void *) obj );
	rb_iv_set(instance, IVAR(DIS_ATTR_RESOLVER), obj );

	return Qtrue;
}

static VALUE cls_disasm_get_debug(VALUE instance) {
	opdis_t  opdis;
	Data_Get_Struct(instance, opdis_info_t, opdis);
	return opdis->debug ? Qtrue : Qfalse;
}

static VALUE cls_disasm_set_debug(VALUE instance, VALUE enabled) {
	opdis_t  opdis;
	Data_Get_Struct(instance, opdis_info_t, opdis);
	opdis->debug = (enabled == Qtrue) ? 1 : 0;
	return Qtrue;
}

static VALUE cls_disasm_get_syntax(VALUE instance) {
	opdis_t  opdis;
	VALUE str;

	Data_Get_Struct(instance, opdis_info_t, opdis);

	if ( opdis->disassembler == print_insn_i386_intel ) {
		str = rb_str_new_cstr(DIS_SYNTAX_INTEL);
	} else {
		str = rb_str_new_cstr(DIS_SYNTAX_ATT);
	}

	return str;
}

static VALUE cls_disasm_set_syntax(VALUE instance, VALUE syntax) {
	opdis_t  opdis;
	enum opdis_x86_syntax_t syn;
	VALUE syntax_s = rb_any_to_s(syntax);
	const char * str = StringValueCStr(syntax_s);

	if (! strcmp(str, DIS_SYNTAX_INTEL) ) {
		syn = opdis_x86_syntax_intel;
	} else if (! strcmp(str, DIS_SYNTAX_ATT) ) {
		syn = opdis_x86_syntax_att;
	} else {
		rb_raise(rb_eArgError, "Syntax must be 'intel' or 'att'");
	}

	Data_Get_Struct(instance, opdis_info_t, opdis);
	opdis_set_x86_syntax(opdis, syn);

	return Qtrue;
}

static VALUE cls_disasm_get_arch(VALUE instance) {
	opdis_t  opdis;
	int i;
	int num_defs = sizeof(arch_definitions) / sizeof(struct arch_def);

	Data_Get_Struct(instance, opdis_info_t, opdis);

	for ( i = 0; i < num_defs; i++ ) {
		struct arch_def *def = &arch_definitions[i];
		if ( def->arch == opdis->config.arch &&
		     def->mach == opdis->config.mach ) {
			return rb_str_new_cstr(def->name);
		}
	}

	return rb_str_new_cstr("unknown");
}

static VALUE cls_disasm_set_arch(VALUE instance, VALUE arch) {
	opdis_t  opdis;
	struct disassemble_info * info;
	int i;
	int num_defs = sizeof(arch_definitions) / sizeof(struct arch_def);
	const char * name = rb_string_value_cstr(&arch);

	Data_Get_Struct(instance, opdis_info_t, opdis);
	info = &opdis->config;

	info->application_data = arch_definitions[0].fn;
	info->arch = bfd_arch_unknown;
	info->mach = 0;

	for ( i = 0; i < num_defs; i++ ) {
		struct arch_def *def = &arch_definitions[i];
		if (! strcmp(name, def->name) ) {
			info->application_data = def->fn;
			info->arch = def->arch;
			info->mach = def->mach;
			rb_iv_set(instance, IVAR(DIS_ATTR_ARCH), arch);
			return Qtrue;
		}
	}

	return Qfalse;
}

static VALUE cls_disasm_get_opts(VALUE instance) {
	opdis_t  opdis;
	Data_Get_Struct(instance, opdis_info_t, opdis);
	return rb_str_new_cstr(opdis->config.disassembler_options);
}

static VALUE cls_disasm_set_opts(VALUE instance, VALUE opts) {
	char * str;
	opdis_t  opdis;
	VALUE opts_s = opts;
	Data_Get_Struct(instance, opdis_info_t, opdis);

	str = StringValueCStr(opts_s);
	opdis_set_disassembler_options( opdis, str );
	return Qtrue;
}

/* get opdis options from an argument hash */
static void cls_disasm_handle_args( VALUE instance, VALUE hash ) {
	VALUE var;

	/* opdis callbacks */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_RESOLVER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_resolver(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_HANDLER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_handler(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_DECODER), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_decoder(instance, var);

	/* opdis settings */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_SYNTAX), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_syntax(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_DEBUG), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_debug(instance, var);

	/* libopcodes settings */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_OPTIONS), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_opts(instance, var);

	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_ARCH), Qfalse);
	if ( Qfalse != var ) cls_disasm_set_arch(instance, var);
}

struct DISPLAY_ARGS { VALUE output; VALUE block; };

/* local display handler: this adds instructions to a Disassembly object
 * and invokes block if provided. */
static void local_display( const opdis_insn_t * i, void * arg ) {
	struct DISPLAY_ARGS * args = (struct DISPLAY_ARGS *) arg;
	VALUE insn = Opdis_insnFromC(i);

	if ( insn == Qnil ) {
		char buf[128];
		VALUE errors = rb_iv_get(args->output, IVAR(OUT_ATTR_ERRORS));
		snprintf( buf, 127-1, "%s: Unable to convert C insn to Ruby", 
			  DIS_ERR_DECODE );
		rb_ary_push( errors, rb_str_new_cstr(buf) );
		return;
	}

	if ( Qnil != args->block ) {
		rb_funcall(args->block, symCall, 1, insn);
	}

	rb_hash_aset( args->output, INT2NUM(i->vma), insn );

	rb_thread_schedule();
}

/* local error handler: this appends errors to a ruby array in arg */
static void local_error( enum opdis_error_t error, const char * msg,
                         void * arg ) {
	const char * type;
	char buf[128] = {0};
	VALUE errors = (VALUE) arg;

	switch (error) {
		case opdis_error_bounds: type = DIS_ERR_BOUNDS; break;
		case opdis_error_invalid_insn: type = DIS_ERR_INVALID; break;
		case opdis_error_decode_insn: type = DIS_ERR_DECODE; break;
		case opdis_error_bfd: type = DIS_ERR_BFD; break;
		case opdis_error_max_items: type = DIS_ERR_MAX; break;
		case opdis_error_unknown: 
		default: type = DIS_ERR_UNK; break;
	}

	snprintf( buf, 128-1, "%s: %s", type, msg );

	/* append error message to error list */
	rb_ary_push( errors, rb_str_new_cstr(buf) );
}

static void config_buf_from_args( opdis_buf_t buf, VALUE hash ) {
	VALUE var;

	/* buffer vma */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_BUFVMA), Qfalse);
	if ( Qfalse != var && Qnil != var ) buf->vma = NUM2UINT(var);

	// TODO: other options? 
}

static opdis_buf_t opdis_buf_for_target( VALUE tgt, VALUE hash ) {
	unsigned char * buf = NULL;
	unsigned int buf_len = 0;
	opdis_buf_t obuf;

	/* String object containing bytes */
	if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cString ) ) {
		buf = (unsigned char*) RSTRING_PTR(tgt);
		buf_len = RSTRING_LEN(tgt);

	/* Array object containing bytes */
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cArray ) ) {
		int i;
		unsigned char * sbuf;
		buf_len = RARRAY_LEN(tgt);
		sbuf = alloca(buf_len);
		for( i=0; i < buf_len; i++ ) {
			VALUE val = rb_ary_entry( tgt, i );
			sbuf[i] = (unsigned char) NUM2UINT(val);
		}

		buf = sbuf;

	/* IO object containing bytes */
	} else if ( Qtrue == rb_obj_is_kind_of( tgt, rb_cIO ) ) {
		VALUE str = rb_funcall( tgt, symRead, 0 );
		buf = (unsigned char*) RSTRING_PTR(str);
		buf_len = RSTRING_LEN(str);

	} else {
		rb_raise(rb_eArgError, "Buffer must be a String, IO or Array");
	}

	if (! buf || ! buf_len ) {
		rb_raise(rb_eArgError, "Cannot disassemble empty buffer");
	}

	obuf = opdis_buf_alloc( buf_len, 0 );
	opdis_buf_fill( obuf, 0, buf, buf_len );

	/* apply target-specific args (vma, etc) */
	config_buf_from_args( obuf, hash );

	return obuf;
}

struct OPDIS_TGT {bfd * abfd; asection * sec; asymbol * sym; opdis_buf_t buf;};

static void load_target( opdis_t opdis, VALUE tgt, VALUE hash, 
			 struct OPDIS_TGT * out ) {

	/* Ruby Bfd::Target object */
	if ( Qnil != GET_BFD_CLASS(clsBfdTgt, BFD_TGT_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdTgt ) ) {
		Data_Get_Struct(tgt, bfd, out->abfd );
	
	/* Ruby Bfd::Symbol object */
	} else if ( Qnil != GET_BFD_CLASS(clsBfdSym, BFD_SYM_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdSym ) ) {
		Data_Get_Struct(tgt, asymbol, out->sym );
		if ( out->sym ) {
			out->abfd = out->sym->the_bfd;
		}

	/* Ruby Bfd::Section object */
	} else if ( Qnil != GET_BFD_CLASS(clsBfdSec, BFD_SEC_PATH) &&
	     Qtrue == rb_obj_is_kind_of( tgt, clsBfdSec ) ) {
		Data_Get_Struct(tgt, asection, out->sec );
		if ( out->sec ) {
			out->abfd = out->sec->owner;
		}

	/* Other non-Bfd Ruby object */
	} else {
		out->buf = opdis_buf_for_target( tgt, hash );
	}

	/* Set arch, etc based on BFD info */
	if ( out->abfd ) {
		opdis_config_from_bfd( opdis, out->abfd );
	}
}

static void perform_disassembly( VALUE instance, opdis_t opdis, VALUE target,
				 VALUE hash ) {
	VALUE var;
	VALUE rb_vma = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_VMA), 
				       INT2NUM(0));
	VALUE rb_len = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_LEN), 
				       INT2NUM(0));
	opdis_vma_t vma = NUM2ULL(rb_vma);
	opdis_off_t len = NUM2UINT(rb_len);
	const char * strategy = DIS_STRAT_LINEAR;
	struct OPDIS_TGT tgt = {0};

	/* load target based on its Ruby object type */
	load_target( opdis, target, hash, &tgt );

	/* apply general args (syntax, arch, etc), overriding Bfd config */
	cls_disasm_handle_args(instance, hash);

	/* get disassembly algorithm to use */
	var = rb_hash_lookup2(hash, str_to_sym(DIS_ARG_STRATEGY), Qfalse);
	if ( Qfalse != var ) strategy = StringValueCStr(var);

	rb_thread_schedule();

	// TODO: INVESTIGATE why without this, ruby crashes!
	rb_gc_disable();

	/* Single instruction disassembly */
	if (! strcmp( strategy, DIS_STRAT_SINGLE ) ) {
		opdis_insn_t * insn = ALLOC_FIXED_INSN;

		if ( tgt.abfd ) {
			opdis_disasm_bfd_insn( opdis, tgt.abfd, vma, insn );
		} else {
			opdis_disasm_insn( opdis, tgt.buf, vma, insn );
		}

		/* invoke display function */
		opdis->display( insn, opdis->display_arg );

		opdis_insn_free(insn);

	/* Linear disassembly */
	} else if (! strcmp( strategy, DIS_STRAT_LINEAR ) ) {
		if ( tgt.abfd ) {
		 	opdis_disasm_bfd_linear( opdis, tgt.abfd, vma, len );
		} else {
		 	opdis_disasm_linear( opdis, tgt.buf, vma, len );
		}

	/* Control Flow disassembly */
	} else if (! strcmp( strategy, DIS_STRAT_CFLOW ) ) {
		if ( tgt.abfd ) {
			opdis_disasm_bfd_cflow( opdis, tgt.abfd, vma );
		} else {
			opdis_disasm_cflow( opdis, tgt.buf, vma );
		}

	/* Control Flow disassembly of BFD symbol */
	} else if (! strcmp( strategy, DIS_STRAT_SYMBOL ) ) {
		if (! tgt.sym ) {
			rb_raise(rb_eArgError, "Bfd::Symbol required");
		}
		opdis_disasm_bfd_symbol( opdis, tgt.sym );

	/* Linear disassembly of BFD section */
	} else if (! strcmp( strategy, DIS_STRAT_SECTION ) ) {
		if (! tgt.sec ) {
			rb_raise(rb_eArgError, "Bfd::Section required");
		}
		opdis_disasm_bfd_section( opdis, tgt.sec );

	/* Control Flow disassembly of BFD entry point */
	} else if (! strcmp( strategy, DIS_STRAT_ENTRY ) ) {
		if (! tgt.abfd ) {
			rb_raise(rb_eArgError, "Bfd::Target required");
		}
		opdis_disasm_bfd_entry( opdis, tgt.abfd );
	} else {
		if ( tgt.buf ) {
			opdis_buf_free(tgt.buf);
		}
		rb_raise(rb_eArgError, "Unknown strategy '%s'", strategy);
	}

	if ( tgt.buf ) {
		opdis_buf_free(tgt.buf);
	}

	// See above TODO
	rb_gc_enable();
}


/* Disassembler strategies produce blocks */
static VALUE cls_disasm_disassemble(VALUE instance, VALUE tgt, VALUE hash ) {
	opdis_t opdis, opdis_orig;
	VALUE args[1] = {Qnil};
	struct DISPLAY_ARGS display_args = { Qnil, Qnil };

	/* Create duplicate opdis_t in order to be threadsafe */
	Data_Get_Struct(instance, opdis_info_t, opdis_orig);
	opdis = opdis_dupe(opdis_orig);

	// TODO: block should also return Disassembly
	/* yielding to a block is easy */
	if ( rb_block_given_p() ) {
		display_args.block = rb_block_proc();
	}

	display_args.output = rb_class_new_instance( 0, args, clsOutput );

	opdis_set_display( opdis, local_display, &display_args );

	opdis_set_error_reporter( opdis, local_error, 
			(void *) rb_iv_get(display_args.output, 
					   IVAR(OUT_ATTR_ERRORS)) );

	perform_disassembly( instance, opdis, tgt, hash );

	opdis_term(opdis);

	return display_args.output;
}

/* new: takes hash of arguments */
static VALUE cls_disasm_new(VALUE class, VALUE hash) {
	VALUE instance;
	VALUE argv[1] = { Qnil };
	opdis_t opdis = opdis_init();

	instance = Data_Wrap_Struct(class, NULL, opdis_term, opdis);
	rb_obj_call_init(instance, 0, argv);

	cls_disasm_handle_args(instance, hash);

	return instance;
}

/* usage: writes a list of disassembler options to IO object */
static VALUE cls_disasm_usage(VALUE class, VALUE io) {
	const char * filename;
	VALUE path;
	FILE * f;

	if (! rb_respond_to(io, symPath) ) {
		rb_raise( rb_eArgError, "Argument must respond to :path" );
	}

	/* get path from ruby File object */
	path = rb_funcall(io, symPath, 0);
	if ( Qnil == path ) {
		rb_raise( rb_eArgError, "Object#path returned nil" );
	}

	filename = StringValueCStr(path);
	f = fopen( filename, "w");
	if (! f ) {
		rb_raise( rb_eRuntimeError, 
			  "Could not open %s object for write: %s", 
			  filename, strerror(errno) );
	}

	/* invoke libopcodes to write usage into to FILE * */
	disassembler_usage(f);

	fclose(f);

	return Qtrue;
}

static void define_disasm_constants() {
	/* Error types */
	rb_define_const(clsDisasm, DIS_ERR_BOUNDS_NAME,
			rb_str_new_cstr(DIS_ERR_BOUNDS));
	rb_define_const(clsDisasm, DIS_ERR_INVALID_NAME,
			rb_str_new_cstr(DIS_ERR_INVALID));
	rb_define_const(clsDisasm, DIS_ERR_DECODE_NAME,
			rb_str_new_cstr(DIS_ERR_DECODE));
	rb_define_const(clsDisasm, DIS_ERR_BFD_NAME,
			rb_str_new_cstr(DIS_ERR_BFD));
	rb_define_const(clsDisasm, DIS_ERR_MAX_NAME,
			rb_str_new_cstr(DIS_ERR_MAX));

	/* Disassembly algorithms */
	rb_define_const(clsDisasm, DIS_STRAT_SINGLE_NAME,
			rb_str_new_cstr(DIS_STRAT_SINGLE));
	rb_define_const(clsDisasm, DIS_STRAT_LINEAR_NAME,
			rb_str_new_cstr(DIS_STRAT_LINEAR));
	rb_define_const(clsDisasm, DIS_STRAT_CFLOW_NAME,
			rb_str_new_cstr(DIS_STRAT_CFLOW));
	rb_define_const(clsDisasm, DIS_STRAT_SYMBOL_NAME,
			rb_str_new_cstr(DIS_STRAT_SYMBOL));
	rb_define_const(clsDisasm, DIS_STRAT_SECTION_NAME,
			rb_str_new_cstr(DIS_STRAT_SECTION));
	rb_define_const(clsDisasm, DIS_STRAT_ENTRY_NAME,
			rb_str_new_cstr(DIS_STRAT_ENTRY));

	/* Lists of symbolic constants */
	rb_define_singleton_method(clsDisasm, DIS_CONST_STRATEGIES, 
				   cls_disasm_strategies, 0);
	rb_define_singleton_method(clsDisasm, DIS_CONST_ARCHES, 
				   cls_disasm_architectures, 0);
	rb_define_singleton_method(clsDisasm, DIS_CONST_SYNTAXES, 
				   cls_disasm_syntaxes, 0);
}

static void init_disasm_class( VALUE modOpdis ) {
	clsDisasm = rb_define_class_under(modOpdis, OPDIS_DISASM_CLASS_NAME, 
					  rb_cObject);
	rb_define_singleton_method(clsDisasm, "ext_new", cls_disasm_new, 1);
	rb_define_singleton_method(clsDisasm, "ext_usage", cls_disasm_usage, 1);

	/* read-only attributes */
	rb_define_attr(clsDisasm, DIS_ATTR_DECODER, 1, 0);
	rb_define_attr(clsDisasm, DIS_ATTR_HANDLER, 1, 0);
	rb_define_attr(clsDisasm, DIS_ATTR_RESOLVER, 1, 0);

	/* setters */
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_DECODER), 
			 cls_disasm_set_decoder, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_HANDLER), 
			 cls_disasm_set_handler, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_RESOLVER), 
			 cls_disasm_set_resolver, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_DEBUG), 
			 cls_disasm_set_debug, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_SYNTAX), 
			 cls_disasm_set_syntax, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_ARCH), 
			 cls_disasm_set_arch, 1);
	rb_define_method(clsDisasm, SETTER(DIS_ATTR_OPTS), 
			 cls_disasm_set_opts, 1);

	/* getters */
	rb_define_method(clsDisasm, DIS_ATTR_DEBUG, cls_disasm_get_debug, 0);
	rb_define_method(clsDisasm, DIS_ATTR_SYNTAX, cls_disasm_get_syntax, 0);
	rb_define_method(clsDisasm, DIS_ATTR_ARCH, cls_disasm_get_arch, 0);
	rb_define_method(clsDisasm, DIS_ATTR_OPTS, cls_disasm_get_opts, 0);

	/* methods */
	rb_define_method(clsDisasm, DIS_METHOD_DISASM, cls_disasm_disassemble, 
			 2);

	define_disasm_constants();
}

/* ---------------------------------------------------------------------- */
/* Opdis Module */

void Init_OpdisExt() {
	symToSym = rb_intern("to_sym");
	symCall = rb_intern("call");
	symRead = rb_intern("read");
	symSize = rb_intern("size");
	symPath = rb_intern("path");

	symDecode = rb_intern(DECODER_METHOD);
	symVisited = rb_intern(HANDLER_METHOD);
	symResolve = rb_intern(RESOLVER_METHOD);

	modOpdis = rb_define_module(OPDIS_MODULE_NAME);

	init_disasm_class(modOpdis);
	init_output_class(modOpdis);

	Opdis_initCallbacks(modOpdis);

	Opdis_initModel(modOpdis);
}
