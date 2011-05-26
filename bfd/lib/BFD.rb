#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Ruby additions to BFD module

require 'BFDext'            # Load C extension wrapping libbfd.so
require 'tempfile'          # For buffer target

# TODO: Support reloc, line no, debug
module Bfd

  class Target

=begin rdoc
core dump
=end
    FORMAT_CORE = 'core'
=begin rdoc
linkable (executable, shared library) or relocatable object file
=end
    FORMAT_OBJECT = 'object'
=begin rdoc
library archive (.ar)
=end
    FORMAT_ARCHIVE = 'archive'
=begin rdoc
unrecognized/unsupported file
=end
    FORMAT_UNKNOWN = 'unknown'
=begin rdoc
BFD object type
=end
    FORMATS = [ FORMAT_UNKNOWN, FORMAT_CORE, FORMAT_OBJECT, FORMAT_ARCHIVE ]

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

# Note: other interesting flags such as HAS_RELOC, HAS_DEBUG will be handled
#       by methods returning (empty?) collections of relocs/linenos/debug-syms.
=begin rdoc
=end
    FLAG_EXEC = 'EXEC_P'
=begin rdoc
=end
    FLAG_DYNAMIC = 'DYNAMIC'

=begin rdoc
File Format Flags.
Defined in /usr/include/bfd.h : struct bfd 
=end
    FLAGS = { 0x0001 => 'HAS_RELOC',
              0x0002 => FLAG_EXEC,
              0x0004 => 'LINEN',
              0x0008 => 'DEBUG',
              0x0010 => 'SYMS',
              0x0020 => 'LOCALS',
              0x0040 => FLAG_DYNAMIC,
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
Instantiate target from a buffer instead of from a file. 
Note: this creates a temporary file which MUST be closed and unlinked by 
calling Target.close, or by passing a block to this method.
=end
    def self.from_buffer(buf, args={})

      f = Tempfile.new( 'bfd_target' )
      path = f.path
      f.close

      f = File.open(path, 'wb')
      f.write(buf)
      f.rewind

      bfd = ext_new(path, args)
      raise "Unable to construct BFD" if not bfd

      if not block_given?
        @temp_file = f
        return bfd
      end

      # yield bfd object, then close temp file
      yield bfd
      f.close
      File.unlink(path)
      nil
    end

=begin rdoc
Free any resources used by BFD Target
=end
    def close
      if @temp_file
        path = @temp_file.path
        @temp_file.close
        File.unlink(path)
        @temp_file = nil
      end
    end

=begin rdoc
Return an array of the names of the file format flags that are set.
See raw_format_flags and type_flags.
=end
    def format_flags
      flag_strings(@raw_format_flags)
    end

=begin rdoc
Is Target a valid BFD object (i.e. did BFD successfully parse it)
=end
  def valid?
    @format != FORMAT_UNKNOWN
  end

=begin rdoc
Is target a standalone executable
=end
    def is_executable?
      @format == FORMAT_OBJECT and format_flags.include? FLAG_EXEC
    end

=begin rdoc
Is target a shared library file (.so)
=end
    def is_shared_object?
      @format == FORMAT_OBJECT and format_flags.include? FLAG_DYNAMIC and
                                   (not format_flags.include? FLAG_EXEC)
    end

=begin rdoc
Is target a relocatable object (not an executable or .so)
=end
    def is_relocatable_object?
      @format == FORMAT_OBJECT and (not format_flags.include? FLAG_DYNAMIC) and
                                   (not format_flags.include? FLAG_EXEC)
    end

=begin rdoc
Return an array of the names of the target type flags that are set.
Note: These are the 'backend' flags for the BFD target type, and seem
to be a list of the flags *available* for the target type, not the list
of flags which are *set*. The format_flags field should always be used to
determine the flags which are set for a BFD target.
See raw_type_flags and format_flags.
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

    def to_s
      "[#{@id}] #{@filename}"
    end

    def inspect
      "#{self.to_s} : #{self.flavour} #{@format} (#{@type} #{endian}-endian)"
    end

    private
=begin rdoc
Return an array of the names of bit-flags that are set.
=end
    def flag_strings(flags)
      f = []
      FLAGS.each { |k,v| f << v if (flags & k > 0) }
      return f
    end

  end

  class Section

=begin rdoc
From bfd.h:
'Tells the OS to allocate space for this section when loading.
 This is clear for a section containing debug information only.'
=end
    FLAG_ALLOC = 'ALLOC'
=begin rdoc
From bfd.h:
'Tells the OS to load the section from the file when loading.
This is clear for a .bss section.'
=end
    FLAG_LOAD = 'LOAD'
=begin rdoc
From bfd.h:
'The section contains data still to be relocated, so there is
some relocation information too.'
=end
    FLAG_RELOC = 'RELOC'
=begin rdoc
From bfd.h:
'A signal to the OS that the section contains read only data.'
=end
    FLAG_RO = 'READONLY'
=begin rdoc
From bfd.h:
'The section contains code only.'
=end
    FLAG_CODE = 'CODE'
=begin rdoc
From bfd.h:
'The section contains data only.'
=end
    FLAG_DATA = 'DATA'
=begin rdoc
From bfd.h:
'The section contains constructor information. This section
type is used by the linker to create lists of constructors and
destructors used by <<g++>>. When a back end sees a symbol
which should be used in a constructor list, it creates a new
section for the type of name (e.g., <<__CTOR_LIST__>>), attaches
the symbol to it, and builds a relocation. To build the lists
of constructors, all the linker has to do is catenate all the
sections called <<__CTOR_LIST__>> and relocate the data
contained within - exactly the operations it would peform on
standard data.'
=end
    FLAG_CTOR = 'CONSTRUCTOR'
=begin rdoc
From bfd.h:
'The section contains thread local data.'
=end
    FLAG_LOCAL = 'THREAD_LOCAL'

=begin rdoc
Section flags.
Defined in /usr/include/bfd.h : typedef struct bfd_section
=end
    FLAGS={ 0x00000001 => FLAG_ALLOC,
            0x00000002 => FLAG_LOAD,
            0x00000004 => FLAG_RELOC,
            0x00000008 => FLAG_RO,
            0x00000010 => FLAG_CODE,
            0x00000020 => FLAG_DATA,
            0x00000040 => 'ROM',
            0x00000080 => FLAG_CTOR,
            0x00000100 => 'HAS_CONTENTS',
            0x00000200 => 'NEVER_LOAD',
            0x00000400 => FLAG_LOCAL,
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

    def to_s
      @name
    end

    def inspect
      spec = "%X (%X), %X" % [ @vma, @file_pos, @size ]
      "[#{@id}] #{@name} #{spec}, #{flags.join('|')}"
    end
  end

  class Symbol
    
=begin rdoc
From bfd.h:
'The symbol has local scope; <<static>> in <<C>>. The value
is the offset into the section of the data.'
=end
    FLAG_LOCAL = 'LOCAL'
=begin rdoc
From bfd.h:
'The symbol has global scope; initialized data in <<C>>. The
value is the offset into the section of the data.'
=end
    FLAG_GLOBAL = 'GLOBAL'
=begin rdoc
From bfd.h:
'The symbol is a debugging record. The value has an arbitrary
meaning, unless BSF_DEBUGGING_RELOC is also set.'
=end
    FLAG_DEBUG = 'DEBUGGING'
=begin rdoc
From bfd.h:
'The symbol denotes a function entry point.  Used in ELF,
perhaps others someday.'
=end
    FLAG_FUNC = 'FUNCTION'
=begin rdoc
From bfd.h:
'A weak global symbol, overridable without warnings by
a regular global symbol of the same name.'
=end
    FLAG_WEAK = 'WEAK'
=begin rdoc
From bfd.h:
'This symbol was created to point to a section, e.g. ELF's
STT_SECTION symbols.'
=end
    FLAG_SEC = 'SECTION_SYM'
=begin rdoc
From bfd.h:
'Signal that the symbol is the label of constructor section.'
=end
    FLAG_CTOR = 'CONSTRUCTOR'
=begin rdoc
From bfd.h:
'Signal that the symbol is indirect.  This symbol is an indirect
pointer to the symbol with the same name as the next symbol.'
=end
    FLAG_INDIRECT = 'INDIRECT'
=begin rdoc
From bfd.h:
'BSF_FILE marks symbols that contain a file name.  This is used
for ELF STT_FILE symbols.'
=end
    FLAG_FILE = 'FILE'
=begin rdoc
From bfd.h:
'Symbol is from dynamic linking information.'
=end
    FLAG_DYNAMIC = 'DYNAMIC'
=begin rdoc
From bfd.h:
'The symbol denotes a data object.  Used in ELF, and perhaps
others someday.'
=end
    FLAG_OBJ = 'OBJECT'
=begin rdoc
From bfd.h:
'This symbol is thread local.  Used in ELF.'
=end
    FLAG_THREAD = 'THREAD_LOCAL'

=begin rdoc
Symbol flags.
Defined in /usr/include/bfd.h : typedef struct bfd_symbol.
From bfd.h:
'A normal C symbol would be one of:
     <<BSF_LOCAL>>, <<BSF_COMMON>>,  <<BSF_UNDEFINED>> or
     <<BSF_GLOBAL>>.'
=end
    FLAGS={ 0x000001 => FLAG_LOCAL,
            0x000002 => FLAG_GLOBAL,
            0x000004 => FLAG_DEBUG,
            0x000008 => FLAG_FUNC,
            0x000020 => 'KEEP',
            0x000040 => 'KEEP_G',
            0x000080 => FLAG_WEAK,
            0x000100 => FLAG_SEC,
            0x000200 => 'OLD_COMMON',
            0x000400 => 'NOT_AT_END',
            0x000800 => FLAG_CTOR,
            0x001000 => 'WARNING',
            0x002000 => FLAG_INDIRECT,
            0x004000 => FLAG_FILE,
            0x008000 => FLAG_DYNAMIC,
            0x010000 => FLAG_OBJ,
            0x020000 => 'DEBUGGING_RELOC',
            0x040000 => FLAG_THREAD,
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

    def to_s
      "#{@name} (#{@binding})"
    end

    def inspect
      "%s 0x%X %s" % [@name, @value, flags.join('|')]
    end

  end

end

