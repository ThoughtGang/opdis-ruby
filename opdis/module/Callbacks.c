/* Calbacks.c
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <ruby.h>

#include <opdis.h>

#include "Opdis.h"
#include "Model.h"


#define IVAR(attr) "@" attr
#define SETTER(attr) attr "="

static VALUE symDecode, symVisited, symResolve;

static VALUE str_to_sym( const char * str ) {
	VALUE var = rb_str_new_cstr(str);
	return rb_funcall(var, symToSym, ));
}

/* BFD Support (requires BFD gem) */
static VALUE clsBfd = Qnil;
static VALUE clsBfdSec = Qnil;
static VALUE clsBfdSym = Qnil;
#define GET_BFD_CLASS(cls,name) (cls = cls == Qnil ? rb_path2class(name) : cls);

#define ALLOC_FIXED_INSN opdis_alloc_fixed(128, 32, 16, 32)

/* ---------------------------------------------------------------------- */
/* Decoder Class */

static VALUE clsDecoder;

static void fill_decoder_hash( VALUE * hash, const opdis_insn_buf_t in, 
                               const opdis_byte_t * buf, opdis_off_t offset,
                               opdis_vma_t vma, opdis_off_t length ) {
	int i;
	VALUE ary;
	char info = in->insn_info_valid;

	/* instruction location and size */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_VMA), INT2NUM(vma) );
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_OFF), INT2NUM(offset) );
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_LEN), INT2NUM(length) );

	/* target buffer */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_BUF), 
		      rb_str_new( (const char *) buf, offset + length ) );
	
	/* decode instruction as provided by libopcodes */

	/* 1. get items from opdis_insn_buf_t */
	ary = rb_ary_new();
	for ( i = 0; i < in->item_count; i++ ) {
		rb_ary_push( ary, rb_str_new_cstr(in->items[i]) );
	}
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_ITEMS), ary );

	/* 2. get raw ASCII version of insn from libopcodes */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_STR), 
		      rb_str_new_cstr(in->string) );

	/* 3. get instruction metadata set by libopcodes */
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_DELAY), 
		      info ? INT2NUM(in->branch_delay_insns) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_DATA), 
		      info ? INT2NUM(in->data_size) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_TYPE), 
		      info ? INT2NUM((int) in->insn_type) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_TGT), 
		      info ? INT2NUM(in->target) : Qnil);
	rb_hash_aset( *hash, str_to_sym(DECODE_MEMBER_TGT2), 
		      info ? INT2NUM(in->target2) : Qnil );

	/* here we cheat and store insn_buf in hash in case one of the local
	 * decoder base classes gets called */
	instance = Data_Wrap_Struct(*hash, NULL, NULL, in);
}

static int invoke_builtin_decoder( OPDIS_DECODER fn, VALUE insn, VALUE hash ) {
	opdis_insn_buf_t inbuf;
        opdis_byte_t * buf;
	opdis_off_t offset;
        opdis_vma_t vma;
	opdis_off_t length;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN();

	insn_to_c( insn, c_insn );

	/* get insn_buf_t from decoder instance; this saves some trouble */
	Data_Get_Struct( hash, opdis_insn_buf_t, inbuf );
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
	buf = ( Qfalse != var ) ? RSTRING_PTR(buf) : Qnil;

	/* invoke C decoder callback */
	rv = fn( inbuf, c_insn, buf, offset, vma, length );
	if ( rv ) {
		fill_ruby_insn( c_insn, insn );
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

static VALUE clsX86Decoder, clsX86IntelDecoder;

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

/* Handler class determines whether to continue ? */
static VALUE clsHandler;

static VALUE cls_handler_visited( VALUE instance, VALUE insn ) {
	int rv;
	opdist_t opdis;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN();

	insn_to_c( insn, c_insn );

	Data_Get_Struct(instance, opdis_t, opdis);
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
	opdist_t opdis = opdis_init();
	instance = Data_Wrap_Struct(class, NULL, opdis_term, opdis);
	rb_obj_call_init(instance, 0, argv);

	opdis->visited_addr = opdis_vma_tree_init();

	return init;
}

static void init_handler_class( VALUE modOpdis ) {

	clsHandler = rb_define_class_under(modOpdis, "VisitedAddressTracker", 
					   rb_cObject);
	rb_define_singleton_method(clsHandler, "new", cls_handler_new, 0);
	rb_define_method(clsHandler, HANDLER_METHOD, cls_handler_visited, 1);
}

/* ---------------------------------------------------------------------- */
/* Resolver Class */

static VALUE clsResolver;

static VALUE cls_resolver_resolve( VALUE instance, VALUE insn ) {
	int rv;
	opdis_insn_t * c_insn = ALLOC_FIXED_INSN();

	insn_to_c( insn, c_insn );

	rb_thread_schedule();
	rv = opdis_default_resolver( c_insn, NULL );

	return INT2NUM(rv);
}

static void init_resolver_class( VALUE modOpdis ) {
	cls = rb_define_class_under(modOpdis, "AddressResolver", rb_cObject);
	rb_define_method(clsResolver, RESOLVER_METHOD, cls_resolver_resolve, 1);
}

void Init_Opdis_initCallbacks( VALUE modOpdis ) {
	symDecode = rb_intern(DECODER_METHOD);
	symVisited = rb_intern(HANDLER_METHOD);
	symResolve = rb_intern(RESOLVER_METHOD); 

	init_resolver_class(modOpdis);
	init_handler_class(modOpdis);
	init_decoder_class(modOpdis);
}

