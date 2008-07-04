import os, os.path
def DefaultExportDir(export_dir = None, file_name = None):
  if export_dir is not None and os.path.exists(export_dir):
    directory = export_dir
  else:
    user_dir = os.path.expanduser("~")
    if os.path.exists(os.path.join(user_dir,"Desktop")):
      directory = os.path.join(user_dir,"Desktop")
    elif os.path.exists(os.path.join(user_dir,"Documents")):
      directory = os.path.join(user_dir,"Documents")
    elif os.path.exists(os.path.join(user_dir,"My Documents")):
      directory = os.path.join(user_dir,"My Documents")
    elif file_name is not None and os.path.exists(os.path.abspath(os.path.dirname(file_name))):
      directory = os.path.abspath(os.path.dirname(file_name));
    else:
      directory = os.path.abspath(os.curdir)
  return directory
