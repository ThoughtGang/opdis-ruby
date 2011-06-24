#!/usr/bin/env ruby

require 'Ptrace'

if __FILE__ == $0

  cmd = ARGV.join(' ')

  tgt = Ptrace::Target.launch cmd
  puts "launched CMD #{tgt.pid}"
  10.times do |i|
    puts "DEBUGGER STEP #{i}"
    tgt.step
  end
  puts "DEBUGGER RESUME"
  tgt.cont

end
