#!/usr/bin/env ruby1.9
# :title: Opdis::DataModel
=begin rdoc
=Opdis Data Model
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

= Opdis Data Model

== Summary

==Example

==Contact
Support:: community@thoughtgang.org
Project:: http://rubyforge.org/projects/opdis/ 
=end


module Opdis

=begin rdoc
=end
  class Instruction

=begin rdoc
The decoding status of an instruction. This will be DECODE_INVALID or any
combination of DECODE_BASIC, DECODE_MNEMONIC, DECODE_OPERANDS,
DECODE_MNEMONIC_FLAGS, and DECODE_OPERAND_FLAGS -- depending on how much
work the instruction decoder performed successfully.
=end
    attr_accessor :status

=begin rdoc
Virtual Memory Address. The in-memory address of the instruction.
=end
    attr_reader :vma

=begin rdoc
The size of the instruction in bytes.
=end
    attr_reader :size

=begin rdoc
An array containing the bytes of the instruction.
=end
    attr_reader :bytes

=begin rdoc
The target operand of a branch instruction, or <i>nil</i>.
=end
    attr_reader :target

=begin rdoc
The destination (write) operand of an instruction, or <i>nil</i>.
=end
    attr_reader :dest

=begin rdoc
The first source (read) operand of an instruction, or <i>nil</i>.
=end
    attr_reader :src

=begin rdoc
The category or high-level type of the instruction. This will be one of
CAT_CFLOW, CAT_STACK, CAT_LOADSTORE, CAT_TEST, CAT_MATH, CAT_BIT, CAT_IO,
CAT_TRAP, CAT_PRIV, CAT_NOP, or <i>nil</i> if the instruction category is
unknown.
=end
    attr_accessor :category

=begin rdoc
=end
    attr_reader :flags

=begin rdoc
The Instruction Set Architecture of the instruction. This is a subset of the
full CPU architecture ISA. 
    ISA_GEN, ISA_FPU, ISA_GPU, ISA_SIMD, ISA_VM
=end
    attr_accessor :isa

=begin rdoc
=end
    attr_reader :prefixes

=begin rdoc
=end
    attr_reader :operands

=begin rdoc
=end
    attr_accessor :mnemonic

=begin rdoc
=end
    attr_accessor :comment

=begin rdoc
=end
    DECODE_INVALID='invalid'
=begin rdoc
=end
    DECODE_BASIC='basic'
=begin rdoc
=end
    DECODE_MNEMONIC='mnemonic'
=begin rdoc
=end
    DECODE_OPERANDS='operands'
=begin rdoc
=end
    DECODE_MNEMONIC_FLAGS='mnemonic flags'
=begin rdoc
=end
    DECODE_OPERAND_FLAGS='operand flags'

=begin rdoc
=end
    ISA_GEN='general'
=begin rdoc
=end
    ISA_FPU='fpu'
=begin rdoc
=end
    ISA_GPU='gpu'
=begin rdoc
=end
    ISA_SIMD='simd'
=begin rdoc
=end
    ISA_VM='vm'
    
=begin rdoc
=end
    CAT_CFLOW='control-flow'
=begin rdoc
=end
    CAT_STACK='stack'
=begin rdoc
=end
    CAT_LOADSTORE='load/store'
=begin rdoc
=end
    CAT_TEST='test'
=begin rdoc
=end
    CAT_MATH='mathematic'
=begin rdoc
=end
    CAT_BIT='bitwise'
=begin rdoc
=end
    CAT_IO='i/o'
=begin rdoc
=end
    CAT_TRAP='trap'
=begin rdoc
=end
    CAT_PRIV='privileged'
=begin rdoc
=end
    CAT_NOP='no-op'

=begin rdoc
=end
    FLG_CALL='call'
=begin rdoc
=end
    FLG_CALLCC='conditional call'
=begin rdoc
=end
    FLG_JMP='jump'
=begin rdoc
=end
    FLG_JMPCC='conditional jump'
=begin rdoc
=end
    FLG_RET='return'
=begin rdoc
=end
    FLG_PUSH='push'
=begin rdoc
=end
    FLG_POP='pop'
=begin rdoc
=end
    FLG_FRAME='enter frame'
=begin rdoc
=end
    FLG_UNFRAME='leave frame'
=begin rdoc
=end
    FLG_AND='bitwise and'
=begin rdoc
=end
    FLG_OR='bitwise or'
=begin rdoc
=end
    FLG_XOR='bitwise xor'
=begin rdoc
=end
    FLG_NOT='bitwise not'
=begin rdoc
=end
    FLG_LSL='logical shift left'
=begin rdoc
=end
    FLG_LSR='logical shift right'
=begin rdoc
=end
    FLG_ASL='arithmetic shift left'
=begin rdoc
=end
    FLG_ASR='arithmetic shift right'
=begin rdoc
=end
    FLG_ROL='rotate left'
=begin rdoc
=end
    FLG_ROR='rotate right'
=begin rdoc
=end
    FLG_RCL='rotate carry left'
=begin rdoc
=end
    FLG_RCR='rotate carry right'
=begin rdoc
=end
    FLG_IN='input from port'
=begin rdoc
=end
    FLG_OUT='output to port'

=begin rdoc
=end
    def initialize(args)
    end

=begin rdoc
=end
    def branch?
    end

=begin rdoc
=end
    def fallthrough?
    end

=begin rdoc
=end
    def to_s
    end
  end

=begin rdoc
=end
  class Operand

=begin rdoc
=end
    attr_accessor :flags

=begin rdoc
=end
    attr_accessor :data_size

=begin rdoc
=end
    FLG_R='r'
=begin rdoc
=end
    FLG_W='w'
=begin rdoc
=end
    FLG_X='x'
=begin rdoc
=end
    FLG_SIGNED='signed'
=begin rdoc
=end
    FLG_ADDR='address'
=begin rdoc
=end
    FLG_INDIRECT='indirect address'

=begin rdoc
=end
    def to_s
    end
  end

=begin rdoc
=end
  class ImmediateOperand < Operand

=begin rdoc
=end
    attr_accessor :value

=begin rdoc
=end
    attr_accessor :signed

=begin rdoc
=end
    attr_accessor :unsigned

=begin rdoc
=end
    attr_accessor :vma
  end

=begin rdoc
=end
  class AddressExpression

=begin rdoc
=end
    attr_accessor :shift

=begin rdoc
=end
    attr_accessor :scale

=begin rdoc
=end
    attr_accessor :index

=begin rdoc
=end
    attr_accessor :base

=begin rdoc
=end
    attr_accessor :displacement

=begin rdoc
=end
    SHIFT_LSL='lsl'
=begin rdoc
=end
    SHIFT_LSR='lsr'
=begin rdoc
=end
    SHIFT_ASL='asl'
=begin rdoc
=end
    SHIFT_ROR='ror'
=begin rdoc
=end
    SHIFT_RRX='rrx'
  end

=begin rdoc
=end
  class AddressExpressionOperand < Operand

=begin rdoc
=end
    attr_accessor :shift

=begin rdoc
=end
    attr_accessor :scale

=begin rdoc
=end
    attr_accessor :index

=begin rdoc
=end
    attr_accessor :base

=begin rdoc
=end
    attr_accessor :displacement

=begin rdoc
=end
    SHIFT_LSL='lsl'
=begin rdoc
=end
    SHIFT_LSR='lsr'
=begin rdoc
=end
    SHIFT_ASL='asl'
=begin rdoc
=end
    SHIFT_ROR='ror'
=begin rdoc
=end
    SHIFT_RRX='rrx'
  end

=begin rdoc
=end
  class AbsoluteAddress

=begin rdoc
=end
    attr_accessor :segment

=begin rdoc
=end
    attr_accessor :offset
  end

=begin rdoc
=end
  class AbsoluteAddressOperand < Operand

=begin rdoc
=end
    attr_accessor :segment

=begin rdoc
=end
    attr_accessor :offset
  end

=begin rdoc
=end
  class Register

=begin rdoc
=end
    attr_reader :id

=begin rdoc
=end
    attr_reader :size

=begin rdoc
=end
    attr_reader :name

=begin rdoc
=end
    attr_accessor :flags

=begin rdoc
=end
    FLG_GEN='general purpose'
=begin rdoc
=end
    FLG_FPU='fpu'
=begin rdoc
=end
    FLG_GPU='gpu'
=begin rdoc
=end
    FLG_SIMD='simd'
=begin rdoc
=end
    FLG_TASK='task mgt'
=begin rdoc
=end
    FLG_MEM='memory mgt'
=begin rdoc
=end
    FLG_DBG='debug'
=begin rdoc
=end
    FLG_PC='pc'
=begin rdoc
=end
    FLG_FLAGS='flags'
=begin rdoc
=end
    FLG_STACK='stack'
=begin rdoc
=end
    FLG_FRAME='stack frame'
=begin rdoc
=end
    FLG_SEG='segment'
=begin rdoc
=end
    FLG_Z='zero'
=begin rdoc
=end
    FLG_IN='args in'
=begin rdoc
=end
    FLG_OUT='args out'
=begin rdoc
=end
    FLG_LOCALS='locals'
=begin rdoc
=end
    FLG_RET='return'
  end

=begin rdoc
=end
  class RegisterOperand < Operand

=begin rdoc
=end
    attr_reader :id

=begin rdoc
=end
    attr_reader :size

=begin rdoc
=end
    attr_reader :name

=begin rdoc
=end
    attr_accessor :flags

=begin rdoc
=end
    FLG_GEN='general purpose'
=begin rdoc
=end
    FLG_FPU='fpu'
=begin rdoc
=end
    FLG_GPU='gpu'
=begin rdoc
=end
    FLG_SIMD='simd'
=begin rdoc
=end
    FLG_TASK='task mgt'
=begin rdoc
=end
    FLG_MEM='memory mgt'
=begin rdoc
=end
    FLG_DBG='debug'
=begin rdoc
=end
    FLG_PC='pc'
=begin rdoc
=end
    FLG_FLAGS='flags'
=begin rdoc
=end
    FLG_STACK='stack'
=begin rdoc
=end
    FLG_FRAME='stack frame'
=begin rdoc
=end
    FLG_SEG='segment'
=begin rdoc
=end
    FLG_Z='zero'
=begin rdoc
=end
    FLG_IN='args in'
=begin rdoc
=end
    FLG_OUT='args out'
=begin rdoc
=end
    FLG_LOCALS='locals'
=begin rdoc
=end
    FLG_RET='return'
  end

end
