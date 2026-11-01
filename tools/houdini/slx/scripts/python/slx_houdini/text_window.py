"""Shows read-only XML, such as compiled MaterialX, in a window with a monospace font and syntax highlighting."""

import hou

try:
    from hutil.PySide import QtCore, QtGui, QtWidgets
except ImportError:
    # Houdini versions without hutil.PySide all ship PySide2.
    from PySide2 import QtCore, QtGui, QtWidgets

# The first is the monospace font Houdini's own text views use. Older Houdini versions ship the others.
MONOSPACE_FAMILIES = ["SideFX Source Code Pro", "Source Code Pro", "DejaVu Sans Mono"]

# Houdini's dark scheme colours attribute names and = signs too dimly to read easily, so the window has its own
# colours. Each has a contrast of at least 7:1 with the background, which is Houdini's dark text box colour.
BACKGROUND_COLOUR = "#131313"
SYNTAX_COLOURS = {  # by Houdini's syntax style name
    "plain": "#e8e8e8",
    "type": "#8cd0ff",  # tags
    "keyword": "#ffd479",  # attribute names
    "extra": "#e8e8e8",  # =
    "string": "#a5e07a",  # attribute values
    "quote": "#ffb08a",  # entities, such as &amp;
    "error": "#ff6b6b",
}

try:
    from hutil.qt.editors.syntax import Style
    from hutil.qt.editors.xml import XmlHighlighter
except ImportError:
    HighContrastXmlHighlighter = None  # older Houdini versions don't have it, so their XML is shown uncoloured
else:
    class HighContrastXmlHighlighter(XmlHighlighter):
        """Houdini's XML highlighter with SYNTAX_COLOURS.

        It also stops the ?> that ends the XML declaration being coloured as an error.
        """

        def styleColor(self, style):
            colour = SYNTAX_COLOURS.get(style.name)
            return QtGui.QColor(colour) if colour else super().styleColor(style)

        def highlightBlock(self, line):
            super().highlightBlock(line)
            end = len(line.rstrip())
            if line.startswith("<?") and line.endswith("?>", 0, end):
                self.setFormat(end - 2, 2, self.styleColor(Style.type))


def show_xml(title, heading, xml):
    """Opens a window showing xml below heading. It stays open while you keep working in Houdini."""
    window = QtWidgets.QDialog(hou.qt.mainWindow())
    window.setWindowTitle(title)
    window.setAttribute(QtCore.Qt.WidgetAttribute.WA_DeleteOnClose)

    text_edit = QtWidgets.QPlainTextEdit(xml)
    text_edit.setReadOnly(True)
    text_edit.setLineWrapMode(QtWidgets.QPlainTextEdit.LineWrapMode.NoWrap)
    text_edit.setFont(create_monospace_font(text_edit.font()))
    set_text_colours(text_edit)
    if HighContrastXmlHighlighter is not None:
        HighContrastXmlHighlighter(text_edit.document())

    buttons = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.StandardButton.Close)
    buttons.rejected.connect(window.reject)

    layout = QtWidgets.QVBoxLayout(window)
    layout.addWidget(QtWidgets.QLabel(heading))
    layout.addWidget(text_edit)
    layout.addWidget(buttons)
    window.resize(hou.ui.scaledSize(900), hou.ui.scaledSize(700))
    window.show()
    return window


def create_monospace_font(font):
    """Returns a copy of font, keeping its size, in Houdini's monospace font."""
    font = QtGui.QFont(font)
    font.setFamilies(MONOSPACE_FAMILIES)
    font.setStyleHint(QtGui.QFont.StyleHint.TypeWriter)  # any monospace font if none of those are installed
    return font


def set_text_colours(text_edit):
    # Set even with Houdini's light scheme, which SYNTAX_COLOURS would be unreadable on.
    palette = text_edit.palette()
    palette.setColor(QtGui.QPalette.ColorRole.Base, QtGui.QColor(BACKGROUND_COLOUR))
    palette.setColor(QtGui.QPalette.ColorRole.Text, QtGui.QColor(SYNTAX_COLOURS["plain"]))
    text_edit.setPalette(palette)
