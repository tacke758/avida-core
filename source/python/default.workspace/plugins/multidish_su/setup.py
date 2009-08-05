import os.path
from distutils.command.bdist import bdist
from setuptools import setup, find_packages

packages = find_packages()
print packages
  
setup(
  name = "multidish_su",
  version = "0.0.1",
  package_data = {'':['*.*']},
  packages = find_packages(),
)
