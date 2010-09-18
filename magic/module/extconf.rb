#!/usrbin/ruby1.9

require 'mkmf'

dir_config('magic')
have_library('magic', 'magic_file')
find_header('magic.h' ) or raise "magic.h not found"

if RUBY_VERSION =~ /1.8/ then
        $CPPFLAGS += " -DRUBY_18"
elsif RUBY_VERSION =~ /1.9/ then
        $CPPFLAGS += " -DRUBY_19"
end

create_makefile('MagicExt')

