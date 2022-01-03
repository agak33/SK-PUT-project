from calendar import Calendar
from setup import *
from serverRequests import ServerRequests
from PyQt5 import QtWidgets, uic

from userInfo import UserInfo

class App():
    def __init__(self) -> None:
        self.app            = QtWidgets.QApplication([])
        self.server         = ServerRequests()
        self.screen         = QtWidgets.QWidget()
        self.user           = UserInfo()
        self.calendarList   = []
        self.eventList      = []

        self.loginWindow()

    def loadScreen(self, path: str) -> None:
        self.screen.close()
        self.screen = QtWidgets.QWidget()
        uic.loadUi(path, self.screen)

    def loginWindow(self) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_LOGIN_REGISTER}')        

        if self.server.errorMessage is not None:
            self.screen.errorLoginLabel.setText("Error:\n" + self.server.errorMessage);
            self.screen.errorRegisterLabel.setText("Error:\n" + self.server.errorMessage);
            self.server.errorMessage = None

        self.screen.loginButton.clicked.connect(self.login)
        self.screen.registerButton.clicked.connect(self.register)
        self.screen.show()

    def userWindow(self) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_USER_VIEW}')

        self.displayCalendars()

        self.screen.addCalendarButton.clicked.connect(self.addCalendar)
        self.screen.newCalendarConfirmButton.clicked.connect(self.addCalendarConfirm)
        self.screen.logoutButton.clicked.connect(self.logout)
        self.screen.calendarList.itemClicked.connect(self.displayCalendarData)
        self.screen.calendarOpenButton.clicked.connect(lambda: self.openCalendar(self.screen.calendarNameLabel.text()))
        self.screen.calendarDeleteButton.clicked.connect(lambda: self.deleteCalendar(self.screen.calendarNameLabel.text()))        

        self.screen.show()

    def calendarWindow(self, calendar: Calendar) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_CALENDAR_VIEW}')

        self.screen.calendarNameLabel.setText(calendar.name)
        self.screen.calendarOwnerLabel.setText(calendar.owner)

        self.screen.goBackButton.clicked.connect(self.goBack)
        self.screen.newEventButton.clicked.connect(self.newEvent)
        self.screen.newEventConfirmButton.clicked.connect(self.newEventConfirm)

        self.screen.show()


    ############################################################################

    def login(self) -> None:
        name, passwd = self.screen.loginLoginField.text(), self.screen.passwordLoginField.text()

        # send to server, receive answer
        message = self.server.sendLoginData(name, passwd)

        if message is not None:
            self.screen.errorLoginLabel.setText(message)
        else:
            self.user.setUserData(name, passwd)
            self.userWindow()

    def register(self) -> None:
        name, passwd = self.screen.loginLoginField.text(), self.screen.passwordLoginField.text()

        # send to server, receive answer
        message = self.server.sendRegisterData(name, passwd)

        if message is not None:
            self.screen.errorRegisterLabel.setText(message)
        else:
            self.user.setUserData(name, passwd)
            self.userWindow()

    def logout(self) -> None:
        # send to server, receive answer
        message = self.server.sendLogoutData(self.user.username)

        if message is not None:
            self.screen.errorLabel.setText(message)
        else:
            self.user.deleteUserData()
            self.loginWindow()

    def addCalendar(self) -> None:
        self.screen.newCalendarFrame.setEnabled(True)

    def addCalendarConfirm(self) -> None:
        # send to server
        message = self.server.sendNewCalendarData(
            self.screen.newCalendarNameField.text(), 
            self.screen.newCalendarUserListField.text()
        )
        self.screen.newCalendarFrame.setEnabled(False)

    def openCalendar(self, calendarName: str) -> None:
        for calendar in self.calendarList:
            if calendar.name == calendarName:
                self.calendarWindow(calendar)
                break

    def deleteCalendar(self, calendarName: str) -> None:
        pass

    def goBack(self) -> None:
        self.userWindow()

    def newEvent(self) -> None:
        self.screen.newEventFrame.setEnabled(True)

    def newEventConfirm(self) -> None:
        self.screen.newEventFrame.setEnabled(False)

    ############################################################################

    def displayCalendars(self) -> None:
        self.calendarList = self.server.getCalendars()
        for calendar in self.calendarList:
            self.screen.calendarList.addItem(calendar.name)

    def displayCalendarData(self, item) -> None:
        self.screen.calendarUserList.clear()
        for calendar in self.calendarList:
            if calendar.name == item.text():
                self.screen.calendarNameLabel.setText(calendar.name)
                self.screen.calendarOwnerName.setText(calendar.owner)
                for user in calendar.userList:
                    self.screen.calendarUserList.addItem(user)
                break
        self.screen.calendarInfoFrame.setEnabled(True)