from calendar import Calendar
from setup import *
from serverRequests import ServerRequests
from PyQt5 import QtWidgets, uic
from functools import partial
import threading

from userInfo import UserInfo

class App(object):
    def __init__(self) -> None:
        self.app            = QtWidgets.QApplication([])
        self.app.aboutToQuit.connect(self.exit)
        self.mainWindow     = QtWidgets.QMainWindow()
        self.screen         = QtWidgets.QWidget()

        self.thread         = threading.Thread()
        self.server         = ServerRequests()

        self.user           = UserInfo()
        self.calendarList   = []
        self.eventList      = []

        self.setupMainWindow()
        self.loginWindow()

    def setupMainWindow(self) -> None:
        uic.loadUi(f'{SCREEN_MODELS_FOLDER}/{MAIN_APP_WINDOW}', self.mainWindow)

        self.mainWindow.actionCalendarList.triggered.connect(self.calendarListWindow)
        self.mainWindow.actionAddNewCalendar.triggered.connect(self.newCalendarWindow)
        self.mainWindow.actionLogout.triggered.connect(self.logout)

    def loadScreen(self, path: str) -> None:
        self.screen = QtWidgets.QWidget()
        uic.loadUi(path, self.screen)

    def loginWindow(self) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_LOGIN_REGISTER}')
        self.screen.loginButton.clicked.connect(
            lambda: self.executeThread(self.screen.errorLoginLabel, self.login)
        )
        self.screen.registerButton.clicked.connect(
            lambda: self.executeThread(self.screen.errorRegisterLabel, self.register)
        )
        self.screen.show()

    def calendarListWindow(self) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_CALENDAR_LIST}')     

        self.mainWindow.setCentralWidget(self.screen)
        self.mainWindow.show()

    def newCalendarWindow(self) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_NEW_CALENDAR}')     

        self.mainWindow.setCentralWidget(self.screen)
        self.mainWindow.show()

    def executeThread(self, errorLabel, target, args=()):
        if not self.thread.is_alive():
            self.thread = threading.Thread(target=target, args=args)
            print('starting thread...')
            self.thread.start()
        else:
            errorLabel.setText('Waiting for server to response')        

    def login(self) -> None:
        login, passwd = self.screen.loginLoginField.text(), self.screen.passwordLoginField.text()
        if login == '' or passwd == '':
            self.screen.errorLoginLabel.setText('Login and password have to be non - empty strings.')
        elif DATA_SEPARATOR in login:
            self.screen.errorLoginLabel.setText(f'Login cannot contain {DATA_SEPARATOR} signs.')
        else:
            message = self.server.sendLoginData(login, passwd)
            if message is None:
                self.calendarListWindow()
            else:
                self.screen.errorLoginLabel.setText(message)
        print('finishing thread...')

    def register(self) -> None:
        login, passwd = self.screen.loginRegisterField.text(), self.screen.passwordRegisterField.text()
        if login == '' or passwd == '':
            self.screen.errorRegisterLabel.setText('Login and password have to be non - empty strings.')
        elif DATA_SEPARATOR in login:
            self.screen.errorRegisterLabel.setText(f'Login cannot contain {DATA_SEPARATOR} signs.')
        else:
            message = self.server.sendRegisterData(login, passwd)
            if message is None:
                self.calendarListWindow()
            else:
                self.screen.errorRegisterLabel.setText(message)
    
    def logout(self) -> None:
        self.mainWindow.close()
        self.loginWindow()

    def exit(self) -> None:
        print('closing...')
        if self.thread.is_alive():
            print('waiting for thread...')
            self.thread.join()
        if self.server.connectionStatus:
            self.thread = threading.Thread(target=self.server.closing, args=(self.user.username,))
            self.thread.start()
            self.thread.join()
        print('closing app')



