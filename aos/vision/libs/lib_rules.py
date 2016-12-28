#!/usr/bin/python3
import subprocess, os
import re
def get_output(cmd):
  my_env = os.environ.copy()
  if(variant == 'windows'):
    my_env['PKG_CONFIG_PATH'] = '/tmp/gtk_download_test/lib/pkgconfig'
  p = subprocess.Popen(cmd, env = my_env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()
  return list(map(lambda x: str(x, 'utf8'), filter(len, re.compile(b'\s+').split(out))))

@GenRuleFunc
def pkg_config(name):
  rule = GenRule(name)
  rule.meta.cc_system_so_flags.add_all(get_output(['pkg-config', name, '--libs']))
  rule.meta.dep_cc_flags.add_all(get_output(['pkg-config', name, '--cflags']))
  return rule

@GenRuleFunc
def cc_lflags(name, lflags = []):
  rule = GenRule(name)
  rule.meta.cc_system_so_flags.add_all(lflags)
  return rule

@GenRuleFunc
def cc_a_library(name, libname, cflags = []):
  rule = GenRule(name)
  rule.inp.cc_a_lib.add(libname)
  rule.out.cc_a_lib.depends_on(rule.inp.cc_a_lib)
  rule.meta.dep_cc_flags.add_all(cflags)
  return rule
