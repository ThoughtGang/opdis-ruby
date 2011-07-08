#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to Ptrace extension

require 'Ptrace_ext'            # Load C extension wrapping ptrace(3)
require 'forwardable'

=begin rdoc
Note: Debugger is a class composed of singleton methods, defined in the C
extension module.
=end
module Ptrace

  # -----------------------------------------------------------------------
=begin rdoc
=end
  class Error < RuntimeError
  end

=begin rdoc
"No such process" returned in errno.
=end
  class InvalidProcessError < Error
  end

=begin rdoc
"Operation not permitted" returned in errno.
=end
  class OperationNotPermittedError < Error
  end

=begin rdoc
Requested command is not listed in PTRACE_COMMANDS and is considered 
unsupported.
=end
  class OperationNotSupportedError < Error
  end

  # -----------------------------------------------------------------------
=begin rdoc
Hash mapping ptrace symbols to command numbers. This is filled by the
Debugger based on the #defines in sys/ptrace.h.
=end
  PTRACE_COMMANDS = Debugger.commands

=begin rdoc
Hash mapping syscall numbers to syscall names (stored as Symbols). This is 
filled by the Debugger based on the #defined in sys/syscalls.h.
=end
  SYSCALL_MAP = Debugger.syscalls

  # -----------------------------------------------------------------------
=begin rdoc
=end
  # all ptrace options
  class Options
    def initialize(pid)
      @pid = pid
    end

    def write
      opts = []

      begin
        Debugger.send_data( Debugger.commands[:set_options], @pid, nil, opts )
      rescue RuntimeError => e
        case e.message
          when 'PTRACE: Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'PTRACE: No such process'
            raise InvalidProcessError.new(e.message)
          else
            raise
          end
      end
    end
  end

  # -----------------------------------------------------------------------
=begin rdoc
A region of Memory in the target process.

Usage:
  mem = MemArea.new(MemArea::DATA, pid)
  val = mem.peek(0x0804100)
  mem.poke(0x0804100, 0x0)
=end
  class MemArea

=begin rdoc
Target user area.
=end
    MEM_USER = 1
=begin rdoc
Target text (code) area.
=end
    MEM_TEXT = 2
=begin rdoc
Target data area.
=end
    MEM_DATA = 3

=begin rdoc
Valid memory region types.
=end
    TYPES = [MEM_USER, MEM_TEXT, MEM_DATA]

=begin rdoc
Type of memory region.
=end
    attr_reader :mem_type
=begin rdoc
PID of process owning this memory region.
=end
    attr_reader :pid

=begin rdoc
Create a new memory region of the specified type for process 'pid'.
=end
    def initialize(type, pid)
      @mem_type = type
      @pid = pid
      case type
        when MEM_USER
          @getter_sym = :peekusr
          @setter_sym = :pokeusr
        when MEM_TEXT
          @getter_sym = :peektext
          @setter_sym = :poketext
        when MEM_DATA
          @getter_sym = :peekdata
          @setter_sym = :pokedata
      end
    end

=begin rdoc
Read a word of data from address 'addr' in memory region.
This can raise an OperationNotPermittedError if access is denied, or an
InvalidProcessError if the target process has exited.
=end
    def peek(addr)
      ptrace_send(:peek, @getter_sym, addr)
    end

=begin rdoc
Write a word of data to address 'addr' in memory region.
This can raise an OperationNotPermittedError if access is denied, or an
InvalidProcessError if the target process has exited.
=end
    def poke(addr, value)
      ptrace_send(:poke, @setter_sym, addr, value)
    end

    private

    def ptrace_send( sym, cmd, addr, arg=nil )
      begin
        raise OperationNotSupportedError if (not PTRACE_COMMANDS.include? cmd)
        args = [PTRACE_COMMANDS[cmd], @pid, addr]
        args << arg if arg
        Debugger.send( sym, *args )
      rescue RuntimeError => e
        case e.message
          when 'PTRACE: Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'PTRACE: No such process'
            raise InvalidProcessError.new(e.message)
          else
            raise
        end
      end
    end
  end

  # -----------------------------------------------------------------------
=begin rdoc
=end
  class Signal
=begin rdoc
=end
    # create signinfo
    def initialize()
    end

=begin rdoc
=end
    # send a signal to the child
    def send
      # TODO: begin/rescue
      #Debugger.signal
    end

=begin rdoc
=end
    # read a signal from the child. returns a Signal data struct.
    def self.get(pid)
      # TODO: begin/rescue
      # send message
      #Debugger.signal=
      # Signal.new
    end

  end

  # -----------------------------------------------------------------------
=begin rdoc
The CPU registers for the process. This acts as a Hash mapping register names
to contents. The Hash acts as a snapshot of the CPU state; the registers are 
read from the process using read(), and written to the process using write().

Usage:
  regs = RegSet.new(RegSet::GEN, pid)
  regs.read
  puts regs.inspect
  regs['eax'] = 0x0
  regs.write
=end
  class RegSet 
    extend Forwardable
    extend Enumerable

=begin rdoc
General register set (EAX and friends in x86).
=end
    GEN = 0
=begin rdoc
Floating-point register set (ST(0) and friends in x86).
=end
    FP = 1
=begin rdoc
Extended floating-point register set (fpx on Linux).
=end
    EXT = 2
=begin rdoc
Valid register set types.
=end
    TYPES = [GEN, FP, EXT]

=begin rdoc
Method names for read accessor, keyed by register type.
=end
    GETTER_SYMS = [:regs, :fpregs, :fpxregs ]
=begin rdoc
Method names for write accessor, keyed by register type.
=end
    SETTER_SYMS = [:regs=, :fpregs=, :fpxregs= ]

=begin rdoc
Type of this register set.
=end
    attr_reader :reg_type
=begin rdoc
PID of process owning this register set.
=end
    attr_reader :pid

=begin rdoc
Create a new register set of the specified type for process 'pid'.
=end
    def initialize(type, pid)
      @reg_type = type
      @getter = GETTER_SYMS[type]
      @setter = SETTER_SYMS[type]
      @pid = pid
      @regs = {}
    end

=begin rdoc
Read the current state of the CPU registers from the process. This fills the
contents of the RegSet Hash, and returns the Hash.
This can raise an OperationNotPermittedError if access is denied, or an
InvalidProcessError if the target process has exited.
=end
    def read
      @regs = ptrace_send(@getter)
    end

=begin rdoc
Write the contents of the RegSet Hash to the process CPU registers.
This can raise an OperationNotPermittedError if access is denied, or an
InvalidProcessError if the target process has exited.
=end
    def write
      ptrace_send(@setter, @regs)
    end

    def_delegators :@regs, :[], :[]=, :clear, :each, :delete, :each_key,
                   :each_pair, :each_value, :empty?, :fetch, :has_key?,
                   :has_value?, :include?, :index, :indexes, :invert,
                   :key?, :keys, :length?, :member, :reject, :replace,
                   :shift, :size, :sort, :store, :to_a, :to_s, :update,
                   :value?, :values

    private

    def ptrace_send( sym, arg=nil )
      begin
        args = [@pid]
        args << arg if arg
        Debugger.send( sym, *args )
      rescue RuntimeError => e
        case e.message
          when 'PTRACE: Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'PTRACE: No such process'
            raise InvalidProcessError.new(e.message)
          else
            raise
        end
      end
    end
  end

  # -----------------------------------------------------------------------
=begin rdoc
A target process managed by ptrace.

Usage:
  tgt = Target.attach(pid)
  loop do
    begin
      tgt.step
      puts tgt.regs.read.inspect
    rescue Ptrace::InvalidProcessError
      break
    end
  end

  tgt = Target.launch(cmd)
  loop do
    begin
      state = tgt.syscall_state
      puts state.inspect
    rescue Ptrace::InvalidProcessError
      break
    end
  end
=end
  class Target

=begin rdoc
PID of target process.
=end
    attr_accessor :pid
=begin rdoc
General (CPU) registers for process.
=end
    attr_accessor :regs
=begin rdoc
FPU registers for process.
=end
    attr_accessor :fpregs
=begin rdoc
extended FPU registers for process.
=end
    attr_accessor :fpregs
=begin rdoc
Text (code) segment for process.
=end
    attr_accessor :text
=begin rdoc
Data segment for process.
=end
    attr_accessor :data
=begin rdoc
Target 'user' (task) area.
=end
    attr_accessor :user
=begin rdoc
Ptrace options.
=end
    attr_accessor :options

=begin rdoc
Create a Ptrace::Target object for process 'pid'. The process is assumed to
have been launched or attached to by ptrace, e.g. using Target.launch or 
Target.attach.
=end
    def initialize(pid)
      @pid = pid
      @text = MemArea.new(MemArea::MEM_TEXT, pid)
      @data = MemArea.new(MemArea::MEM_DATA, pid)
      @user = MemArea.new(MemArea::MEM_USER, pid)
      @regs = RegSet.new(RegSet::GEN, pid)
      @fpregs = RegSet.new(RegSet::FP, pid)
      @fpxregs = RegSet.new(RegSet::Ext, pid)
      @options = Options.new(pid)
      @valid = true
    end

=begin rdoc
PT_ATTACH : Attach to running process 'pid' and return a Ptrace::Target object.
Raises an exception if the attach fails.
=end
    def self.attach(pid)
      tgt = Target.new(pid)
      begin
        Ptrace::Debugger.send_cmd( Ptrace::Debugger.commands[:attach], pid, nil)
        Process.waitpid(pid)
      rescue RuntimeError => e
        case e.message
          when 'PTRACE: Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'PTRACE: No such process'
            raise InvalidProcessError.new(e.message)
          else
            raise
        end
      end
      return tgt
    end

=begin rdoc
PT_TRACE_ME : Launch command 'cmd' and return a Ptrace::Target object for 
controlling it.
Raises an exception if the command cannot be launched; returns nil if
the command cannot be traced.
=end
    def self.launch(cmd)
      pid = fork
      if ! pid
        begin
          Ptrace::Debugger.send_cmd(Ptrace::Debugger.commands[:traceme], nil, 
                                    nil)
          exec(cmd)
        rescue RuntimeError => e
          case e.message
            when 'PTRACE: Operation not permitted'
              raise OperationNotPermittedError.new(e.message)
            when 'PTRACE: No such process'
              raise InvalidProcessError.new(e.message)
            else
              raise
          end
        end

      elsif pid == -1
        return nil

      else
        Process.waitpid(pid)
        tgt = Target.new(pid)
        return tgt
      end
    end

=begin rdoc
=end
    def signal(sig=nil)
      # if sig, send
      # else:
      Signal.read(pid)
    end

=begin rdoc
=end
    def event_msg
      # send signal
      # For PTRACE_EVENT_EXIT this is the child's exit status. For PTRACE_EVENT_FORK, PTRACE_EVENT_VFORK and PTRACE_EVENT_CLONE this is the PID of the new process. 
      # Debugger.event_msg
    end

=begin rdoc
PT_KILL : Terminate the process.
Note: This makes the Ptrace::Target object invalid.
=end
    def kill
      ptrace_send( :kill )
      Process.waitpid(@pid)
      @valid = false
    end

=begin rdoc
PT_DETACH : Detach from the process.
Note: This makes the Ptrace::Target object invalid.
=end
    def detach
      ptrace_send( :detach )
      Process.waitpid(@pid)
      @valid = false
    end

=begin rdoc
PT_STEP : Step a single instruction.
=end
    def step
      ptrace_send( :singlestep )
      Process.waitpid(@pid)
    end

=begin rdoc
PT_SYSCALL : Execute until the start or end of the next system call.

Usage:
  tgt.syscall
  in_regs = tgt.regs.read
  tgt.syscall
  out_regs = tgt.regs.read
=end
    def syscall
      ptrace_send( :syscall )
      Process.waitpid(@pid)
    end

=begin rdoc
Wrapper for recording syscalls. This issues a PT_SYSCALL to stop the target at
the next syscall, records the 'in' register set, issues a PT_SYSCALL to stop the
target after the syscall returns, records the 'out' register set, and 
returns a Hash  { :in, :out } of the register sets. The target is stopped
on return from this syscall.
=end
    def syscall_state
      begin
        state = {}
        syscall
        state[:in] = @regs.read
        syscall
        state[:out] = @regs.read
        state
      rescue InvalidProcessError
        # Program exited without a syscall
        return state
      end
    end

=begin rdoc
PT_CONTINUE: Continue execution of target.
=end
    def cont
      ptrace_send( :cont )
      Process.waitpid(@pid)
    end

=begin rdoc
PT_SYSEMU
=end
    def sysemu
      ptrace_send( :sysemu )
    end

=begin rdoc
PT_SYSEMU_SINGLESTEP
=end
    def sysemu_step
      ptrace_send( :sysemu_singlestep )
    end

=begin rdoc
=end
    # misc
    def options=
      # TODO
      # ptrace_send_data( :setoptions,  )
    end

    private

    def ptrace_send( cmd, arg=nil )
      begin
        raise OperationNotSupportedError if (not PTRACE_COMMANDS.include? cmd)
        raise OperationNotPermittedError if (not @valid)

        Debugger.send_cmd( PTRACE_COMMANDS[cmd], @pid, arg )
      rescue RuntimeError => e
        case e.message
          when 'PTRACE: Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'PTRACE: No such process'
            raise InvalidProcessError.new(e.message)
          else
            raise
        end
      end
    end

  end
end
