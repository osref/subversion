#
# run-tests.py - run the tests in the regression test suite.
#

import os, sys


class TestHarness:
  '''Test harness for Subversion tests.
  '''

  def __init__(self, abs_srcdir, abs_builddir, python, shell, logfile,
               base_url = None):
    '''Construct a TestHarness instance.

    ABS_SRCDIR and ABS_BUILDDIR are the source and build directories.
    PYTHON is the name of the python interpreter.
    SHELL is the name of the shell.
    LOGFILE is the name of the log file.
    BASE_URL is the base url for DAV tests.
    '''
    self.srcdir = abs_srcdir
    self.builddir = abs_builddir
    self.python = python
    self.shell = shell
    self.logfile = logfile
    self.base_url = base_url
    self.log = None

  def run(self, list):
    'Run all test programs given in LIST.'
    self._open_log('w')
    failed = 0
    for prog in list:
      failed = self._run_test(prog) or failed

    print '\nSummary results from', self.logfile
    self._open_log('r')
    test_log = self.log.readlines()
    pass_lines = filter(lambda x: x[:5] == 'PASS:', test_log)
    fail_lines = filter(lambda x: x[:5] == 'FAIL:', test_log)
    xpass_lines = filter(lambda x: x[:6] == 'XPASS:', test_log)
    xfail_lines = filter(lambda x: x[:6] == 'XFAIL:', test_log)
    self._close_log()

    print 'number of passed tests:', len(pass_lines)
    print 'number of failed tests:', len(fail_lines)
    print 'number of expected failures:', len(xfail_lines)
    print 'number of unexpected passes:', len(xpass_lines)

    if len(fail_lines):
      print '\nFailed tests:'
      map(sys.stdout.write, fail_lines)
    if len(xfail_lines):
      print '\nExpected failures:'
      map(sys.stdout.write, xfail_lines)
    if len(xpass_lines):
      print '\nUnexpected passes:'
      map(sys.stdout.write, xpass_lines)

    return failed

  def _open_log(self, mode):
    'Open the log file with the required MODE.'
    self._close_log()
    self.log = open(self.logfile, mode)

  def _close_log(self):
    'Close the log file.'
    if not self.log is None:
      self.log.close()
      self.log = None

  def _run_test(self, prog):
    'Run a single test.'

    def quote(arg):
      if sys.platform == 'win32':
        return '"' + arg + '"'
      else:
        return arg

    progdir, progbase = os.path.split(prog)
    # Using write here because we don't want even a trailing space
    sys.stdout.write('Running all tests in ' + progbase + '...')
    print >> self.log, 'START: ' + progbase

    if progbase[-3:] == '.py':
      progname = self.python
      cmdline = [quote(progname),
                 quote(os.path.join(self.srcdir, prog))]
      if self.base_url is not None:
        cmdline.append('--url')
        cmdline.append(quote(self.base_url))
    elif progbase[-3:] == '.sh':
      progname = self.shell
      cmdline = [quote(progname),
                 quote(os.path.join(self.srcdir, prog)),
                 quote(os.path.join(self.builddir, progdir)),
                 quote(os.path.join(self.srcdir, progdir))]
    elif os.access(prog, os.X_OK):
      progname = './' + progbase
      cmdline = [quote(progname)]
    else:
      print 'Don\'t know what to do about ' + progbase
      sys.exit(1)

    old_cwd = os.getcwd()
    try:
      os.chdir(progdir)
      failed = self._run_prog(progname, cmdline)
    except:
      os.chdir(old_cwd)
      raise
    else:
      os.chdir(old_cwd)

    if failed:
      print 'FAILURE'
    else:
      print 'success'
    print >> self.log, 'END: ' + progbase + '\n'
    return failed

  def _run_prog(self, progname, cmdline):
    'Execute COMMAND, redirecting standard output and error to the log file.'
    def restore_streams(stdout, stderr):
      os.dup2(stdout, 1)
      os.dup2(stderr, 2)
      os.close(stdout)
      os.close(stderr)

    sys.stdout.flush()
    sys.stderr.flush()
    self.log.flush()
    old_stdout = os.dup(1)
    old_stderr = os.dup(2)
    try:
      os.dup2(self.log.fileno(), 1)
      os.dup2(self.log.fileno(), 2)
      rv = os.spawnv(os.P_WAIT, progname, cmdline)
    except:
      restore_streams(old_stdout, old_stderr)
      raise
    else:
      restore_streams(old_stdout, old_stderr)
      return rv


def main():
  '''Argument parsing and test driver.

  Usage: run-tests.py [--url <base_url>] <abs_srcdir> <abs_builddir>
                      <python> <shell> <prog ...>

  The optional base_url and the first four parameters and  are passed
  unchanged to the TestHarness constuctor.  All other parameters
  are names of test programs.
  '''
  if len(sys.argv) < 6 \
     or sys.argv[1] == '--url' and len(sys.argv) < 8:
    print 'Usage: run-tests.py <abs_srcdir> <abs_builddir>' \
          '<python> <shell> [--url <base_url>] <prog ...>'
    sys.exit(2)

  if sys.argv[1] == '--url':
    base_index = 2
    base_url = sys.argv[2]
  else:
    base_index = 0
    base_url = None

  th = TestHarness(sys.argv[base_index+1], sys.argv[base_index+2],
                   sys.argv[base_index+3], sys.argv[base_index+4],
                   os.path.abspath('tests.log'), base_url)

  failed = th.run(sys.argv[base_index+5:])
  if failed:
    sys.exit(1)


# Run main if not imported as a module
if __name__ == '__main__':
  main()
