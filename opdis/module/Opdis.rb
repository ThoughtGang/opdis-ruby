#!/usr/bin/env/ruby1.0
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to the Opdis module

require 'BFD'
require 'OpdisExt'
require 'tempfile'    # for Disassembler#options

module Opdis

  class Disassembler

=begin rdoc

The args parameter is a Hash which can contain any of the following members:

  resolver:: AddressResolver object to use instead of the builtin
             address resolver.

  addr_tracker:: VisitedAddressTracker object to use instead of the built-in 
                 address tracker.

  insn_decoder:: InstructionDecoder object to use instead of the built-in
                 instruction decoder.

  syntax:: Assembly language syntax to use; either SYNTAX_ATT (default) or
           SYNTAX_INTEL.

  debug:: Enable or disable libopdis debug mode.

  options:: Options command-line string for libopcodes. See disassembler_usage()
            in dis-asm.h for details.

  arch:: 
=end

  def self.new(args={})
    dis = ext_new(args)
    yield dis if (dis and block_given?)
    return dis
  end

=begin rdoc
Disassemble all bytes in a buffer. This is simply a wrapper for ext_disasm
that provides a default value for <i>args</i>.
See ext_disasm.
=end
    def disassemble( target, args={}, &block )
      # Wrapper provides a default option
      ext_disassemble(target, args, &block)
    end

=begin rdoc
Convenience method for invoking disassemble() with STRATEGY_SINGLE.
=end
    def disasm_single( target, args={}, &block )
      args[:strategy] = STRATEGY_SINGLE
      disassemble(target, args, &block)
    end

=begin rdoc
Convenience method for invoking disassemble() with STRATEGY_LINEAR.
=end
    def disasm_linear( target, args={}, &block )
      args[:strategy] = STRATEGY_LINEAR
      disassemble(target, args, &block)
    end

=begin rdoc
Convenience method for invoking disassemble() with STRATEGY_CFLOW.
=end
    def disasm_cflow( target, args={}, &block )
      args[:strategy] = STRATEGY_CFLOW
      disassemble(target, args, &block)
    end

=begin rdoc
Convenience method for invoking disassemble() with STRATEGY_SECTION.
=end
    def disasm_section( target, args={}, &block )
      args[:strategy] = STRATEGY_SECTION
      disassemble(target, args, &block)
    end

=begin rdoc
Convenience method for invoking disassemble() with STRATEGY_SYMBOL.
=end
    def disasm_symbol( target, args={}, &block )
      args[:strategy] = STRATEGY_SYMBOL
      disassemble(target, args, &block)
    end

=begin rdoc
Convenience method for invoking disassemble() with STRATEGY_ENTRY.
=end
    def disasm_entry( target, args={}, &block )
      args[:strategy] = STRATEGY_ENTRY
      disassemble(target, args, &block)
    end

=begin rdoc
Convenience alias for disassemble().
=end
    alias :disasm :disassemble

=begin rdoc
Return array of available disassembler options, as printed by libopcodes.
=end
    def self.options()
      opts = []
      Tempfile.open("opcodes-options") do |tmp|
        tmp.close
        ext_usage(tmp)
        tmp.open
        opts.concat tmp.readlines
      end

      opts
    end

  end

end
