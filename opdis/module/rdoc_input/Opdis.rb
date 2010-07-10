#!/usr/bin/env ruby1.9
# :title: Opdis::Callbacks
=begin rdoc
=Opdis Callbacks
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

descr
=end

module Opdis

  class Disassembler
    attr_accessor :insn_decoder, :addr_tracker, :resolver
    attr_accessor :debug, :syntax, :arch, :opcodes_options

    ERROR_BOUNDS='Bounds exceeded'
    ERROR_INVALID_INSN='Invalid instruction'
    ERROR_DECODE_INSN='Decoder error'
    ERROR_BFD='Bfd error'
    ERROR_MAX_ITEMS='Max insn items error'
    ERROR_UNK='Unknown error'

    STRATEGY_SINGLE='single'
    STRATEGY_LINEAR='linear'
    STRATEGY_CFLOW='cflow'
    STRATEGY_SYMBOL='bfd-symbol'
    STRATEGY_SECTION='bfd-section'
    STRATEGY_ENTRY='bfd-entry'

    STRATEGIES = [ STRATEGY_SINGLE, STRATEGY_LINEAR, STRATEGY_CFLOW,
                   STRATEGY_SYMBOL, STRATEGY_SECTION, STRATEGY_ENTRY ]

    SYNTAX_ATT='att'
    SYNTAX_INTEL='intel'

    SYNTAXES = [ SYNTAX_ATT, SYNTAX_INTEL ]

    def disassemble(target, args)
    end

    alias :disasm, :disassemble

    def architectures
    end
  end

  class Disassembly < Hash
    attr_reader :errors

    def containing(vma)
    end
  end

end
