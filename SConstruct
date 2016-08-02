# -*- coding: utf-8 -*-

#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('env_default')

# Create a build environment for the ARM9 based netX chips.
env_arm9 = env_default.CreateEnvironment(['gcc-arm-none-eabi-4.7', 'asciidoc'])

# Create a build environment for the Cortex-R based netX chips.
env_cortex7 = env_default.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])


#----------------------------------------------------------------------------
#
# Create the compiler environments.
#
astrIncludePaths = ['src', '#platform/src', '#platform/src/lib', '#targets/version']


env_netx4000_default = env_cortex7.CreateCompilerEnv('4000', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])
env_netx4000_default.Append(CPPPATH = astrIncludePaths)
env_netx4000_default.Replace(BOOTBLOCK_CHIPTYPE = 4000)

env_netx56_default = env_arm9.CreateCompilerEnv('56', ['arch=armv5te'])
env_netx56_default.Append(CPPPATH = astrIncludePaths)
env_netx56_default.Replace(BOOTBLOCK_CHIPTYPE = 56)

Export('env_netx4000_default', 'env_netx56_default')


#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
env_default.Version('#targets/version/version.h', 'templates/version.h')


#----------------------------------------------------------------------------
#
# Build the platform libraries.
#
PLATFORM_LIB_CFG_BUILDS = [4000, 56]
SConscript('platform/SConscript', exports='PLATFORM_LIB_CFG_BUILDS')
Import('platform_lib_netx4000', 'platform_lib_netx56')


#----------------------------------------------------------------------------
# This is the list of sources. The elements must be separated with whitespace
# (i.e. spaces, tabs, newlines). The amount of whitespace does not matter.
sources = """
	src/console_io.c
	src/header.c
	src/init_netx_test.S
	src/main.c
"""


#----------------------------------------------------------------------------
#
# Build all files.
#
# netX4000 CR7
env_netx4000_cr7 = env_netx4000_default.Clone()
env_netx4000_cr7.Replace(LDFILE = 'src/netx4000/netx4000_cr7.ld')
src_netx4000_cr7 = env_netx4000_cr7.SetBuildPath('targets/netx4000_cr7', 'src', sources)
elf_netx4000_cr7 = env_netx4000_cr7.Elf('targets/netx4000_cr7/netx4000_cr7.elf', src_netx4000_cr7 + platform_lib_netx4000)
txt_netx4000_cr7 = env_netx4000_cr7.ObjDump('targets/netx4000_cr7/netx4000_cr7.txt', elf_netx4000_cr7, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
bb0_netx4000_cr7 = env_netx4000_cr7.HBootImage('targets/showcfg_netx4000_cr7_spi_intram.bin', 'src/netx4000/CR7_to_INTRAM.xml', KNOWN_FILES=dict({'tElf': elf_netx4000_cr7[0]}))
bb1_netx4000_cr7 = env_netx4000_cr7.HBootImage('targets/showcfg_netx4000_demo.bin', 'src/netx4000/config_demo.xml', KNOWN_FILES=dict({'tElf': elf_netx4000_cr7[0]}))
bb0_cifx4000_cr7 = env_netx4000_cr7.HBootImage('targets/showcfg_cifx4000_demo.bin', 'src/netx4000/board_config_1430100R1_cifx4000.xml', KNOWN_FILES=dict({'tElfCR7': elf_netx4000_cr7[0]}))



#env_netx56 = env_netx56_default.Clone()
#env_netx56.Replace(LDFILE = 'src/netx56/netx56.ld')
#src_netx56 = env_netx56.SetBuildPath('targets/netx56', 'src', sources)
#elf_netx56 = env_netx56.Elf('targets/netx56/netx56.elf', src_netx56 + platform_lib_netx56)
#bb0_netx56 = env_netx56.HBootImage('targets/showcfg_netx56_spi_intram.bin', 'src/netx56/SPI_to_INTRAM.xml', KNOWN_FILES=dict({'tElf': elf_netx56[0]}))
