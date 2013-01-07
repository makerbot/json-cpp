# vim:ai:et:ff=unix:fileencoding=utf-8:sw=4:syntax=python:ts=4:

import os
import sys

env = Environment(ENV=os.environ)

if 'win32' == sys.platform:
    env.Tool('mingw')
    env.Replace(CCFLAGS=[])
else:
    env.Append(CCFLAGS='-fPIC')

env.Append(CCFLAGS='-Wall')

libjsonenv = env.Clone()
libjsonenv.Append(CPPPATH=[Dir('#/include/')])
libjsonenv.Repository(Dir('#/src/lib_json/'))
libjson = libjsonenv.SharedLibrary(
    'json', [
        File('src/lib_json/json_reader.cpp'),
        File('src/lib_json/json_value.cpp'),
        File('src/lib_json/json_writer.cpp'),])
