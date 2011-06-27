#!/usr/bin/env ruby
# Copyright 2011 Thoughtgang <http://www.thoughtgang.org>
# A demonstration of a single-step debugger

require 'optparse'
require 'ostruct'

require 'curses'
include Curses

require 'Ptrace'
require 'Opcodes'

# ----------------------------------------------------------------------
def unpack_bytes(int)
  [int, (int >> 8), (int >> 16), (int >> 24)].map { |byte| byte & 0xFF }
end

def print_state(tgt, dis)
	setpos(0,0)
  count = 0

  # register set
  regs = tgt.regs.read
  regs.keys.sort.each do |name|
    addstr "%8s: %016X  " % [name, regs[name]]
    count += 1
    if count >= 2
      count = 0
      addstr "\n"
    end
  end
  addstr "\n"

  # current insn
  ip = (regs.include? 'rip') ? regs['rip'] : regs['eip']
  v1 = tgt.text.peek(ip)
  v2 = tgt.text.peek(ip + 4)
  v3 = tgt.text.peek(ip + 8)
  v4 = tgt.text.peek(ip + 12)
  bytes = unpack_bytes(v1)
  bytes.concat unpack_bytes(v2)
  bytes.concat unpack_bytes(v3)
  bytes.concat unpack_bytes(v4)
  insns = dis.disasm(bytes.pack('C*'))
  addstr "\n%016X\t%s\n" % [ip, insns.first[:insn].join]
  
  addstr "\nPress space to step, c to continue\n"
end

# ----------------------------------------------------------------------
def get_options(args)
  options = OpenStruct.new
  opts_str = 'TARGET'

  options.target = nil

   opts = OptionParser.new do |opts|
     opts.banner = "Usage: stepper #{opts_str}"
     opts.separator 'Options:'

     opts.on_tail( '-?', '--help', 'Show this message') do
      puts opts
      exit 1
    end

   end

   opts.parse!(args)

   options.target = args.count > 0 ? args.join(' ') : nil
   options
end

# ----------------------------------------------------------------------
def main(opts)
	init_screen
	noecho
	trap(0) { echo }

  tgt = Ptrace::Target.launch(opts.target)
  if not tgt
    puts "Unable to launch #{opts.target}"
    return
  end

  disasm = Opcodes::Disassembler.new(:arch => 'x86_64')

#	while not (['c', 'q'].include?(getch))
  cmd = 0
  begin
    print_state(tgt, disasm)
    tgt.step
    cmd = getch
	end until (cmd == 99)

  tgt.cont
end

if __FILE__ == $0

  opts = get_options(ARGV)

  if ! opts.target
    $stderr.puts "No target specified"
    exit -1
  end


  main(opts)

end
