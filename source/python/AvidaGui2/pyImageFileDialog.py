from qt import QFileDialog

class pyImageFileDialog(QFileDialog):

  def saveImageDialog(self):
    """Dialog that lets user select where to save images.
    Returns filename and filetype."""
    # Let user select file format
    dialog_caption = "Export Image"
    fd = self.getSaveFileName("", "JPEG (*.jpg);;PNG (*.png)", None,
                              "Save As", dialog_caption)
    filename = str(fd)
    if filename == "":
      return

    if filename[-4:].lower() == ".jpg":
      type = "JPEG"
    elif filename[-4:].lower() == ".png":
      type = "PNG"
    else:
      filename += ".jpg"
      type = "JPEG"

    return filename, type
