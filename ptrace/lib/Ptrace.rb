#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to Ptrace module

require 'Ptraceext'            # Load C extension wrapping libbfd.so

=begin rdoc
=end
module Ptrace


=begin rdoc
=end
  # all ptrace options
  class Options
  end

=begin rdoc
=end
  class MemArea
=begin rdoc
=end
    MEM_USER = 0
=begin rdoc
=end
    MEM_TEXT = 0
=begin rdoc
=end
    MEM_DATA = 0

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
    end

=begin rdoc
=end
    def peek
      # get command based on type
    end

=begin rdoc
=end
    def poke
      # get command based on type
    end
  end

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
    end

=begin rdoc
=end
    # read a signal from the child. returns a Signal data struct.
    def self.get(pid)
      # send message
      # Signal.new
    end

  end

=begin rdoc
=end
# NOTE: this is the only real problem area. Where to parse regs for platform?
  class RegSet
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
      # send read message to PID
    end

=begin rdoc
=end
    def write
      # send message
    end
  end

=begin rdoc
=end
  class Target

=begin rdoc
=end
    attr_accessor :pid
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
    def initialize(pid)
      @pid = pid
      @text = MemArea.new(MEM_TEXT, pid)
      @data = MemArea.new(MEM_DATA, pid)
      @user = MemArea.new(MEM_USER, pid)
    end

=begin rdoc
=end
    def self.attach(pid)
      # return target.new
      # send attach
    end

=begin rdoc
=end
    def self.launch(cmd)
      # fork/exec
      # return target.new
      # send traceme
    end

=begin rdoc
=end
    # info
    def regs
      RegSet.new(TargetRegs::GEN, pid)
    end

=begin rdoc
=end
    def fpregs
      RegSet.new(TargetRegs::FP, pid)
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
    end

=begin rdoc
=end
    # actions
    def kill
    end

=begin rdoc
=end
    def detach
    end

=begin rdoc
=end
    def step
    end

=begin rdoc
=end
    def syscall
    end

=begin rdoc
=end
    def cont
    end

=begin rdoc
=end
    def sysemu
    end

=begin rdoc
=end
    def sysemu_step
    end

=begin rdoc
=end
    # misc
    def options=
    end

  end
end
