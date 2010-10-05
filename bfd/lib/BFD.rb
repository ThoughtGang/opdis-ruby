#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to BFD module

require 'BFDext'            # Load C extension wrapping libbfd.so
require 'tempfile'          # For buffer target

# TODO: inspect() methods for these classes

module Bfd

  class Target
=begin rdoc
BFD file format.
Defined in /usr/include/bfd.h : enum bfd_flavour
=end
    FLAVOURS = %w{ unknown aout coff ecoff xcoff elf ieee nlm oasys tekhex srec
                   verilog ihex som os0k versados msdos ovax evax mmo mach_o
                   pef pef_xlib sym }
             
=begin rdoc
Byte order.
Defined in /usr/include/bfd.h : enum bfd_endian
=end
    ENDIAN = %w{ big little unknown }

=begin rdoc
File Format Flags.
Defined in /usr/include/bfd.h : struct bfd 
=end
    FLAGS = { 0x0001 => 'HAS_RELOC',
              0x0002 => 'EXEC_P',
              0x0004 => 'LINEN',
              0x0008 => 'DEBUG',
              0x0010 => 'SYMS',
              0x0020 => 'LOCALS',
              0x0040 => 'DYNAMIC',
              0x0080 => 'WP_TEXT',
              0x0100 => 'D_PAGED',
              0x0200 => 'IS_RELAXABLE',
              0x0400 => 'TRADITIONAL_FORMAT',
              0x0800 => 'IN_MEMORY',
              0x1000 => 'HAS_LOAD_PAGE',
              0x2000 => 'LINKER_CREATED',
              0x4000 => 'DETERMINISTIC_OUTPUT' }

=begin rdoc
Temporary file used if Target is instantiated from a buffer. BFD does not
operate on memory locations and requires a file descriptor; a temporary
file is therefore created on the filesystem when Target.from_buffer is
called. The temp file is deleted when Target.close is called.
=end
    attr_accessor :temp_file

=begin rdoc
Create a new Target from a path or IO object. This just wraps for ext_new
and provides a default value for args.
NOTE: target should either be the path to a file or an IO object with a valid 
read-only (i.e. opened with 'rb' flags) file descriptor returned by fileno(). 
File descriptors opened for write ('wb') will be rejected by libbfd.
=end
    def self.new(target, args={})
      bfd = ext_new(target, args)
      yield bfd if (bfd and block_given?)
      return bfd
    end

=begin rdoc
Instantiate target from a buffer instead of from a file. Note: this creates
a temp_file which MUST be freed by calling Target.close, or by passing a 
block to this method.
=end
    def self.from_buffer(buf, args={})

      f = Tempfile.new( 'bfd_target' )
      f.write(buf)

      bfd = ext_new(f.path, args)
      raise "Unable to construct BFD" if not bfd

      if not block_given?
        @temp_file = f
        return bfd
      end

      # yield bfd object, then close temp file
      yield bfd
      f.close
      nil
    end

=begin rdoc
Free any resources used by BFD Target
=end
    def close
      if @temp_file
        @temp_file.close
        @temp_file = nil
      end
    end

=begin rdoc
Return an array of the names of bit-flags that are set.
=end
    def flag_strings(flags)
      f = []
      FLAGS.each { |k,v| f << v if (flags & k > 0) }
      return f
    end

=begin rdoc
Return an array of the names of the file format flags that are set.
See raw_format_flags.
=end
    def format_flags()
      flag_strings(@raw_format_flags)
    end

=begin rdoc
Return an array of the names of the target type flags that are set.
See raw_type_flags.
=end
    def type_flags()
      flag_strings(@raw_type_flags)
    end

=begin rdoc
Return the target file format.
See raw_flavour.
=end
    def flavour
      FLAVOURS[@raw_flavour]
    end

=begin rdoc
Return the byte order of the target.
See raw_endian.
=end
    def endian
      ENDIAN[@raw_endian]
    end


=begin rdoc
Return the Bfd::Section in the target that contains <i>vma</i>, or <i>nil</i>.
=end
    def section_for_vma(vma)
      @sections.values.each do |s|
        return s if (vma >= s.vma and vma < (s.vma + s.size))
      end

      return nil
    end
  end

  class Section

=begin rdoc
Section flags.
Defined in /usr/include/bfd.h : typedef struct bfd_section
=end
    FLAGS={ 0x00000001 => 'ALLOC',
            0x00000002 => 'LOAD',
            0x00000004 => 'RELOC',
            0x00000008 => 'READONLY',
            0x00000010 => 'CODE',
            0x00000020 => 'DATA',
            0x00000040 => 'ROM',
            0x00000080 => 'CONSTRUCTOR',
            0x00000100 => 'HAS_CONTENTS',
            0x00000200 => 'NEVER_LOAD',
            0x00000400 => 'THREAD_LOCAL',
            0x00000800 => 'HAS_GOT_REF',
            0x00001000 => 'IS_COMMON',
            0x00002000 => 'DEBUGGING',
            0x00004000 => 'IN_MEMORY',
            0x00008000 => 'EXCLUDE',
            0x00010000 => 'SORT_ENTRIES',
            0x00020000 => 'LINK_ONCE',
            0x00040000 => 'LINK_DUPLICATES_ONE_ONLY',
            0x00080000 => 'LINK_DUPLICATES_SAME_SIZE',
            0x00100000 => 'LINKER_CREATED',
            0x00200000 => 'KEEP',
            0x00400000 => 'SMALL_DATA',
            0x00800000 => 'MERGE',
            0x01000000 => 'STRINGS',
            0x02000000 => 'GROUP',
            0x04000000 => 'COFF_SHARED_LIBRARY',
            0x08000000 => 'COFF_SHARED',
            0x10000000 => 'TIC54X_BLOCK',
            0x20000000 => 'TIC54X_CLINK',
            0x40000000 => 'COFF_NOREAD' 
    }

=begin rdoc
Return an array of the names of the bit-flags that are set in Section.
See raw_flags.
=end
    def flags()
      f = []
      FLAGS.each { |k,v| f << v if (@raw_flags & k > 0) }
      return f
    end

  end

  class Symbol
    
=begin rdoc
Symbol flags.
Defined in /usr/include/bfd.h : typedef struct bfd_symbol
=end
    FLAGS={ 0x000001 => 'LOCAL',
            0x000002 => 'GLOBAL',
            0x000004 => 'DEBUGGING',
            0x000008 => 'FUNCTION',
            0x000020 => 'KEEP',
            0x000040 => 'KEEP_G',
            0x000080 => 'WEAK',
            0x000100 => 'SECTION_SYM',
            0x000200 => 'OLD_COMMON',
            0x000400 => 'NOT_AT_END',
            0x000800 => 'CONSTRUCTOR ',
            0x001000 => 'WARNING',
            0x002000 => 'INDIRECT',
            0x004000 => 'FILE',
            0x008000 => 'DYNAMIC',
            0x010000 => 'OBJECT',
            0x020000 => 'DEBUGGING_RELOC',
            0x040000 => 'THREAD_LOCAL',
            0x080000 => 'RELC',
            0x100000 => 'SRELC',
            0x200000 => 'SYNTHETIC',
            0x400000 => 'GNU_INDIRECT_FUNCTION',
            0x800000 => 'GNU_UNIQUE' 
    }

=begin rdoc
Return an array of the names of the bit-flags that are set in Symbol.
See raw_flags.
=end
    def flags()
      f = []
      FLAGS.each { |k,v| f << v if (@raw_flags & k > 0) }
      return f
    end

  end

end

