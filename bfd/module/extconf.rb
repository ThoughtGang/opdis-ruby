#!/usrbin/ruby1.9

require 'mkmf'

have_library('bfd', 'bfd_init')

if RUBY_VERSION =~ /1.8/ then
        $CPPFLAGS += " -DRUBY_18"
elsif RUBY_VERSION =~ /1.9/ then
        $CPPFLAGS += " -DRUBY_19"
end

create_makefile('BFDext')

