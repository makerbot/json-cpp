# vim:ai:et:ff=unix:fileencoding=utf-8:sw=4:syntax=python:ts=4:

import os
import sys

env = Environment(ENV=os.environ)

if 'win32' == sys.platform:
    env.Tool('mingw')
    env.Replace(CCFLAGS=[])

env.Tool('default')
env.Append(CCFLAGS='-Wall')

pysrc_root = str(Dir('#/src/main/python'))
for root,dirnames,filenames in os.walk(pysrc_root):
    for filename in fnmatch.filter(filenames,'*.py'):
        rpath = os.path.relpath(root,pysrc_root)
        outdir = os.path.join('module',rpath)
        insrc = os.path.join(root,filename)
        inst.append(cppenv.Install(outdir,insrc))
        # print outdir,insrc

includes = Dir('include')
sources = Dir('#/src/lib_json/')

libjsonenv = env.Clone()
libjsonenv.Append(CPPPATH=[includes])
libjsonenv.Repository(sources)

env.Tool('mb_install', toolpath=[Dir('submodules/mw-scons-tools')])

libjson = libjsonenv.SharedLibrary(
    'json', [
        File('src/lib_json/json_reader.cpp'),
        File('src/lib_json/json_value.cpp'),
        File('src/lib_json/json_writer.cpp'),])
Default(libjson)

env.Tool('mb_install', toolpath=[Dir('submodules/mw-scons-tools')])
env.MBInstallLib(libjson)
env.MBInstallHeaders(env.MBGlob('include/*'))

env.MBCreateInstallTarget()

