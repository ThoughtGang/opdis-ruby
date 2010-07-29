/* Opdis.h
 * Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_RB_OPDIS_H
#define OPDIS_RB_OPDIS_H

/* method names */
#define DIS_METHOD_DISASM "ext_disassemble"
#define DIS_METHOD_usage "ext_usage"

/* attribute names */
#define DIS_ATTR_DECODER "insn_decoder"
#define DIS_ATTR_HANDLER "addr_tracker"
#define DIS_ATTR_RESOLVER "resolver"
#define DIS_ATTR_DEBUG "debug"
#define DIS_ATTR_SYNTAX "syntax"
#define DIS_ATTR_ARCH "arch"
#define DIS_ATTR_OPTS "opcodes_options"

/* argument (hash) names */
#define DIS_ARG_DECODER DIS_ATTR_DECODER 
#define DIS_ARG_HANDLER DIS_ATTR_HANDLER
#define DIS_ARG_RESOLVER DIS_ATTR_RESOLVER
#define DIS_ARG_SYNTAX "syntax"
#define DIS_ARG_DEBUG "debug"
#define DIS_ARG_OPTIONS "options"
#define DIS_ARG_STRATEGY "strategy"
#define DIS_ARG_ARCH "arch"
#define DIS_ARG_VMA "vma"
#define DIS_ARG_OFFSET "offset"
#define DIS_ARG_LEN "length"
#define DIS_ARG_BUFVMA "buffer_vma"

/* constants */
#define DIS_ERR_BOUNDS_NAME "ERROR_BOUNDS"
#define DIS_ERR_BOUNDS "Bounds exceeded"
#define DIS_ERR_INVALID_NAME "ERROR_INVALID_INSN"
#define DIS_ERR_INVALID "Invalid instruction"
#define DIS_ERR_DECODE_NAME "ERROR_DECODE_INSN"
#define DIS_ERR_DECODE "Decoder error"
#define DIS_ERR_BFD_NAME "ERROR_BFD"
#define DIS_ERR_BFD "Bfd error"
#define DIS_ERR_MAX_NAME "ERROR_MAX_ITEMS"
#define DIS_ERR_MAX "Max insn items error"
#define DIS_ERR_UNK "Unknown error"

#define DIS_STRAT_SINGLE_NAME "STRATEGY_SINGLE"
#define DIS_STRAT_SINGLE "single-instruction"
#define DIS_STRAT_LINEAR_NAME "STRATEGY_LINEAR"
#define DIS_STRAT_LINEAR "linear"
#define DIS_STRAT_CFLOW_NAME "STRATEGY_CFLOW"
#define DIS_STRAT_CFLOW "control-flow"
#define DIS_STRAT_SYMBOL_NAME "STRATEGY_SYMBOL"
#define DIS_STRAT_SYMBOL "bfd-symbol"
#define DIS_STRAT_SECTION_NAME "STRATEGY_SECTION"
#define DIS_STRAT_SECTION "bfd-section"
#define DIS_STRAT_ENTRY_NAME "STRATEGY_ENTRY"
#define DIS_STRAT_ENTRY "bfd-entry"

#define DIS_CONST_STRATEGIES "STRATEGIES"

#define DIS_CONST_ARCHES "architectures"

#define DIS_SYNTAX_ATT "att"
#define DIS_SYNTAX_INTEL "intel"

#define DIS_CONST_SYNTAXES "SYNTAXES"

/* Output */

#define OUT_ATTR_ERRORS "errors"
#define OUT_METHOD_CONTAIN "containing"

/* BFD */
#define BFD_TGT_PATH "Bfd::Target"
#define BFD_SEC_PATH "Bfd::Section"
#define BFD_SYM_PATH "Bfd::Symbol"

#define OPDIS_MODULE_NAME "Opdis"
#define OPDIS_DISASM_CLASS_NAME "Disassembler"
#define OPDIS_OUTPUT_CLASS_NAME "Disassembly"

void Init_OpdisExt();

#endif
