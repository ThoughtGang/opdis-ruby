#!/usr/bin/env/ruby1.0
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to the Opcodes module

require 'OpcodesExt'

module Opcodes

  class Disassembler

=begin rdoc
Disassemble all bytes in a buffer. This is simply a wrapper for ext_disasm
that provides a default value for <i>args</i>.
=end
    def disasm( target, args={} )
      # Wrapper provides a default option
      ext_disasm(target, args)
    end

=begin rdoc
Disassemble a single instruction in a target buffer. This is simply a wrapper 
for ext_disasm_insn that provides a default value for <i>args</i>.
=end
    def disasm_insn( target, args={} )
      # Wrapper provides a default option
      ext_disasm_insn(target, args)
    end

  end

end
