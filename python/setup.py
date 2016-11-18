import os
from setuptools import setup, Extension
import numpy
include_dirs_numpy = [numpy.get_include()]

extension = Extension('alm._alm',
                      libraries=['stdc++'],
                      include_dirs=include_dirs_numpy,
                      extra_compile_args = ['-fopenmp'],
                      extra_link_args=['libalmcxx.a'],
                      sources=['_alm.c',
                               'alm_wrapper.cpp'])

setup(name='alm',
      version='0.9.8',
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
