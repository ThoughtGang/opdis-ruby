#!/usrbin/ruby1.9

require 'mkmf'

dir_config('opdis')

have_library('bfd', 'bfd_init')
have_library('opcodes', 'init_disassemble_info')
have_library('opdis', 'opdis_init')

create_makefile('opdis')
