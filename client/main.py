from ctypes import sizeof
from app import App
from PyQt5.QtWidgets import QWidget, QApplication
import sys
import signal


if __name__ == '__main__':    
    app = App()
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)
    sys.exit(app.app.exec_())
