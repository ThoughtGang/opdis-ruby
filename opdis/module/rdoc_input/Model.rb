#!/usr/bin/env ruby1.9
# :title: Opdis::Callbacks
=begin rdoc
=Opdis Callbacks
<i>Copyright 2010 Thoughtgang <http://www.thoughtgang.org></i>

descr
=end


module Opdis

  class Instruction
    attr_reader :vma, :size, :bytes, :target, :dest, :src
    attr_reader :flags, :prefixes, :operands
    attr_accessor :status, :mnemonic, :category, :isa, :comment

    DECODE_INVALID='invalid'
    DECODE_BASIC='basic'
    DECODE_MNEMONIC='mnemonic'
    DECODE_OPERANDS='operands'
    DECODE_MNEMONIC_FLAGS='mnemonic flags'
    DECODE_OPERAND_FLAGS='operand flags'

    ISA_GEN='general'
    ISA_FPU='fpu'
    ISA_GPU='gpu'
    ISA_SIMD='simd'
    ISA_VM='vm'
    
    CAT_CFLOW='control-flow'
    CAT_STACK='stack'
    CAT_LOADSTORE='load/store'
    CAT_TEST='test'
    CAT_MATH='mathematic'
    CAT_BIT='bitwise'
    CAT_IO='i/o'
    CAT_TRAP='trap'
    CAT_PRIV='privileged'
    CAT_NOP='no-op'

    FLG_CALL='call'
    FLG_CALLCC='conditional call'
    FLG_JMP='jump'
    FLG_JMPCC='conditional jump'
    FLG_RET='return'
    FLG_PUSH='push'
    FLG_POP='pop'
    FLG_FRAME='enter frame'
    FLG_UNFRAME='leave frame'
    FLG_AND='bitwise and'
    FLG_OR='bitwise or'
    FLG_XOR='bitwise xor'
    FLG_NOT='bitwise not'
    FLG_LSL='logical shift left'
    FLG_LSR='logical shift right'
    FLG_ASL='arithmetic shift left'
    FLG_ASR='arithmetic shift right'
    FLG_ROL='rotate left'
    FLG_ROR='rotate right'
    FLG_RCL='rotate carry left'
    FLG_RCR='rotate carry right'
    FLG_IN='input from port'
    FLG_OUT='output to port'

    def initialize(args)
    end

    def branch?
    end

    def fallthrough?
    end

    def to_s
    end
  end

  class Operand
    attr_accessor :flags, :data_size

    FLG_R='r'
    FLG_W='w'
    FLG_X='x'
    FLG_SIGNED='signed'
    FLG_ADDR='address'
    FLG_INDIRECT='indirect address'

    def to_s
    end
  end

  class ImmediateOperand < Operand
    attr_accessor :value, :signed, :unsigned, :vma
  end

  class AddressExpression
    attr_accessor :shift, :scale, :index, :base, :displacement

    SHIFT_LSL='lsl'
    SHIFT_LSR='lsr'
    SHIFT_ASL='asl'
    SHIFT_ROR='ror'
    SHIFT_RRX='rrx'
  end

  class AddressExpressionOperand < Operand
    attr_accessor :shift, :scale, :index, :base, :displacement

    SHIFT_LSL='lsl'
    SHIFT_LSR='lsr'
    SHIFT_ASL='asl'
    SHIFT_ROR='ror'
    SHIFT_RRX='rrx'
  end

  class AbsoluteAddress
    attr_accessor :segment, :offset
  end

  class AbsoluteAddressOperand < Operand
    attr_accessor :segment, :offset
  end

  class Register
    attr_reader :id, :size, :name
    attr_accessor :flags

    FLG_GEN='general purpose'
    FLG_FPU='fpu'
    FLG_GPU='gpu'
    FLG_SIMD='simd'
    FLG_TASK='task mgt'
    FLG_MEM='memory mgt'
    FLG_DBG='debug'
    FLG_PC='pc'
    FLG_FLAGS='flags'
    FLG_STACK='stack'
    FLG_FRAME='stack frame'
    FLG_SEG='segment'
    FLG_Z='zero'
    FLG_IN='args in'
    FLG_OUT='args out'
    FLG_LOCALS='locals'
    FLG_RET='return'
  end

  class RegisterOperand < Operand
    attr_reader :id, :size, :name
    attr_accessor :flags

    FLG_GEN='general purpose'
    FLG_FPU='fpu'
    FLG_GPU='gpu'
    FLG_SIMD='simd'
    FLG_TASK='task mgt'
    FLG_MEM='memory mgt'
    FLG_DBG='debug'
    FLG_PC='pc'
    FLG_FLAGS='flags'
    FLG_STACK='stack'
    FLG_FRAME='stack frame'
    FLG_SEG='segment'
    FLG_Z='zero'
    FLG_IN='args in'
    FLG_OUT='args out'
    FLG_LOCALS='locals'
    FLG_RET='return'
  end

end
