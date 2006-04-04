
avida_ed_version_string = 'Avida-ED v2.0'
try:
  import Avida_ED_version
  reload(Avida_ED_version)
  avida_ed_version_string += '.%s' % Avida_ED_version.svn_revision_string
except:
  avida_ed_version_string += '.(unknown)'
