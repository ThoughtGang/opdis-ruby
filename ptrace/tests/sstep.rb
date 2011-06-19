#!/usr/bin/env ruby

require 'Ptrace'

if __FILE__ == $0

  cmd = ARGV.join(' ')

  tgt = Ptrace::Target.launch cmd
  puts "launched CMD #{tgt.pid}"
  # TODO
  #tgt.kill
  tgt.detach
  Process.wait
end
