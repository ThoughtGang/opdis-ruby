#!/usr/bin/env ruby1.9
# Opdis Example: Resolver
# Custom Address Resolver used in control-flow disassemlby of BFD entry point
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>

require 'BFD'
require 'Opdis'

# ----------------------------------------------------------------------
class Opdis::ImmediateOperand
  # return immediate operand value
  def resolve( ign )
    return self.vma
  end
end

# ----------------------------------------------------------------------
class Opdis::AddressExpressionOperand
  # attempt to resolve address expression [if stack-register only]
  def resolve( vm )
    # TODO: if base is stack or frame register, return item from vm.stack;
    # otherwise, return nil (need to be able to deref mem addr for
    # general addr expressions).
    return nil
  end
end

# ----------------------------------------------------------------------
class Opdis::AbsoluteAddressOperand
  # attempt to resolve absolute address [segment reg + addr]
  def resolve( vm )
    # TODO: lookup segment register in VM; return reg value + addr
    return nil
  end
end

# ----------------------------------------------------------------------
class Opdis::RegisterOperand
  # attempt to resolve register operand
  def resolve( vm )
    # TODO: lookup register name in vm.registers and return stored value
    return nil
  end
end

# ----------------------------------------------------------------------
class CustomResolver < Opdis::AddressResolver
  attr_reader :registers  # reg name -> value
  attr_reader :stack      # array of pushed values
  attr_reader :stack_ptr  # index (into @stack) of 'top' of stack
  attr_reader :frame_ptr  # index (into @stack) of start of frame

  def initialize()
    @registers = {}
    @stack = []
    @stack_ptr = @frame_ptr = nil
  end

  # return VMA for target operand or nil
  def resolve(insn)
    return insn.target ? insn.target.resolve(self) : nil
  end

  # -- Stack Management --
  def stack_push( val )
    @stack.push val
    @stack_ptr = @stack.length - 1
  end

  def stack_pop()
    val = @stack.push
    @stack_ptr = @stack.length - 1
    return val
  end

  def stack_frame()
    @frame_ptr = @stack_ptr
  end

  def stack_unframe()
    @stack_ptr = @frame_ptr
  end

  def stack_adjust( val )
    if val < 0 
      val.abs.times { @stack.push 0 }
    else
      val.abs.times { @stack.pop }
    end

    @stack_ptr = @stack.length - 1
  end

  # -- Register Management --
  # -- Instruction Handlers --
  # call insn
  def call(insn, ign, ignn)
    stack_push insn.vma + insn.size
  end

  # return insn
  def ret(insn, ign, ignn)
    stack_pop
  end

  # push insn
  def push(ign, op, ignn)
    # TODO: stack_push op.value
  end

  # pop insn
  def pop(ign, op, ignn)
    val = stack_pop
    # TODO: if dest is reg, move val to reg, otherwise discard
  end

  # frame insn
  def frame(ign, ignn, ignnn)
    stack_frame
  end

  # unframe insn
  def unframe(ign, ignn, ignnn)
    stack_unframe
  end

  # load/store insn
  def lost(ign, src, dest)
    # TODO: if dest is addr_expr for stack, set stack item to reg.value
    #       if dest is reg, fill reg with src.value
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
    INSN_HANDLERS.each do |hdlr|
      if insn.category == hdlr[0] && (not hdlr[1] or insn.flags & hdlr[1])
        # Invoke handler method with instruction and src, dest operands
        self.send( hdlr[2], insn, insn.src, insn.dest )
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
  # custom resolver to use in disassembler
  vm = CustomResolver.new

  Opdis::Disassembler.new( resolver: vm ) do |dis|

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
