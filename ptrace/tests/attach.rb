#!/usr/bin/env ruby

require 'Ptrace'

if __FILE__ == $0

  pid = ARGV.pop.to_i

  tgt = Ptrace::Target.attach(pid)
  puts "Attached to CMD #{tgt.pid}"
  puts "KILLING #{tgt.pid}"
  tgt.kill

  Process.wait
end
