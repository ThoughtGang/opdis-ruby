#!/usr/bin/env/ruby1.0
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to the Opdis module

require 'OpdisExt'

module Opdis

  class Disassembler

=begin rdoc
=end

  def self.new(args={})
    ext_new(args)
  end

=begin rdoc
Disassemble all bytes in a buffer. This is simply a wrapper for ext_disasm
that provides a default value for <i>args</i>.
See ext_disasm.
=end
    def disassemble( target, args={} )
      # Wrapper provides a default option
      ext_disassemble(target, args)
    end

=begin rdoc
=end
    alias :disasm, :disassemble

  end

end
