/* Calbacks.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <ruby.h>
#include "ruby_compat.h"

#include <opdis/opdis.h>
#include <opdis/model.h>
#include <opdis/x86_decoder.h>

#include "Callbacks.h"
#include "Opdis.h"
#include "Model.h"

#define IVAR(attr) "@" attr
#define SETTER(attr) attr "="

static VALUE symToSym, symDecode, symVisited, symResolve;
static VALUE clsDecoder, clsX86Decoder, clsX86IntelDecoder;
static VALUE clsHandler, clsResolver;

static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, symToSym, 0);
}

#define ALLOC_FIXED_INSN opdis_insn_alloc_fixed(128, 32, 16, 32)

/* ---------------------------------------------------------------------- */
/* Decoder Class */

static VALUE insn_type_to_str( enum dis_insn_type t ) {
	const char *s = "Unknown";
	switch (t) {
		case dis_noninsn: s = "Invalid"; break;
		case dis_nonbranch: s = "Not branch"; break;
		case dis_branch: s = "Unconditional branch"; break;
		case dis_condbranch: s = "Conditional branch"; break;
		case dis_jsr: s = "Jump to subroutine"; break;
		case dis_condjsr: s = "Conditional jump to subroutine"; break;
		case dis_dref: s = "Data reference"; break;
		case dis_dref2: s = "Two data references"; break;
	}
	return rb_str_new_cstr(s);
}

static void fill_decoder_hash( VALUE hash, const opdis_insn_buf_t in, 
                               const opdis_byte_t * buf, opdis_off_t offset,
                               opdis_vma_t vma, opdis_off_t length ) {
	int i;
	VALUE str, ary;
	char info = in->insn_info_valid;

	/* instruction location and size */
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_VMA), INT2NUM(vma) );
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_OFF), INT2NUM(offset) );
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_LEN), INT2NUM(length) );

	/* target buffer */
	str = rb_str_new( (const char *) buf, offset + length );
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_BUF), str );
	
	/* decode instruction as provided by libopcodes */

	/* 1. get items from opdis_insn_buf_t */
	ary = rb_ary_new();
	for ( i = 0; i < in->item_count; i++ ) {
		rb_ary_push( ary, rb_str_new_cstr(in->items[i]) );
	}
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_ITEMS), ary );

	/* 2. get raw ASCII version of insn from libopcodes */
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_STR), 
		      rb_str_new_cstr(in->string) );

	/* 3. get instruction metadata set by libopcodes */
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_DELAY), 
		      info ? INT2NUM(in->branch_delay_insns) : Qnil);
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_DATA), 
		      info ? INT2NUM(in->data_size) : Qnil);
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_TYPE), 
		      info ? insn_type_to_str(in->insn_type) : Qnil);
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_TGT), 
		      info ? INT2NUM(in->target) : Qnil);
	rb_hash_aset( hash, str_to_sym(DECODER_MEMBER_TGT2), 
		      info ? INT2NUM(in->target2) : Qnil );
}

static int invoke_builtin_decoder( OPDIS_DECODER fn, VALUE insn, VALUE hash ) {
	int rv;
	VALUE var;
	opdis_insn_buf_t inbuf;
        opdis_byte_t * buf;
	opdis_off_t offset;
        opdis_vma_t vma;
	opdis_off_t length;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN;

	Opdis_insnToC( insn, c_insn );

	/* get insn_buf_t from decoder instance; this saves some trouble */
	Data_Get_Struct( hash, opdis_insn_buffer_t, inbuf );
	if (! inbuf ) {
		/* something went wrong: we weren't called from local_decoder */
		rb_raise( rb_eRuntimeError, "opdis_insn_buf_t not found" );
	}

	/* convert decoder arguments to C */
	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_VMA), Qfalse);
	vma = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_OFF), Qfalse);
	offset = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_LEN), Qfalse);
	length = ( Qfalse != var ) ? NUM2UINT(var) : 0;

	var = rb_hash_lookup2(hash, str_to_sym(DECODER_MEMBER_BUF), Qfalse);
	buf = ( Qfalse != var ) ? (opdis_byte_t *) RSTRING_PTR(var) : NULL;

	if ( buf == NULL ) {
		rb_raise( rb_eRuntimeError, "DecodeHash.buf is NULL" );
	}

	/* invoke C decoder callback */
	// TODO: pass something meaningful in arg
	rv = fn( inbuf, c_insn, buf, offset, vma, length, NULL );
	if ( rv ) {
		// TODO : error handler?
		Opdis_insnFillFromC( c_insn, insn );
	}

	opdis_insn_free(c_insn);

	return rv ? Qtrue : Qfalse;
}

static VALUE cls_decoder_decode( VALUE instance, VALUE insn, VALUE hash ) {
	rb_thread_schedule();
	invoke_builtin_decoder(opdis_default_decoder, insn, hash);
	return insn;
}

static void init_decoder_class( VALUE modOpdis ) {
	clsDecoder = rb_define_class_under(modOpdis, "InstructionDecoder", 
					   rb_cObject);
	rb_define_method(clsDecoder, DECODER_METHOD, cls_decoder_decode, 2);
}

/*      ----------------------------------------------------------------- */
/* 	X86 Decoder Class */

static VALUE cls_x86decoder_decode( VALUE instance, VALUE insn, VALUE hash ) {
	rb_thread_schedule();
	invoke_builtin_decoder(opdis_x86_att_decoder, insn, hash);
	return insn;
}

static VALUE cls_x86inteldecoder_decode(VALUE instance, VALUE insn, VALUE hash){
	invoke_builtin_decoder(opdis_x86_intel_decoder, insn, hash);
	return insn;
}

static void init_x86decoder_class( VALUE modOpdis ) {
	/* AT&T Decoder */
	clsX86Decoder = rb_define_class_under(modOpdis, "X86Decoder", 
					      clsDecoder);
	rb_define_method(clsX86Decoder, DECODER_METHOD, 
			 cls_x86decoder_decode, 2);

	/* Intel Decoder */
	clsX86IntelDecoder = rb_define_class_under(modOpdis, "X86IntelDecoder", 
					           clsDecoder);
	rb_define_method(clsX86IntelDecoder, DECODER_METHOD, 
			 cls_x86inteldecoder_decode, 2);
}

/* ---------------------------------------------------------------------- */
/* VisitedAddr Class */

static VALUE cls_handler_visited( VALUE instance, VALUE insn ) {
	int rv;
	opdis_t opdis;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN;

	Opdis_insnToC( insn, c_insn );

	Data_Get_Struct(instance, opdis_info_t, opdis);
	if (! opdis ) {
		rb_raise(rb_eRuntimeError, "opdis_t not found in Handler");
	}

	rb_thread_schedule();
	rv = opdis_default_handler(c_insn, opdis);

	return rv ? Qtrue : Qfalse;
}

/* NOTE: this uses its own opdis_t with a visited_addr tree */
static VALUE cls_handler_new( VALUE class ) {
	VALUE argv[1] = {Qnil};
	opdis_t opdis = opdis_init();
	VALUE instance = Data_Wrap_Struct(class, NULL, opdis_term, opdis);
	rb_obj_call_init(instance, 0, argv);

	opdis->visited_addr = opdis_vma_tree_init();

	return instance;
}

static void init_handler_class( VALUE modOpdis ) {

	clsHandler = rb_define_class_under(modOpdis, "VisitedAddressTracker", 
					   rb_cObject);
	rb_define_singleton_method(clsHandler, "new", cls_handler_new, 0);
	rb_define_method(clsHandler, HANDLER_METHOD, cls_handler_visited, 1);
}

/* ---------------------------------------------------------------------- */
/* Resolver Class */

static VALUE cls_resolver_resolve( VALUE instance, VALUE insn ) {
	int rv;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN;

	Opdis_insnToC( insn, c_insn );

	rb_thread_schedule();
	rv = opdis_default_resolver( c_insn, NULL );

	return INT2NUM(rv);
}

static void init_resolver_class( VALUE modOpdis ) {
	clsResolver = rb_define_class_under(modOpdis, "AddressResolver", 
					    rb_cObject);
	rb_define_method(clsResolver, RESOLVER_METHOD, cls_resolver_resolve, 1);
}

void Opdis_initCallbacks( VALUE modOpdis ) {
	symToSym = rb_intern("to_sym");

	symDecode = rb_intern(DECODER_METHOD);
	symVisited = rb_intern(HANDLER_METHOD);
	symResolve = rb_intern(RESOLVER_METHOD); 

	init_resolver_class(modOpdis);
	init_handler_class(modOpdis);
	init_decoder_class(modOpdis);
	init_x86decoder_class(modOpdis);
}

VALUE Opdis_decoderHash( const opdis_insn_buf_t in, 
                         const opdis_byte_t * buf, opdis_off_t offset,
                         opdis_vma_t vma, opdis_off_t length ) {
	/* here we cheat and store insn_buf in hash in case one of the local
	 * decoder base classes gets called */
	VALUE hash = Data_Wrap_Struct(rb_cHash, NULL, NULL, in);
	fill_decoder_hash( hash, in, buf, offset, vma, length );
	return hash;
}

