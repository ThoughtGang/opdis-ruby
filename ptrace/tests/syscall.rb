#!/usr/bin/env ruby

require 'Ptrace'

if __FILE__ == $0

  cmd = ARGV.join(' ')

  tgt = Ptrace::Target.launch cmd
  puts "launched CMD #{tgt.pid}"
  cont = true
  while cont

    begin
      # test PT_SYSCALL directly
      # call
      tgt.syscall
      puts "IN: #{tgt.regs.read.inspect}"

      # ret
      tgt.syscall
      puts "OUT: #{tgt.regs.read.inspect}"

      # test syscall wrapper
      state = tgt.syscall_state
      puts state.inspect
    rescue Ptrace::InvalidProcessError
      cont = false
    end

  end

end
