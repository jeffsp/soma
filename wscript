# @file wscript
# @brief waf build script
# @author Jeff Perry <jeffsp@gmail.com>
# @version 1.0
# @date 2013-07-20

import sys

# waf project directories
top = '.'
out = 'build'

# global definitions
SOURCES='*.cc'
CXXFLAGS=['-fopenmp','-Wall','-std=c++0x']
INCLUDES='../Leap_Developer_Kit/LeapSDK/include'
# osx LIBPATHS='/Users/jeffperry/Projects/leap/Leap_Developer_Kit/LeapSDK/lib/'
# linux LIBPATHS='../Leap_Developer_Kit/LeapSDK/lib/x64'
LIBPATHS='/Users/jeffperry/Projects/leap/Leap_Developer_Kit/LeapSDK/lib/'
if sys.platform=="linux2":
    LIBS=['Leap','X11','Xtst']
    LINKFLAGS=['']
elif sys.platform=='darwin':
    LIBS=['Leap']
    LINKFLAGS=['-framework ApplicationServices']
elif sys.platform=='windows':
    LIBS=['Leap']
    LINKFLAGS=['']
else:
    LIBS=['Leap']
    LINKFLAGS=['']

# variant specific build flags
DEBUG_CXXFLAGS=CXXFLAGS+['-g']
RELEASE_CXXFLAGS=CXXFLAGS+['-O2','-DNDEBUG']

import glob

def configure(ctx):

    ctx.setenv('debug')
    ctx.load('compiler_cxx')
    ctx.env.CXXFLAGS=DEBUG_CXXFLAGS
    ctx.env.SOURCES=glob.glob(SOURCES)

    ctx.setenv('release')
    ctx.load('compiler_cxx')
    ctx.env.CXXFLAGS=RELEASE_CXXFLAGS
    ctx.env.SOURCES=glob.glob(SOURCES)

def options(opt):

    opt.load('compiler_cxx')

def init(ctx):

    # setup contexts build_debug, build_release, clean_debug, ...
    from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext
    for x in (BuildContext, CleanContext, InstallContext, UninstallContext):
        for y in ['debug','release']:
            class tmp(x):
                variant=y
                cmd=x.__name__.replace('Context','').lower()+'_'+y

def build(ctx):

    # if no variant was specified then build them all
    if not ctx.variant:
        import waflib.Options
        for x in ['debug', 'release']:
            waflib.Options.commands.insert(0, ctx.cmd+'_'+x)
    else:
        # the executable name is the filename without the extension
        for s in ctx.env.SOURCES:
            ctx.program(source=s,target=s.replace('.cc',''),includes=INCLUDES,lib=LIBS,libpath=LIBPATHS,linkflags=LINKFLAGS)
