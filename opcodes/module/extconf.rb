#!/usrbin/ruby1.9

require 'mkmf'

have_library('opcodes', 'init_disassemble_info')
dir_config('opcodes')

if RUBY_VERSION =~ /1.8/ then
      $CPPFLAGS += " -DRUBY_18"
elsif RUBY_VERSION =~ /1.9/ then
      $CPPFLAGS += " -DRUBY_19"
end

create_makefile('OpcodesExt')
