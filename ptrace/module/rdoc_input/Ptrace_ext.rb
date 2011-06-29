#!/usr/bin/env ruby
# :title: Ptrace
=begin rdoc
=Ptrace
<i>Copyright 2011 Thoughtgang <http://www.thoughtgang.org></i>

= Ptrace wrapper.

All of the original ptrace data structures and constants can be found in
<b>ptrace.h</b>, available on Linux installations at 
<b>/usr/include/sys/ptrace.h</b>.

== Summary
A wrapper for the ptrace debugging facility.

== Example
require 'Ptrace_ext'

== Disclaimer

== Contact
Support:: community@thoughtgang.org
Project:: http://rubyforge.org/projects/opdis/
=end


=begin rdoc
Ptrace support.
=end
module Ptrace

=begin rdoc
A ptrace debugger. Note that this class is not instantiated; it is simply
a wrapper for the ptrace system call that performs translation of objects
between Ruby and C.
=end
  class Debugger

=begin rdoc
Return a Hash mapping command symbols (e.g. :singlestep) to ptrace command
numbers. This serves as a list of all ptrace commands supported by the OS.
=end
    def self.commands
    end

=begin rdoc
Send a ptrace command number to the specified PID. The addr argument can be
nil. This is a wrapper for the ptrace system call.
=end
    def self.send_cmd(cmd, pid, addr)
    end

=begin rdoc
Send a ptrace command number to the specified PID, using addr and data as
the address and data arguments. This is a wrapper for the ptrace system call.
=end
    def self.send_data(cmd, pid, addr, data)
    end

=begin rdoc
Read a word from memory at the specified address in the specified process. The
cms is expected to be one of PT_READ_I, PT_READ_D, or PT_READ_U.
=end
    def self.peek(cmd, pid, addr)
    end

=begin rdoc
Write a word to memory at the specified address in the specified process. The
cms is expected to be one of PT_WRITE_I, PT_WRITE_D, or PT_WRITE_U.
=end
    def self.poke(cmd, pid, addr, word)
    end

=begin rdoc
Read the general (CPU) registers for the specified process. This returns a Hash
mapping register name to value.
=end
    def self.regs(pid)
    end

=begin rdoc
Write the general (CPU) registers for the specified process. The Hash must be
a complete set of registers as returned by Debugger.regs; missing registers
will be set to 0.
=end
    def self.regs=(pid, hash)
    end

=begin rdoc
Read the FPU registers for the specified process. This returns a Hash
mapping register name to value.
=end
    def self.fpregs(pid)
    end

=begin rdoc
Write the FPU registers for the specified process. The Hash must be
a complete set of registers as returned by Debugger.fpregs; missing registers
will be set to 0.
=end
    def self.fpregs=(pid, hash)
    end

=begin rdoc
=end
    def self.signal(pid)
    end

=begin rdoc
=end
    def self.signal=(pid, hash)
    end

=begin rdoc
=end
    def self.event_msg(pid)
    end
  end

end
