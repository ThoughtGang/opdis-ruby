/* Bfd.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef BFD_RUBY_EXTENSION_H
#define BFD_RUBY_EXTENSION_H

/* Bfd::Symbol */
#define SYM_ATTR_NAME "name"
#define SYM_ATTR_TYPE "type"
#define SYM_ATTR_VALUE "value"
#define SYM_ATTR_FLAGS "raw_flags"
#define SYM_ATTR_SECTION "section"
#define SYM_ATTR_BIND "binding"

#define SYM_BIND_DYNAMIC "dynamic"
#define SYM_BIND_DYN_NAME "DYNAMIC"
#define SYM_BIND_STATIC "static"
#define SYM_BIND_STAT_NAME "STATIC"

/* Bfd::Section */
#define SEC_ATTR_ID "id"
#define SEC_ATTR_NAME "name"
#define SEC_ATTR_INDEX "index"
#define SEC_ATTR_FLAGS "raw_flags"
#define SEC_ATTR_VMA "vma"
#define SEC_ATTR_LMA "lma"
#define SEC_ATTR_SIZE "size"
#define SEC_ATTR_ALIGN "alignment_power"
#define SEC_ATTR_FPOS "file_pos"
#define SEC_ATTR_CONTENTS "contents"
#define SEC_ATTR_SYM "symbol"

/* Bfd::Target */
#define TGT_ATTR_ID "id"
#define TGT_ATTR_FILENAME "filename"
#define TGT_ATTR_FORMAT "format"
#define TGT_ATTR_FMT_FLAGS "raw_format_flags"
#define TGT_ATTR_START_ADDR "start_address"
#define TGT_ATTR_ARCH_INFO "arch_info"
#define TGT_ATTR_FLAVOUR "raw_flavour"
#define TGT_ATTR_TYPE "type"
#define TGT_ATTR_TYPEFLAGS "raw_type_flags"
#define TGT_ATTR_ENDIAN "raw_endian"
#define TGT_ATTR_SECTIONS "sections"
#define TGT_ATTR_SYMBOLS "symbols"

#define TGT_METHOD_SECVMA "section_for_vma"

/* arch_info members */
#define AINFO_MEMBER_BPW "bits_per_word"
#define AINFO_MEMBER_BPA "bits_per_address"
#define AINFO_MEMBER_BPB "bits_per_byte"
#define AINFO_MEMBER_ARCH "architecture"
#define AINFO_MEMBER_ALIGN "section_align_power"

/* module and class names */
#define BFD_MODULE_NAME "Bfd"
#define TARGET_CLASS_NAME "Target"
#define BUFFER_TGT_CLASS_NAME "BufferTarget"
#define SECTION_CLASS_NAME "Section"
#define SYMBOL_CLASS_NAME "Symbol"

void Init_BFDext();

#endif
