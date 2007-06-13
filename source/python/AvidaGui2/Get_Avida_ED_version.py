
avida_ed_version_string = 'Avida-ED v1.0 '
try:
  import Avida_ED_version
  reload(Avida_ED_version)
  avida_ed_version_string += '(build %s)' % Avida_ED_version.svn_revision_string
except:
  avida_ed_version_string += '.(unknown)'
