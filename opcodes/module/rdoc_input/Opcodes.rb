#!/usr/bin/env ruby1.9
# :title: Opcodes
=begin rdoc
=LibOpcodes
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

= Gnu Binutils libopcodes support

Requires GNU binutils, available at
http://www.gnu.org/software/binutils .

All of the original libopcodes data structures and constants can be found in
<b>dis-asm.h</b>, available on Linux installations at 
<b>/usr/include/dis-asm.h</b>.

== Summary
A wrapper for the libopcodes disassembler distributed with GNU binutils.

== Example
require 'BFD'
require 'Opcodes'

t = Bfd::Target.new('/tmp/a.out', {})

o = Opcodes::Disassembler.new( :bfd => t, :arch => 'x86' )

o.disasm( t.sections['.text'], {} )

== Disclaimer
This is a minimal implementation of a libopcodes wrapper. It is intended for
demonstration purposes only, and should not be expected to support all
of the funtionality of the GNU binutils lipopcodes library.

For a fully functional disassembler, see the Opdis gem.

== Contact
Support:: community@thoughtgang.org
Project:: http://rubyforge.org/projects/opdis/
=end

=begin rdoc
GNU libopcodes support
=end
module Opcodes

=begin rdoc
A disassembler object.
Source: <b>struct disassemble_info</b>
=end
  class Disassembler

=begin rdoc
Disassembler options.
Source: <b>disassemble_info.disassembler_options</b>
=end
    attr_accessor :options

=begin rdoc
The <i>args</i> hash can contain the following options:
 bfd:: A Bfd::Target object to use for configuring the disassembler.
 arch:: Architecture of target.
 bfd:: Bfd::Target to use to configure the disassembler.
 opts:: Disassembler options (see @options).
=end
  def initialize(args)
  end

=begin rdoc
Disassemble a single instruction.

The arguments are:

 tgt:: The target to disassemble. This can be a filename, an IO object, a
       Bfd::Section object, or a Bfd::Symbol object.
 args:: A has of optional arguments.

The optional arguments can include:

 vma:: Virtual Memory (Load) Address to disassemble.
 buffer_vma:: Virtual Memory (Load) address of target.

This returns a hash with the following members:

 vma:: Virtual Memory (Load) Address of the instruction. 
 size:: Size of the instruction in bytes.
 info:: Meta-info for the instruction, if available.
 insn:: Array of strings returned from libopcodes.

The <i>info</i> member is a hash with the following members:

 branch_delay_insn:: How many instructions before branch takes effect.
 data_size:: Size of data reference in instruction.
 type:: Type of instruction (e.g. branch).
 target:: Target address of branch or dereference.
 target2:: Second target address.
=end
    def ext_disasm_insn(tgt, args)
    end

=begin rdoc
Disassemble instructions in a target.

The arguments are:

 tgt:: The target to disassemble. This can be a filename, an IO object, a
       Bfd::Section object, or a Bfd::Synmbol object.
 args:: A has of optional arguments.

The optional arguments can include:

 vma:: Virtual Memory (Load) Address to disassemble.
 length:: Number of bytes to disassemble. The default is the length of the
          buffer.
 buffer_vma:: Virtual Memory (Load) address of target. Note that this will
              override the VMA for Bfd objects if supplied; this allows
              the caller to disassemble the first <i>n</i> bytes of
              a Bfd::Section without knowing its VMA by specifying a
              <i>buffer_vma</i> of 0 and a <i>length</i> of <i>n</i>.

This returns an array of the hash described in disasm_insn.
=end
    def ext_disasm( tgt, args )
    end


=begin rdoc
List the architectures supported by the local copy of libopcodes.
This returns an array of strings.
=end
    def architectures()
    end

  end

end


