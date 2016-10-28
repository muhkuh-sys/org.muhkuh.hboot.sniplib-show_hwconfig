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
# netX4000 CR7 with console
env_netx4000_cr7 = env_netx4000_default.Clone()
env_netx4000_cr7.Replace(LDFILE = 'src/netx4000/netx4000_cr7.ld')
env_netx4000_cr7.Append(CPPDEFINES = [['SHOW_HWCONFIG_CONSOLE']])
src_netx4000_cr7 = env_netx4000_cr7.SetBuildPath('targets/netx4000_cr7', 'src', sources)
elf_netx4000_cr7 = env_netx4000_cr7.Elf('targets/netx4000_cr7/netx4000_cr7.elf', src_netx4000_cr7 + platform_lib_netx4000)
txt_netx4000_cr7 = env_netx4000_cr7.ObjDump('targets/netx4000_cr7/netx4000_cr7.txt', elf_netx4000_cr7, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
bin_netx4000_cr7 = env_netx4000_cr7.ObjCopy('targets/netx4000_cr7/netx4000_cr7.bin', elf_netx4000_cr7)
tmp_netx4000_cr7 = env_netx4000_cr7.GccSymbolTemplate('targets/netx4000_cr7/snippet.xml', elf_netx4000_cr7, GCCSYMBOLTEMPLATE_TEMPLATE='templates/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=bin_netx4000_cr7[0])

# Create the snippet from the parameters.
global PROJECT_VERSION
aArtifactGroupReverse = ['org', 'muhkuh', 'hboot', 'sniplib']
atSnippet = {
    'group': '.'.join(aArtifactGroupReverse),
    'artifact': 'show_hwconfig_netx4000',
    'version': PROJECT_VERSION,
    'vcs_id': env_netx4000_cr7.Version_GetVcsIdLong(),
    'vcs_url': env_netx4000_cr7.Version_GetVcsUrl(),
    'license': 'GPL-2.0',
    'author_name': 'Muhkuh team',
    'author_url': 'https://github.com/muhkuh-sys',
    'description': 'Show the current hardware configuration with an interactive menu on UART0.',
    'categories': ['netx4000', 'hardware configuration']
}
strArtifactPath = 'targets/snippets/%s/%s/%s' % ('/'.join(aArtifactGroupReverse), atSnippet['artifact'], PROJECT_VERSION)
snippet_netx4000_cr7 = env_netx4000_cr7.HBootSnippet('%s/%s-%s.xml' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), tmp_netx4000_cr7, PARAMETER=atSnippet)

# Create the POM file.
tPOM = env_netx4000_cr7.POMTemplate('%s/%s-%s.pom' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), 'templates/pom.xml', POM_TEMPLATE_GROUP=atSnippet['group'], POM_TEMPLATE_ARTIFACT=atSnippet['artifact'], POM_TEMPLATE_VERSION=atSnippet['version'], POM_TEMPLATE_PACKAGING='xml')



# netX4000 CR7 with automatic display
env_netx4000_cr7_auto = env_netx4000_default.Clone()
env_netx4000_cr7_auto.Replace(LDFILE = 'src/netx4000/netx4000_cr7.ld')
env_netx4000_cr7_auto.Append(CPPDEFINES = [['SHOW_HWCONFIG_AUTO']])
src_netx4000_cr7_auto = env_netx4000_cr7_auto.SetBuildPath('targets/netx4000_cr7_auto', 'src', sources)
elf_netx4000_cr7_auto = env_netx4000_cr7_auto.Elf(         'targets/netx4000_cr7_auto/netx4000_cr7_auto.elf', src_netx4000_cr7_auto + platform_lib_netx4000)
txt_netx4000_cr7_auto = env_netx4000_cr7_auto.ObjDump(     'targets/netx4000_cr7_auto/netx4000_cr7_auto.txt', elf_netx4000_cr7_auto, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
bin_netx4000_cr7_auto = env_netx4000_cr7_auto.ObjCopy(     'targets/netx4000_cr7_auto/netx4000_cr7_auto.bin', elf_netx4000_cr7_auto)
tmp_netx4000_cr7_auto = env_netx4000_cr7_auto.GccSymbolTemplate('targets/netx4000_cr7_auto/snippet.xml', elf_netx4000_cr7_auto, GCCSYMBOLTEMPLATE_TEMPLATE='templates/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=bin_netx4000_cr7_auto[0])

# Create the snippet from the parameters.
atSnippet = {
    'group': '.'.join(aArtifactGroupReverse),
    'artifact': 'show_hwconfig_auto_netx4000',
    'version': PROJECT_VERSION,
    'vcs_id': env_netx4000_cr7_auto.Version_GetVcsIdLong(),
    'vcs_url': env_netx4000_cr7_auto.Version_GetVcsUrl(),
    'license': 'GPL-2.0',
    'author_name': 'Muhkuh team',
    'author_url': 'https://github.com/muhkuh-sys',
    'description': 'Show the current hardware configuration with an interactive menu on UART0.',
    'categories': ['netx4000', 'hardware configuration']
}
strArtifactPath = 'targets/snippets/%s/%s/%s' % ('/'.join(aArtifactGroupReverse), atSnippet['artifact'], PROJECT_VERSION)
snippet_netx4000_cr7_auto = env_netx4000_cr7_auto.HBootSnippet('%s/%s-%s.xml' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), tmp_netx4000_cr7_auto, PARAMETER=atSnippet)

# Create the POM file.
tPOM = env_netx4000_cr7_auto.POMTemplate('%s/%s-%s.pom' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), 'templates/pom.xml', POM_TEMPLATE_GROUP=atSnippet['group'], POM_TEMPLATE_ARTIFACT=atSnippet['artifact'], POM_TEMPLATE_VERSION=atSnippet['version'], POM_TEMPLATE_PACKAGING='xml')





#env_netx56 = env_netx56_default.Clone()
#env_netx56.Replace(LDFILE = 'src/netx56/netx56.ld')
#src_netx56 = env_netx56.SetBuildPath('targets/netx56', 'src', sources)
#elf_netx56 = env_netx56.Elf('targets/netx56/netx56.elf', src_netx56 + platform_lib_netx56)
#bb0_netx56 = env_netx56.HBootImage('targets/showcfg_netx56_spi_intram.bin', 'src/netx56/SPI_to_INTRAM.xml', KNOWN_FILES=dict({'tElf': elf_netx56[0]}))
