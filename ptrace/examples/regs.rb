#!/usr/bin/env ruby
# TODO : read, write regs

require 'Ptrace'

if __FILE__ == $0

  cmd = ARGV.join(' ')

  tgt = Ptrace::Target.launch cmd
  return if not tgt

  puts "launched CMD #{cmd} as #{tgt.pid}"
  10.times do |i|
  sleep 1
    begin
      tgt.step
      regs = tgt.regs.read
      ebx_name = (regs.include? 'rbx') ? 'rbx' : 'ebx'

      v = regs[ebx_name]
      puts "EBX ORIG : %016X" % v

      tgt.regs[ebx_name] = v + 0x1000
      tgt.regs.write

      regs = tgt.regs.read
      v1 = regs[ebx_name]
      puts "EBX AFTER WRITE : %016X" % v1

      tgt.regs[ebx_name] = v
      tgt.regs.write

      regs = tgt.regs.read
      v2 = regs[ebx_name]
      puts "EBX AFTER REVERT : %016X" % v2

      # TODO: FPREGS

    rescue Exception => e
      puts e.message
      puts e.backtrace.join("\n")
    end
  end
  puts "DEBUGGER RESUME"
  tgt.cont
end
