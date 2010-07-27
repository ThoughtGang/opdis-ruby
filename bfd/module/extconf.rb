#!/usrbin/ruby1.9

require 'mkmf'

have_library('bfd', 'bfd_init')
create_makefile('BFDext')

