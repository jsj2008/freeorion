"""
Test case for building an app bundle with a command-line tool, that bundle
is than queried in the various test methods to check if the app bundle
is correct.

This is basicly a black-box functional test of the core py2app functionality

The app itself:
    - main script has 'if 0: import modules'
    - main script has a loop that reads and exec-s statements
    - the 'modules' module depends on a set of modules/packages through
      various forms of imports (absolute, relative, old-style python2,
      namespace packages 'pip-style', namespace package other,
      zipped eggs and non-zipped eggs, develop eggs)
    - add another test that does something simular, using virtualenv to
      manage a python installation
"""
import sys
if (sys.version_info[0] == 2 and sys.version_info[:2] >= (2,7)) or \
        (sys.version_info[0] == 3 and sys.version_info[:2] >= (3,2)):
    import unittest
else:
    import unittest2 as unittest

import subprocess
import shutil
import time
import os
import signal

DIR_NAME=os.path.dirname(os.path.abspath(__file__))


class TestBasicApp (unittest.TestCase):

    # Basic setup code
    #
    # The code in this block needs to be moved to
    # a base-class.
    @classmethod
    def setUpClass(cls):
         p = subprocess.Popen([
                 sys.executable,
                     'setup.py', 'py2app'],
             cwd = os.path.join(DIR_NAME, 'basic_app'),
             stdout=subprocess.PIPE,
             stderr=subprocess.STDOUT)
         lines = p.communicate()[0]
         if p.wait() != 0:
             print lines
             self.fail("Creating basic_app bundle failed")

    @classmethod
    def tearDownClass(cls):
        if os.path.exists(os.path.join(DIR_NAME, 'basic_app/build')):
            shutil.rmtree(os.path.join(DIR_NAME, 'basic_app/build'))

        if os.path.exists(os.path.join(DIR_NAME, 'basic_app/dist')):
            shutil.rmtree(os.path.join(DIR_NAME, 'basic_app/dist'))

    def start_app(self):
        # Start the test app, return a subprocess object where
        # stdin and stdout are connected to pipes.
        path = os.path.join(
                DIR_NAME,
            'basic_app/dist/BasicApp.app/Contents/MacOS/BasicApp')

        p = subprocess.Popen([path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                )
                #stderr=subprocess.STDOUT)
        return p

    def wait_with_timeout(self, proc, timeout=10):
        for i in xrange(timeout):
            x = proc.poll()
            if x is None:
                time.sleep(1)
            else:
                return x

        os.kill(proc.pid, signal.SIGKILL)
        return proc.wait()

    #
    # End of setup code
    # 

    def test_basic_start(self):
        p = self.start_app()

        p.stdin.close()

        exit = self.wait_with_timeout(p)
        self.assertEquals(exit, 0)

    def test_simple_imports(self):
        p = self.start_app()

        # Basic module that is always present:
        p.stdin.write('import_module("os")\n')
        p.stdin.flush()
        ln = p.stdout.readline()
        self.assertEquals(ln.strip(), b"os")

        # Dependency of the main module:
        p.stdin.write('import_module("decimal")\n')
        p.stdin.flush()
        ln = p.stdout.readline()
        self.assertEquals(ln.strip(), b"decimal")

        # Not a dependency of the module:
        p.stdin.write('import_module("xmllib")\n')
        p.stdin.flush()
        ln = p.stdout.readline().decode('utf-8')
        self.assertTrue(ln.strip().startswith("* import failed"), ln)

if __name__ == "__main__":
    unittest.main()

