import os
from setuptools import setup, Extension
# from distutils.core import setup, Extension
import numpy

os.environ["CC"] = "gcc-8"
os.environ["CXX"] = "g++-8"

include_dirs_numpy = [numpy.get_include()]

compile_with_sources = False
extra_link_args = ['-lstdc++', '-lgomp',
                   '-llapack', '-static',
                   '-L/Users/tadano/src/spglib/lib', '-lsymspg',
                   '-fopenmp', '-Wl,-rpath,/usr/local/opt/gcc/lib/gcc/7,-rpath,/Users/tadano/src/spglib/lib']
#                   '/Users/tadano/src/spglib/lib/libsymspg.a',
#                   '/usr/local/opt/gcc/lib/gcc/7/libgomp.a']


include_dir_spglib=['/Users/tadano/src/spglib/include', '/usr/local/include/eigen3/']

library_dirs = []
if compile_with_sources:
    sources = ['alm.cpp',
               'alm_cui.cpp',
               'constraint.cpp',
               'fcs.cpp',
               'files.cpp',
               'fitting.cpp',
               'input_parser.cpp',
               'input_setter.cpp',
               'interaction.cpp',
               'lasso.cpp',
               'main.cpp',
               'patterndisp.cpp',
               'rref.cpp',
               'symmetry.cpp',
               'system.cpp',
               'timer.cpp',
               'writer.cpp']
    if os.path.exists('src'):
        source_dir = "src"
    else:
        source_dir = "../src"
    
    include_dirs = [source_dir,]
    include_dirs += include_dirs_numpy
    for i, s in enumerate(sources):
        sources[i] = "%s/%s" % (source_dir, s) 
    sources += ['_alm.c', 'alm_wrapper.cpp']
else: # compile with library
    sources = ['_alm.c', 'alm_wrapper.cpp']
    # static link library
    extra_link_args += ['../lib/libalmcxx.a']
    # dynamic link library
    # extra_link_args += ['-lalmcxx']
    # library_dirs = ['../lib']

extension = Extension('alm._alm',
                      include_dirs=include_dirs_numpy + include_dir_spglib,
                      library_dirs=library_dirs,
                      extra_compile_args = ['-fopenmp', '-std=c++11'],
                      extra_link_args=extra_link_args,
                      sources=sources)

setup(name='alm',
      version='1.0.2',
      description='Force constants generator',
      setup_requires=['numpy', 'setuptools>=18.0'],
      author='Terumasa Tadano',
      author_email='terumasa.tadano@gmail.com',
      url='https://github.com/ttadano/ALM',
      packages=['alm'],
      install_requires=['numpy'],
      provides=['alm'],
      platforms=['all'],
      ext_modules=[extension])
