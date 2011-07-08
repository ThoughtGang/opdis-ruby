#!/usr/bin/env ruby


File.open( ARGV.pop ) do |f|

  f.lines.each do |line|
    tok = line.chomp
    syscall = tok
    if tok =~ /SYS_+([^_]+.*$)/
      syscall = $1
    end
    puts "#ifdef #{tok}"
    puts "\tCMD_HASH_ADD(h, \"#{syscall}\", #{tok});"
    puts "#endif"
  end
end
