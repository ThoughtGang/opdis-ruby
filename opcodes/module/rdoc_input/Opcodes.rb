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

== Example
require 'BFD'
t = Bfd::Target.new('/tmp/a.out', {})

require 'Opcodes'

o = Opcodes::Disassembler.new( :bfd => t, :arch => 'x86' )

o.disasm_insn( t.sections['.text'].contents, :vma => 0 )

== Contact
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

 tgt:: The target to disassemble. This can be a filename, an IO object, or
       a Bfd::Target object.
 args:: A has of optional arguments.

The optional arguments can include:

 vma:: Virtual Memory (Load) Address to disassemble.
 buffer_vma:: Virtual Memory (Load) address of target.

This returns a hash with the following members:

 vma:: Virtual Memory (Load) Address of the instruction. 
 size:: Size of the instruction in bytes.
 info:: Meta-info for the instruction, if available.
 insn:: String containing the disassembled instruction.

The <i>info</i> member is a hash with the following members:

 branch_delay_insn:: How many instructions before branch takes effect.
 data_size:: Size of data reference in instruction.
 type:: Type of instruction (e.g. branch).
 target:: Target address of branch or dereference.
 target2:: Second target address.
=end
    def disasm_insn(tgt, args)
    end

=begin rdoc
Disassemble all instructions in a section.

 sec:: Bfd::Section to disassemble.

Returns an array of the hashes from disasm_insn.

<b>NOTE:</b> Not currently implemented.
=end
    def disasm_section( sec )
    end


=begin rdoc
List the architectures supported by the local copy of libopcodes.
This returns an array of strings.
=end
    def architectures()
    end

  end

end


