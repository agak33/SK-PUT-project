from ctypes import sizeof
from app import App
from PyQt5.QtWidgets import QWidget, QApplication
import sys


if __name__ == '__main__':    
    app = App()
    sys.exit(app.app.exec_())
