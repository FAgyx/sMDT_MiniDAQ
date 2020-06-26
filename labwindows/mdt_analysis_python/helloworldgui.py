#! /usr/bin/python3
# Character Encoding: UTF-8
#
# Here we provide the necessary imports.
# The basic GUI widgets are located in QtWidgets module.
import sys
from PyQt5.QtWidgets import QApplication, QWidget

# Every PyQt5 application must create an application object.
# The application object is located in the QtWidgets module.
app = QApplication(sys.argv)

# The QWidget widget is the base class of all user interface objects in PyQt5.
# We provide the default constructor for QWidget. The default constructor has no parent.
# A widget with no parent is called a window.
root = QWidget()

root.resize(320, 240)  # The resize() method resizes the widget.
root.setWindowTitle("Hello, World!")  # Here we set the title for our window.
root.show()  # The show() method displays the widget on the screen.

sys.exit(app.exec_())  # Finally, we enter the mainloop of the application.