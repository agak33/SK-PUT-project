from calendar import Calendar
from setup import *
from serverRequests import ServerRequests
from PyQt5 import QtWidgets, uic
from functools import partial
import threading
from typing import Union, Any, List
from calendar import Calendar

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

        self.mainWindow.actionCalendarList.triggered.connect(
            lambda: self.calendarListWindow(self.server.getCalendars(self.user.username))
        )
        self.mainWindow.actionAddNewCalendar.triggered.connect(self.newCalendarWindow)
        self.mainWindow.actionLogout.triggered.connect(self.logout)

    def loadScreen(self, path: str) -> None:
        self.screen = QtWidgets.QWidget()
        uic.loadUi(path, self.screen)

    def loginWindow(self) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_LOGIN_REGISTER}')
        self.screen.loginButton.clicked.connect(self.login)
        self.screen.registerButton.clicked.connect(self.register)
        self.screen.show()

    def calendarListWindow(self, calendarList: List[str]) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_CALENDAR_LIST}')
        self.screen.calendarList.clear()
        self.screen.calendarList.addItems(calendarList)

        self.screen.calendarList.itemClicked.connect(self.displayCalendarInfo)
        self.screen.calendarOpenButton.clicked.connect(self.openCalendar)
        self.screen.calendarDeleteButton.clicked.connect(self.deleteCalendar)
        self.mainWindow.setCentralWidget(self.screen)
        self.mainWindow.show()

    def displayCalendarInfo(self) -> None:
        calendar: Calendar = self.server.getCalendarInfo(self.screen.calendarList.currentItem().text())
        self.screen.calendarUserList.clear()
        if calendar is not None:
            self.screen.calendarInfoFrame.setEnabled(True)
            self.screen.calendarNameLabel.setText(calendar.name)
            self.screen.calendarOwnerLabel.setText(calendar.owner)
            self.screen.calendarUserList.addItems(calendar.userList)

            if calendar.owner != self.user.username:
                self.screen.calendarDeleteButton.setEnabled(False)
            else:
                self.screen.calendarDeleteButton.setEnabled(True)
        else:
            self.screen.errorLabel.setText('Calendar not exists')
            calendarList = self.server.getCalendars(self.user.username)
            self.screen.calendarList.clear()
            self.screen.calendarList.addItems(calendarList)

    def newCalendarWindow(self) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_NEW_CALENDAR}')
        self.screen.newCalendarConfirmButton.clicked.connect(self.newCalendar)
        self.mainWindow.setCentralWidget(self.screen)
        self.mainWindow.show()  

    def login(self) -> None:
        login, passwd = self.screen.loginLoginField.text(), self.screen.passwordLoginField.text()
        if login == '' or passwd == '':
            self.screen.errorLoginLabel.setText('Login and password have to be non - empty strings.')
        elif DATA_SEPARATOR in login:
            self.screen.errorLoginLabel.setText(f'Login cannot contain {DATA_SEPARATOR} signs.')
        else:
            message = self.server.sendLoginData(login, passwd)
            if message is None:
                self.user = UserInfo(login, passwd)
                self.calendarListWindow(self.server.getCalendars(login))
            else:
                self.screen.errorLoginLabel.setText(message)

    def register(self) -> None:
        login, passwd = self.screen.loginRegisterField.text(), self.screen.passwordRegisterField.text()
        if login == '' or passwd == '':
            self.screen.errorRegisterLabel.setText('Login and password have to be non - empty strings.')
        elif DATA_SEPARATOR in login:
            self.screen.errorRegisterLabel.setText(f'Login cannot contain {DATA_SEPARATOR} signs.')
        else:
            message = self.server.sendRegisterData(login, passwd)
            if message is None:
                self.user = UserInfo(login, passwd)
                self.calendarListWindow([])
            else:
                self.screen.errorRegisterLabel.setText(message)
    
    def logout(self) -> None:
        message = self.server.sendLogoutData(self.user.username)
        if message is None:
            self.user = UserInfo()
        else:
            print(message)
        self.mainWindow.close()
        self.loginWindow()

    def exit(self) -> None:
        if self.server.connectionStatus:
            self.server.closingApp(self.user.username)
        print('app closed')

    def newCalendar(self) -> None:
        name = self.screen.newCalendarNameField.text()
        userList = self.screen.newCalendarUserListField.text()
        if name == '':
            self.screen.errorLabel.setText('Calendar name cannot be empty')
        elif DATA_SEPARATOR in name:
            self.screen.errorLabel.setText(f'Calendar name cannot contain {DATA_SEPARATOR} signs')
        else:
            userList = userList.split(DATA_SEPARATOR)
            for i in range(len(userList)):
                userList[i] = userList[i].strip()
            try:
                while len(userList) > 0:
                    userList.remove('')
            except ValueError:
                pass
            message = self.server.sendNewCalendarData(name, self.user.username, userList)
            if message is None:
                self.screen.errorLabel.setText('Successfully added calendar')
            else:
                self.screen.errorLabel.setText(message)

    def openCalendar(self) -> None:
        print('open calendar...')

    def deleteCalendar(self) -> None:
        message = self.server.sendDeleteCalendarData(self.screen.calendarNameLabel.text(), self.user.username)
        if message is None:
            self.screen.errorLabel.setText('Calendar deleted')
            calendarList = self.server.getCalendars(self.user.username)
            self.screen.calendarList.clear()
            self.screen.calendarList.addItems(calendarList)
        else:
            self.screen.errorLabel.setText(message)

