#!/usr/bin/env ruby1.9
# Opdis Example: Resolver
# Custom Address Resolver used in control-flow disassemlby of BFD entry point
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

# ----------------------------------------------------------------------
class Opdis::ImmediateOperand
  # return immediate operand value
  def resolve( vm )
  end
end

# ----------------------------------------------------------------------
class Opdis::AddressExpressionOperand
  # attempt to resolve address expression [if stack-register only]
  def resolve( vm )
  end
end

# ----------------------------------------------------------------------
class Opdis::AbsoluteAddressOperand
  # attempt to resolve absolute address [segment reg + addr]
  def resolve( vm )
  end
end

# ----------------------------------------------------------------------
class Opdis::RegisterOperand
  # attempt to resolve register operand
  def resolve( vm )
  end
end

# ----------------------------------------------------------------------
class CustomResolver < Opdis::AddressResolver
  attr_reader :registers, :stack

  def initialize()
    @registers = {}
    @stack = []
  end

  # return VMA for target operand or nil
  def resolve(insn)
    return insn.target ? insn.target.resolve(self) : nil
  end

  # -- Instruction Handlers --
  # call insn
  def call(insn, ign, ignn)
    # push vma + size
  end

  # return insn
  def ret(insn, ign, ignn)
  end

  # push insn
  def push(ign, op, ignn)
  end

  # pop insn
  def pop(ign, op, ignn)
  end

  # frame insn
  def frame(ign, op, ign)
  end

  # unframe insn
  def unframe(ign, op, ignn)
  end

  # load/store insn
  def lost(ign, src, dest)
  end

  INSN_HANDLERS = [
    [CAT_CFLOW, FLG_CALL, :call],
    [CAT_CFLOW, FLG_CALL_CC, :call],
    [CAT_CFLOW, FLG_RET, :ret],
    [CAT_STACK, FLG_PUSH, :push],
    [CAT_STACK, FLG_POP, :pop],
    [CAT_STACK, FLG_FRAME, :frame],
    [CAT_STACK, FLG_UNFRAME, :unframe],
    [CAT_MATH, FLG_ADD, :add],
    [CAT_MATH, FLG_SUB, :sub],
    [CAT_LOADSTORE, nil, :lost]
  ]

  # Modify stack/insn contents based on insn
  def process(insn)
    INSN_HANDLERS.each do |h|
      if insn.category == h[0] && (not h[1] or insn.flags & h[1])
        # invoke insn, insn.src, insn.dest
      end
    end
  end

end

# ----------------------------------------------------------------------
# print an instruction in the standard disasm listing format:
#       VMA 8_hex_bytes instruction
def print_insn(insn)
  hex_str = insn.bytes.collect { |b| "%02X" % b }.join(' ')
  puts "%08X %-23.23s %s" % [ insn.vma, hex_str, insn.ascii ]
end

# ----------------------------------------------------------------------
def disasm_entry( tgt )
  vm = CustomResolver.new

  Opdis::Disassembler.new( resolver: vm) do |dis|

    dis.disasm_entry( tgt ) do |insn|
      # Set register/stack contents based on insn
      vm.process(insn)

    # Print instructions in order of VMA
    end.values.sort_by( |i| i.vma ).each { |i| print_insn(i) }
  end
end

# ----------------------------------------------------------------------
if __FILE__ == $0
  raise "Usage: #{$0} FILE [FILE...]" if ARGV.length == 0

  ARGV.each do |filename| 
    Bfd::Target.new(filename) { |f| disasm_entry( f ) }
  end

end
