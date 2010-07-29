#!/usr/bin/env ruby1.9
# :title: Opdis
=begin rdoc
=Opdis
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

= Opdis Disassembler

== Summary

Requires GNU binutils. BFD gem may be required.

== Example

  require 'BFD'

  t = Bfd::Target.new( filename ) # Load target using BFD gem

  require 'Opdis'

  o = Opdis.Disassembler.new() # Create disassembler for target

  # Control-flow disassembly from BFD entry point
  o.disassemble( t, strategy: o.STRATEGY_ENTRY ).each do |i|
     # ... do something with instruction
  end

  # Control-flow disassembly from offset 0 in target
  insns = o.disassemble( t, strategy: o.STRATEGY_CFLOW, start: 0 )
  insn.each { |i| puts i }

  insns = o.disassemble( t, start: 0, len : 100 )
  insn.each { |i| puts i }

== Contact
Support:: community@thoughtgang.org
Project:: http://rubyforge.org/projects/opdis/
=end

module Opdis

=begin rdoc
=end
  class Disassembler

=begin rdoc
=end
    attr_accessor :insn_decoder

=begin rdoc
=end
    attr_accessor :addr_tracker

=begin rdoc
=end
    attr_accessor :resolver

=begin rdoc
=end
    attr_accessor :debug

=begin rdoc
=end
    attr_accessor :syntax

=begin rdoc
=end
    attr_accessor :arch

=begin rdoc
=end
    attr_accessor :opcodes_options

=begin rdoc
=end
    ERROR_BOUNDS='Bounds exceeded'
    ERROR_INVALID_INSN='Invalid instruction'
    ERROR_DECODE_INSN='Decoder error'
    ERROR_BFD='Bfd error'
    ERROR_MAX_ITEMS='Max insn items error'
    ERROR_UNK='Unknown error'

=begin rdoc
=end
    STRATEGY_SINGLE='single'
=begin rdoc
=end
    STRATEGY_LINEAR='linear'
=begin rdoc
=end
    STRATEGY_CFLOW='cflow'
=begin rdoc
=end
    STRATEGY_SYMBOL='bfd-symbol'
=begin rdoc
=end
    STRATEGY_SECTION='bfd-section'
=begin rdoc
=end
    STRATEGY_ENTRY='bfd-entry'

=begin rdoc
=end
    STRATEGIES = [ STRATEGY_SINGLE, STRATEGY_LINEAR, STRATEGY_CFLOW,
                   STRATEGY_SYMBOL, STRATEGY_SECTION, STRATEGY_ENTRY ]

=begin rdoc
=end
    SYNTAX_ATT='att'
=begin rdoc
=end
    SYNTAX_INTEL='intel'

=begin rdoc
=end
    SYNTAXES = [ SYNTAX_ATT, SYNTAX_INTEL ]

=begin rdoc
=end
    def ext_disassemble(target, args)
    end

=begin rdoc
=end
    def initialize( args )
    end

=begin rdoc
=end
    def architectures()
    end
  end

=begin rdoc
=end
    def ext_usage()
    end

=begin rdoc
=end
  class Disassembly < Hash
    attr_reader :errors

=begin rdoc
=end
    def containing(vma)
    end
  end

end
