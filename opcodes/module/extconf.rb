#!/usrbin/ruby1.9

require 'mkmf'

have_library('opcodes', 'init_disassemble_info')
dir_config('opcodes')
create_makefile('OpcodesExt')
