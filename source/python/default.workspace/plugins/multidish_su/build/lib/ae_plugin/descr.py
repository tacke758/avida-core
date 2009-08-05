
from qt import QMessageBox

import inspect
import sys

DEBUG = True
#DEBUG = False

def _details(*details):
    s = inspect.stack()
    l = len(s) - 1
    frame = s[2]
    args, varargs, varkw, locals = inspect.getargvalues(frame[0])
    if locals.has_key('self'):
      mthd = getattr(locals['self'], frame[3], None)
      doc, cls, fnm = mthd is None and (None, None, None) or (mthd.im_func.__doc__, mthd.im_class.__name__, mthd.im_func.__name__)
      fun = "%s.%s" % (cls, fnm)
    else:
      doc = frame[0].f_code.co_consts[0]
      fun = frame[0].f_code.co_name
    if doc is None: doc = "<no docs.>"
    if details is ():
      pfx, txt = '.'*l, doc
    elif len(details) == 1:
      pfx, txt = ' '*l, str(*details)
    else:
      pfx, txt = ' '*l, str(details)
    return pfx, fun, txt

def descr(*details):
  if DEBUG:
    pfx, fun, txt = _details(*details)
    print "%s %s: %s" % (pfx, fun, txt)
    sys.stdout.flush()
    return "%s %s: %s" % (pfx, fun, txt)

def info(*details):
  pfx, fun, txt = _details(*details)
  QMessageBox.information(
    None,
    "Avida-ED Information",
    txt,
    QMessageBox.Ok,
    QMessageBox.NoButton,
    QMessageBox.NoButton
  )

def question(*details):
  pfx, fun, txt = _details(*details)
  return QMessageBox.Ok == QMessageBox.question(
    None,
    "Question",
    txt,
    QMessageBox.Ok,
    QMessageBox.Cancel
  )

def warning(*details):
  pfx, fun, txt = _details(*details)
  QMessageBox.warning(
    None,
    "Avida-ED Warning",
    "(In function %s:)\n%s" % (fun, txt),
    QMessageBox.Ok,
    QMessageBox.NoButton,
    QMessageBox.NoButton
  )

def warningNoMethodName(*details):
  pfx, fun, txt = _details(*details)
  QMessageBox.warning(
    None,
    "Avida-ED Warning",
    "%s" % (txt),
    QMessageBox.Ok,
    QMessageBox.NoButton,
    QMessageBox.NoButton
  )

def critical(*details):
  pfx, fun, txt = _details(*details)
  QMessageBox.critical(
    None,
    "Avida-ED Critical Error",
    "(In function %s:)\n%s" % (fun, txt),
    QMessageBox.Ok,
    QMessageBox.NoButton,
    QMessageBox.NoButton
  )

