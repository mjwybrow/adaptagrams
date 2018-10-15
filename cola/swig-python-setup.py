#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

from distutils.core import setup, Extension
from distutils import util
from distutils import sysconfig
import os

platform = util.get_platform()

# special settings for Linux
if platform.startswith("linux"):
    # option '-Wstrict-prototypes' is enabled by default and gives a warning on Linux
    if sysconfig.get_config_vars().has_key("OPT"):
        opt = sysconfig.get_config_vars().get("OPT")
        if type(opt) == str:
            opt = opt.replace("-Wstrict-prototypes", "")
            (sysconfig.get_config_vars())["OPT"] = opt

    os.environ['CC'] = 'gcc'
    os.environ['CXX'] = 'g++'

adaptagrams_module = Extension('_adaptagrams',
                               sources=['adaptagrams_wrap.cxx'],
			                   extra_compile_args=['-DUSE_ASSERT_EXCEPTIONS','-DSWIG_PYTHON_SILENT_MEMLEAK','-std=gnu++11','-stdlib=libc++'],
                               # use this line for SWIG 2.0.12 to silence some harmless warnings 
                               #extra_compile_args=['-DUSE_ASSERT_EXCEPTIONS','-DSWIG_PYTHON_SILENT_MEMLEAK','-Wno-uninitialized'],
			                   include_dirs=['.'],
			                   extra_link_args=['libcola/.libs/libcola.a','libtopology/.libs/libtopology.a', 'libavoid/.libs/libavoid.a','libvpsc/.libs/libvpsc.a','libdialect/.libs/libdialect.a'],
                              )

setup (name = 'adaptagrams',
       version = '0.1',
       author      = "MArVL",
       description = """Adaptagrams libraries""",
       ext_modules = [adaptagrams_module],
       py_modules = ["adaptagrams"],
      )
