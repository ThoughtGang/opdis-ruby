#!/usr/bin/env ruby

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
      ip = (regs.include? 'rip') ? regs['rip'] : regs['eip']
      puts ("DEBUGGER STEP %d: %X in %d" % [i, ip, tgt.pid])

      v = tgt.text.peek(ip)
      bytes = [v, v >> 8, v >> 12, v >> 16].map { |byte| byte & 0xFF }
      puts "    BYTES AT EIP: #{bytes.map{ |byte| "%02X" % byte }.join(' ')}"

      esp_name = (regs.include? 'rsp') ? 'rsp' : 'esp'
      v = tgt.data.peek(regs[esp_name])
      bytes = [v, v >> 8, v >> 12, v >> 16].map { |byte| byte & 0xFF }
      puts "    BYTES AT ESP: #{bytes.map{ |byte| "%02X" % byte }.join(' ')}"

      puts "    ...WRITING ESP..."
      tgt.data.poke(regs[esp_name], v + 0x100)

      regs = tgt.regs.read
      v = tgt.data.peek(regs[esp_name])
      bytes = [v, v >> 8, v >> 12, v >> 16].map { |byte| byte & 0xFF }
      puts "    BYTES AT EBP: #{bytes.map{ |byte| "%02X" % byte }.join(' ')}"
    rescue Exception => e
      puts e.message
      puts e.backtrace.join("\n")
    end
  end
  puts "DEBUGGER RESUME"
  tgt.cont
end
