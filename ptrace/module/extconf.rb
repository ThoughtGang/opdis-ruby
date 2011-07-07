#!/usrbin/ruby1.9

require 'mkmf'

if RUBY_VERSION =~ /1.8/ then
        $CPPFLAGS += " -DRUBY_18"
elsif RUBY_VERSION =~ /1.9/ then
        $CPPFLAGS += " -DRUBY_19"
end

create_makefile('Ptrace_ext')
