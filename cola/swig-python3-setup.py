#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

from distutils.core import setup, Extension
# distutils are marked deprecated and will be removed from Python 3.13 and beyond
# install setuptools
# from setuptools import setup, Extension

adaptagrams_module = Extension('_adaptagrams',
                               sources=['adaptagrams_wrap.cxx'],
                               # extra_compile_args=['-DUSE_ASSERT_EXCEPTIONS','-DSWIG_PYTHON_SILENT_MEMLEAK','-std=gnu++11','-stdlib=libc++'],
                               extra_compile_args=['-DUSE_ASSERT_EXCEPTIONS','-DSWIG_PYTHON_SILENT_MEMLEAK','-std=c++11'],
                               include_dirs=['.'],
                               extra_link_args=['libcola/.libs/libcola.a','libtopology/.libs/libtopology.a', 'libavoid/.libs/libavoid.a','libvpsc/.libs/libvpsc.a','libdialect/.libs/libdialect.a'],
                               # use this line on MacOS with older versions of the developer tools to avoid linker warning
                               # '-undefined dynamic_lookup may not work with chained fixups'
                               # see https://github.com/pybind/pybind11/pull/4301
                               # extra_link_args=['-Wl,-no_fixup_chains','libcola/.libs/libcola.a','libtopology/.libs/libtopology.a', 'libavoid/.libs/libavoid.a','libvpsc/.libs/libvpsc.a','libdialect/.libs/libdialect.a'],
                              )

setup (name = 'adaptagrams',
       version = '0.1',
       author      = "MArVL",
       description = """Adaptagrams libraries""",
       ext_modules = [adaptagrams_module],
       py_modules = ["adaptagrams"],
      )
