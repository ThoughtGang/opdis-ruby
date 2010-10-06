#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Print supported architectures to STDOUT

require 'Opdis'

if __FILE__ == $0

  Opdis::Disassembler.architectures.each { |arch| puts arch }

end
