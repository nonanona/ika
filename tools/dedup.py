import os
import sys
import hashlib

def sha256(fname):
  return hashlib.sha256(open(fname, 'rb').read()).hexdigest()

def main():
  target_dir = sys.argv[1]
  dryrun = '--dryrun' in sys.argv
  verbose = '-v' in sys.argv
  files = os.listdir(target_dir)
  m = {}
  for f in files:
    full_path = os.path.join(target_dir, f)
    if os.path.isdir(full_path):
      continue
    dig = sha256(full_path)
    if dig not in m:
      m[dig] = []
    m[dig].append(full_path)

  for k, v in m.iteritems():
    if len(v) == 1:
      continue
    if verbose:
      print 'Found dupped files:'
      for f in v:
        print '  %s' % f
    for f in v[1:]:
      if verbose:
        print 'Removing %s' % f
      if not dryrun:
        os.remove(f)
      

if __name__ == '__main__':
  main()
