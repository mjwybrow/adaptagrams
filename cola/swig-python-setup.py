#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

from distutils.core import setup, Extension


adaptagrams_module = Extension('_adaptagrams',
                           sources=['adaptagrams_wrap.cxx'],
			   extra_compile_args=['-DUSE_ASSERT_EXCEPTIONS','-DSWIG_PYTHON_SILENT_MEMLEAK'],
			   include_dirs=['.'],
			   extra_link_args=['libcola/.libs/libcola.a','libtopology/.libs/libtopology.a','libavoid/.libs/libavoid.a','libvpsc/.libs/libvpsc.a'],
                           )

setup (name = 'adaptagrams',
       version = '0.1',
       author      = "MArVL",
       description = """Adaptagrams libraries""",
       ext_modules = [adaptagrams_module],
       py_modules = ["adaptagrams"],
       )
