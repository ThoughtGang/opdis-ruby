#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to Ptrace module

require 'Ptrace_ext'            # Load C extension wrapping libbfd.so
require 'forwardable'

=begin rdoc
Note: Debugger is a class composed of singleton methods, defined in the C
extension module.
=end
module Ptrace

  # -----------------------------------------------------------------------
  class Error < RuntimeError
  end

  # No such process
  class InvalidProcessError < Error
  end

  # Operation not permitted
  class OperationNotPermittedError < Error
  end

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
          when 'Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'No such process'
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
  class MemArea
=begin rdoc
=end
    MEM_USER = 1
=begin rdoc
=end
    MEM_TEXT = 2
=begin rdoc
=end
    MEM_DATA = 3

=begin rdoc
=end
    TYPES = [MEM_TEXT, MEM_DATA, MEM_USER]

=begin rdoc
=end
    attr_reader :mem_type
=begin rdoc
=end
    attr_reader :pid

=begin rdoc
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
=end
    def peek(addr)
      ptrace_send(@getter_sym, addr)
    end

=begin rdoc
=end
    def poke(addr, value)
      ptrace_send(@setter_sym, addr, value)
    end

    protected

    def ptrace_send( sym, addr, arg=nil )
      begin
        args = [@pid, addr]
        args << arg if arg
        Debugger.send( sym, *args )
      rescue RuntimeError => e
        case e.message
          when 'Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'No such process'
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
=end
  class RegSet 
    extend Forwardable
    extend Enumerable

=begin rdoc
=end
    GEN = 0
=begin rdoc
=end
    FP = 0
=begin rdoc
=end
    TYPES = [GEN, FP]

=begin rdoc
=end
    attr_reader :reg_type
=begin rdoc
=end
    attr_reader :pid

=begin rdoc
=end
    def initialize(type, pid)
      @reg_type = type
      @getter = (type == GEN) ? :regs : :fpregs
      @setter = (type == GEN) ? :regs= : :fpregs=
      @pid = pid
      @regs = {}
    end

=begin rdoc
=end
    def read
      # this returns a Hash
      ptrace_send(@getter)
    end

=begin rdoc
=end
    def write
      ptrace_send(@setter, regs)
    end

    def_delegators :@regs, :[], :[]=, :clear, :each, :delete, :each_key,
                   :each_pair, :each_value, :empty?, :fetch, :has_key?,
                   :has_value?, :include?, :index, :indexes, :invert,
                   :key?, :keys, :length?, :member, :reject, :replace,
                   :shift, :size, :sort, :store, :to_a, :to_s, :update,
                   :value?, :values

    protected

    def ptrace_send( sym, arg=nil )
      begin
        args = [@pid]
        args << arg if arg
        Debugger.send( sym, *args )
      rescue RuntimeError => e
        case e.message
          when 'Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'No such process'
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
  class Target

=begin rdoc
=end
    attr_accessor :pid
=begin rdoc
=end
    attr_accessor :regs
=begin rdoc
=end
    attr_accessor :fpregs
=begin rdoc
=end
    # Target.text.peek/poke, etc
    attr_accessor :text
=begin rdoc
=end
    attr_accessor :data
=begin rdoc
=end
    attr_accessor :user
=begin rdoc
=end
    attr_accessor :options

=begin rdoc
=end
    def initialize(pid)
      @pid = pid
      @text = MemArea.new(MemArea::MEM_TEXT, pid)
      @data = MemArea.new(MemArea::MEM_DATA, pid)
      @user = MemArea.new(MemArea::MEM_USER, pid)
      @regs = RegSet.new(RegSet::GEN, pid)
      @fpregs = RegSet.new(RegSet::FP, pid)
      @ptrace_commands = Debugger.commands
      @options = Options.new(pid)
    end

=begin rdoc
=end
    def self.attach(pid)
      # TODO: verify
      tgt = Target.new(pid)
      begin
      # TODO: begin/rescue
        Ptrace::Debugger.send_cmd( Ptrace::Debugger.commands[:attach], pid, nil)
      rescue RuntimeError => e
        case e.message
          when 'Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'No such process'
            raise InvalidProcessError.new(e.message)
          else
            raise
        end
      end
      return tgt
    end

=begin rdoc
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
            when 'Operation not permitted'
              raise OperationNotPermittedError.new(e.message)
            when 'No such process'
              raise InvalidProcessError.new(e.message)
            else
              raise
          end
        end

      elsif pid == -1
        return false
      else
        sleep 1
        tgt = Target.new(pid)
        # TODO : put a wait call somewhere ?
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
=end
    # actions
    def kill
      ptrace_send( :kill )
    end

=begin rdoc
=end
    def detach
      ptrace_send( :detach )
    end

=begin rdoc
=end
    def step
      ptrace_send( :singlestep )
    end

=begin rdoc
=end
    def syscall
      ptrace_send( :syscall )
    end

=begin rdoc
=end
    def cont
      ptrace_send( :cont )
    end

=begin rdoc
=end
    def sysemu
      ptrace_send( :sysemu )
    end

=begin rdoc
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

    protected

    def ptrace_send( cmd, arg=nil )
      begin
        Debugger.send_cmd( @ptrace_commands[cmd], @pid, arg )
      rescue RuntimeError => e
        case e.message
          when 'Operation not permitted'
            raise OperationNotPermittedError.new(e.message)
          when 'No such process'
            raise InvalidProcessError.new(e.message)
          else
            raise
        end
      end
    end

  end
end
