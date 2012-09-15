import os, sys, shutil, string
import platform  as syspl

# Helper: "normalize" a name to make it a suitable C macro name
def cnorm( name ):
  name = name.replace( '-', '' )
  name = name.replace( ' ', '' )
  return name.upper()

# List of toolchains
toolchain_list = {
  'arm-gcc' : { 
    'compile' : 'arm-elf-gcc', 
    'link' : 'arm-elf-ld', 
    'asm' : 'arm-elf-as', 
    'bin' : 'arm-elf-objcopy', 
    'size' : 'arm-elf-size',
    'cross_cpumode' : 'little',
    'cross_fp' : 'float_arm 64',
    'cross_nofp' : 'int 32'
  },
  'arm-eabi-gcc' : {
    'compile' : 'arm-eabi-gcc',
    'link' : 'arm-eabi-ld',
    'asm' : 'arm-eabi-as',
    'bin' : 'arm-eabi-objcopy',
    'size' : 'arm-eabi-size',
    'cross_cpumode' : 'little',
    'cross_fp' : 'float 64',
    'cross_nofp' : 'int 32'
  },
  'codesourcery' : { 
    'compile' : 'arm-none-eabi-gcc', 
    'link' : 'arm-none-eabi-ld', 
    'asm' : 'arm-none-eabi-as', 
    'bin' : 'arm-none-eabi-objcopy', 
    'size' : 'arm-none-eabi-size',
    'cross_cpumode' : 'little',
    'cross_fp' : 'float 64',
    'cross_nofp' : 'int 32'
  },
  'i686-gcc' : { 
    'compile' : 'i686-elf-gcc', 
    'link' : 'i686-elf-ld', 
    'asm' : 'nasm', 
    'bin' : 'i686-elf-objcopy', 
    'size' : 'i686-elf-size',
    'cross_cpumode' : 'little',
    'cross_fp' : 'float 64',
    'cross_nofp' : 'int 32'
  }
}

# Toolchain Aliases
toolchain_list['devkitarm'] = toolchain_list['arm-eabi-gcc']

# List of platform/CPU/toolchains combinations
# The first toolchain in the toolchains list is the default one
# (the one that will be used if none is specified)
platform_list = {  
  'lm3s' : { 'cpus' : [ 'LM3S1968', 'LM3S8962', 'LM3S6965', 'LM3S6918', 'LM3S9B92' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] },
  'i386' : { 'cpus' : [ 'I386' ], 'toolchains' : [ 'i686-gcc' ] },
  'stm32' : { 'cpus' : [ 'STM32F103ZE', 'STM32F103RE' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ] }
}

# List of board/CPU combinations
board_list = { 'EK-LM3S8962' : [ 'LM3S8962' ],
               'EK-LM3S1968' : [ 'LM3S1968' ],
               'EK-LM3S6965' : [ 'LM3S6965' ],
               'EK-LM3S9B92' : [ 'LM3S9B92' ],
               'PC' : [ 'I386' ],
               'ET-STM32' : [ 'STM32F103RE' ],
               'EAGLE-100' : [ 'LM3S6918' ],
               'STM3210E-EVAL' : [ 'STM32F103ZE' ],
             }

cpu_list = sum([board_list[i] for i in board_list],[])

# ROMFS file list "groups"
# To include a file in a ROMFS build, include it in a group here (or create one
# if you need) and make sure the group is included on your platform's file_list
# definition (right after this).

# The following table will be left here just as an example
#romfs = { 'adcpoll' : [ 'adcpoll.c' ],
#          'hello' : [ 'hello.c' ],
#          'morse' : [ 'morse.c' ]
#}

romfs = { 'adcpoll' : [ 'adcpoll.c' ],
	  'hello' : [ 'hello.c' ],
	  'info' : [ 'info.c' ],
	  'morse' : [ 'morse.c' ]
}

# List of board/romfs data combinations
# The following table will be left here just as an example
#file_list = { 'EK-LM3S8962' : [ 'adcpoll', 'hello', 'morse.c' ],
#              'EK-LM3S1968' : [ 'adcpoll', 'hello' ],
#              'EK-LM3S6965' : [ 'adcpoll', 'hello', 'morse.c' ],
#              'EK-LM3S9B92' : [ 'adcpoll', 'hello' ],
#              'PC' : [ 'hello' ],
#              'ET-STM32' : [ 'adcpoll', 'hello' ],
#              'EAGLE-100' : [ 'adcpoll', 'hello' ],
#              'STM3210E-EVAL' : [ 'adcpoll', 'hello' ],
#}

file_list = { 'EK-LM3S8962' : [ ],
              'EK-LM3S1968' : [ ],
              'EK-LM3S6965' : [ ],
              'EK-LM3S9B92' : [ ],
              'PC' : [ 'hello', 'info' ],
              'ET-STM32' : [ ],
              'EAGLE-100' : [ ],
	      'STM3210E-EVAL' : [ ],
}

comp = Environment( tools = [],
                    OBJSUFFIX = ".o",
                    PROGSUFFIX = ".elf",
                    ENV = os.environ,
                    CPPDEFINES = {} )

if comp['PLATFORM'] == 'win32':
  Tool('mingw')(comp)
else:
  Tool('default')(comp)

# Replacement for standard EnumVariable functionality to derive case from original list
class InsensitiveString(object):
  def __init__(self, s):
    self.s = s
  def __cmp__(self, other):
    return cmp(self.s.lower(), other.lower())

def _validator(key, val, env, vals): 
  if not val in vals: 
    raise SCons.Errors.UserError( 
      'Invalid value for option %s: %s' % (key, val)) 

def MatchEnumVariable(key, help, default, allowed_values, map={}):
  help = '%s (%s)' % (help, string.join(allowed_values, '|'))
  
  validator = lambda key, val, env, vals=allowed_values: \
              _validator(key, InsensitiveString(val), env, vals)

  converter = lambda val, map=map: \
              map.get(val, allowed_values[allowed_values.index(InsensitiveString(val))])
  
  return (key, help, default, validator, converter) 


# Add Configurable Variables
vars = Variables()

vars.AddVariables(
  MatchEnumVariable('target',
                    'build "regular" float picoc or integer-only', 
                    'fp',
                    allowed_values = [ 'fp', 'nofp' ] ),
  MatchEnumVariable('cpu',
                    'build for the specified CPU (board will be inferred, if possible)',
                    'auto',
                    allowed_values = cpu_list + [ 'auto' ] ),
  MatchEnumVariable('allocator',
                    'select memory allocator',
                    'auto',
                    allowed_values=[ 'newlib', 'multiple', 'simple', 'auto' ] ),
  MatchEnumVariable('board',
                    'selects board for target (cpu will be inferred)',
                    'auto',
                    allowed_values=board_list.keys() + [ 'auto' ] ),
  MatchEnumVariable('toolchain',
                    'specifies toolchain to use (auto=search for usable toolchain)',
                    'auto',
                    allowed_values=toolchain_list.keys() + [ 'auto' ] ),
  BoolVariable(     'optram',
                    'enables Tiny RAM enhancements',
                    True ),
  MatchEnumVariable('boot',
                    'boot mode, standard will boot to shell',
                    'standard',
                    allowed_values=[ 'standard' ] ),
  MatchEnumVariable('romfs',
                    'ROMFS compilation mode',
                    'verbatim',
                    allowed_values=[ 'verbatim' ] ) )

vars.Update(comp)

if not GetOption( 'help' ):

  conf = Configure(comp)

  # Variants: board = <board>
  #           cpu = <cpuname>
  #           board = <board> cpu=<cpuname>
  if comp['board'] == 'auto' and comp['cpu'] == 'auto':
    print "Must specifiy board, cpu, or both"
    Exit( -1 )
  elif comp['board'] != 'auto' and comp['cpu'] != 'auto':
    # Check if the board, cpu pair is correct
    if not comp['cpu'] in board_list[ comp['board'] ]:
      print "Invalid CPU %s for board %s" % ( comp['cpu'], comp['board'] )
      Exit( -1 )
  elif comp['board'] != 'auto':
    # Find CPU
    comp['cpu'] = board_list[ comp['board'] ][ 0 ]
  else:
    # cpu = <cputype>
    # Find board name
    for b, v in board_list.items():
      if comp['cpu'] in v:
        comp['board'] = b
        break
    else:
      print "CPU %s not found" % comp['cpu']
      Exit( -1 )

  # Look for the given CPU in the list of platforms
  platform = None
  for p, v in platform_list.items():
    if comp['cpu'] in v[ 'cpus' ]:
      platform = p
      break
  else:
    print "Unknown CPU %s" % comp['cpu']
    print "List of accepted CPUs: "
    for p, v in platform_list.items():
      print " ", p, "-->",
      for cpu in v[ 'cpus' ]:
        print cpu,
      print
    Exit( -1 )

  # Check the toolchain
  if comp['toolchain'] != 'auto':
    if not comp['toolchain'] in platform_list[ platform ][ 'toolchains' ]:
      print "Invalid toolchain '%s' for CPU '%s'" % ( comp['toolchain'], comp['cpu'] )
      Exit( -1 )
    toolset = toolchain_list[ comp['toolchain'] ]
    comp[ 'CC' ] = toolset[ 'compile' ]
    comp[ 'AS' ] = toolset[ 'compile' ]
  else:
    # if 'auto' try to match a working toolchain with target
    usable_chains = [toolchain_list[ toolchain ][ 'compile' ] for toolchain in platform_list[ platform ]['toolchains']]
    comp['CC'] = comp.Detect( usable_chains )
    if comp['CC']:
        comp['toolchain'] =  platform_list[ platform ]['toolchains'][usable_chains.index(comp['CC'])]
        comp['AS'] = comp['CC']
        toolset = toolchain_list[ comp['toolchain'] ]
    else:
      print "Unable to find usable toolchain in your path."
      print "List of accepted toolchains (for %s):" % ( comp['cpu'] )
      print ', '.join(usable_chains)
      Exit( -1 )

    if not conf.CheckCC():
      print "Test compile failed with selected toolchain: %s" % (comp['toolchain'])
      Exit( -1 )

  # CPU/allocator mapping (if allocator not specified)
  if comp['allocator'] == 'auto':
    comp['allocator'] = 'newlib'

  # Variable for romfs
  compcmd = ''

  # User report
  if not GetOption( 'clean' ):
    print
    print "*********************************"
    print "Compiling remix ..."
    print "CPU:            ", comp['cpu']
    print "Board:          ", comp['board']
    print "Platform:       ", platform
    print "Allocator:      ", comp['allocator']
    print "Boot Mode:      ", comp['boot']
    print "Target:         ", comp['target']
    print "Toolchain:      ", comp['toolchain']
    print "ROMFS mode:     ", comp['romfs']
    print "*********************************"
    print

  output = 'picoc_remix_' + comp['target'] + '_' + comp['cpu'].lower()

  comp.Append(CPPDEFINES = { 'PICOC_CPU' : comp['cpu'],
                             'PICOC_BOARD' : comp['board'],
                             'PICOC_PLATFORM' : platform.upper() } )
  comp.Append(CPPDEFINES = {'__BUFSIZ__' : 128})

  # Also make the above into direct defines (to use in conditional C code)
  conf.env.Append(CPPDEFINES = ["PICOC_CPU_" + cnorm( comp['cpu'] ), "PICOC_BOARD_" + cnorm( comp['board'] ), "PICOC_PLATFORM_" +  cnorm( platform )])

  if comp['allocator'] == 'multiple':
     conf.env.Append(CPPDEFINES = ['USE_MULTIPLE_ALLOCATOR'])
  elif comp['allocator'] == 'simple':
     conf.env.Append(CPPDEFINES = ['USE_SIMPLE_ALLOCATOR'])

  # picoc source files and include path
  picoc_files = """picoc.c table.c lex.c parse.c expression.c heap.c type.c variable.c platform.c clibrary.c include.c
    cstdlib/stdio.c cstdlib/math.c cstdlib/string.c cstdlib/stdlib.c cstdlib/errno.c cstdlib/ctype.c
    cstdlib/stdbool.c platform/platform_unix.c platform/library_unix.c rotable.c"""

  picoc_full_files = " " + " ".join( [ "src/picoc/%s" % name for name in picoc_files.split() ] )
  
  comp.Append(CPPPATH = ['inc', 'inc/newlib', 'src/platform', 'src/picoc'])
  if comp['target'] == 'nofp':
    conf.env.Append(CPPDEFINES = ['NO_FP'])

  conf.env.Append(CPPPATH = ['src/modules', 'src/platform/%s' % platform])
  conf.env.Append(CPPDEFINES = {"PICOC_OPTIMIZE_MEMORY" : ( comp['optram'] != 0 and 2 or 0 ) } )
  if comp['optram'] == 0:
    conf.env.Append(CPPDEFINES = ['BUILTIN_MINI_STDLIB'])
    conf.env.Append(CPPDEFINES = ['PICOC_LIBRARY'])

  # Additional libraries
  local_libs = ''

  # Application files
  app_files = """ src/main.c src/romfs.c src/semifs.c src/xmodem.c src/shell.c src/term.c src/common.c src/common_tmr.c src/buf.c
                  src/salloc.c src/picoc_int.c src/linenoise.c src/common_uart.c src/picoc_adc.c src/dlmalloc.c """

  # Newlib related files
  newlib_files = " src/newlib/devman.c src/newlib/stubs.c src/newlib/genstd.c src/newlib/stdtcp.c"

  # FatFs files
  app_files = app_files + "src/picoc_mmc.c src/mmcfs.c src/fatfs/ff.c src/fatfs/ccsbcs.c "
  comp.Append(CPPPATH = ['src/fatfs'])

  # Module files
  module_names = "pd.c pwm.c can.c tmr.c term.c adc.c pio.c i2c.c picoc.c spi.c uart.c cpu.c"
  module_files = " " + " ".join( [ "src/modules/%s" % name for name in module_names.split() ] )

  # Optimizer flags (speed or size)
  comp.Append(CCFLAGS = ['-Os','-fomit-frame-pointer'])
  #opt += " -ffreestanding"
  #opt += " -fconserve-stack" # conserve stack at potential speed cost, >=GCC4.4

  # Toolset data (filled by each platform in part)
  tools = {}

  # We get platform-specific data by executing the platform script
  execfile( "src/platform/%s/conf.py" % platform )

  # Complete file list
  source_files = Split( app_files + specific_files + newlib_files + picoc_full_files + module_files )
  
  comp = conf.Finish()

  # Make ROM File System first
  if not GetOption( 'clean' ):
    print "Building ROM File System..."
    romdir = "romfs"
    flist = []
    for sample in file_list[ comp['board'] ]:
      flist += romfs[ sample ]
    import mkfs
    mkfs.mkfs( romdir, "romfiles", flist, comp['romfs'], compcmd )
    print
    if os.path.exists( "inc/romfiles.h" ): 
      os.remove( "inc/romfiles.h" )
    shutil.move( "romfiles.h", "inc/" )
    if os.path.exists( "src/fs.o" ): 
      os.remove( "src/fs.o" )

  # comp.TargetSignatures( 'content' )
  # comp.SourceSignatures( 'MD5' )
  comp[ 'INCPREFIX' ] = "-I"
  Default( comp.Program( target = output, source = source_files ) )
  Decider( 'MD5-timestamp' )

  # Programming target
  prog = Environment( BUILDERS = { 'program' : Builder( action = Action ( tools[ platform ][ 'progfunc' ] ) ) }, ENV = os.environ )
  prog.program( "prog", output + ".elf" )

Help(vars.GenerateHelpText(comp))
