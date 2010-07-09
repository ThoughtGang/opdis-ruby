/* Callbacks.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_RB_CALLBACKS_H
#define OPDIS_RB_CALLBACKS_H

/* Resolver */

#define RESOLVER_METHOD "resolve"

/* Handler */

#define HANDLER_METHOD "visited?"

/* Decoder */

#define DECODER_METHOD "decode"

/* info provided to decoder */
#define DECODER_MEMBER_VMA "vma"
#define DECODER_MEMBER_OFF "offset"
#define DECODER_MEMBER_LEN "size"
#define DECODER_MEMBER_BUF "buffer"
#define DECODER_MEMBER_ITEMS "items"
#define DECODER_MEMBER_STR "raw_insn"
#define DECODER_MEMBER_DELAY "branch_delay"
#define DECODER_MEMBER_DATA "data_size"
#define DECODER_MEMBER_TYPE "type"
#define DECODER_MEMBER_TGT "target"
#define DECODER_MEMBER_TGT2 "target2"

/* Genererate a hash suitable for passing to a Ruby Decoder method */
VALUE Opdis_decoderHash( const opdis_insn_buf_t in, 
                         const opdis_byte_t * buf, opdis_off_t offset,
                         opdis_vma_t vma, opdis_off_t length );

#endif
