from calendar import Calendar

from PyQt5.QtCore import QLocale, QThread
from setup import *
from serverRequests import ServerRequests
from PyQt5 import QtWidgets, uic
from functools import partial
import threading
from typing import Union, Any, List
from calendar import Calendar
from event import Event

from userInfo import UserInfo

class App(object):
    def __init__(self, APPLICATION_PORT: int, APPLICATION_HOST: str) -> None:
        self.app            = QtWidgets.QApplication([])
        self.app.aboutToQuit.connect(self.exit)
        self.mainWindow     = QtWidgets.QMainWindow()
        self.screen         = QtWidgets.QWidget()

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

    def calendarWindow(self, name: str, owner: str) -> None:
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_CALENDAR_VIEW}')

        self.screen.calendar.setLocale(QLocale(QLocale.English))
        self.screen.calendar.clicked.connect(self.displayEvents)
        self.screen.calendarNameLabel.setText(name)
        self.screen.calendarOwnerLabel.setText(owner)

        # tab 1
        self.displayEvents()        
        self.screen.eventFrame.setEnabled(True)
        self.screen.deleteEventButton.setEnabled(False)

        self.screen.eventList.itemClicked.connect(self.displayEventInfo)
        self.screen.modifyEventButton.clicked.connect(self.modifyEvent)
        self.screen.deleteEventButton.clicked.connect(self.deleteEvent)

        # tab 2
        #self.screen.newEventDateTime.setText()
        self.screen.newEventConfirmButton.clicked.connect(self.addEvent)

        # tab 3
        self.displayUsers()
        self.screen.modifyUserListFrame.setEnabled(False)

        self.screen.userList.itemClicked.connect(self.displayUserOption)
        self.screen.deleteUserButton.clicked.connect(self.deleteUser)
        self.screen.addUserButton.clicked.connect(self.addUser)

        # tab 4
        if owner != self.user.username:
            self.screen.calendarTabs.setTabVisible(3, False)
        self.screen.newCalendarNameButton.clicked.connect(self.newCalendarName)

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
        if self.server.getCalendarInfo(self.screen.calendarNameLabel.text()) is not None:
            self.calendarWindow(self.screen.calendarNameLabel.text(), self.screen.calendarOwnerLabel.text())
        else:
            print('CALENDAR DOESNT EXISTS')

    def deleteCalendar(self) -> None:
        message = self.server.sendDeleteCalendarData(self.screen.calendarNameLabel.text(), self.user.username)
        if message is None:
            self.screen.errorLabel.setText('Calendar deleted')
            calendarList = self.server.getCalendars(self.user.username)
            self.screen.calendarList.clear()
            self.screen.calendarList.addItems(calendarList)
        else:
            self.screen.errorLabel.setText(message)

    def modifyEvent(self) -> None:
        currRowIndex = self.screen.eventList.currentRow()
        oldEvent = self.eventList[currRowIndex]
        date, time = self.screen.eventDateTimeField.text().split()
        newEvent = Event(self.screen.eventNameField.text(),
                         date, time, self.screen.eventOwnerLabel.text())

        if newEvent == oldEvent:
            print('to modify event you need to change its data')
        else:
            message = self.server.sendModifyEventData(
                                self.screen.calendarNameLabel.text(), self.user.username,
                                oldEvent, newEvent
                    )
            if message is None:
                self.displayEvents()
            elif message is not None:
                print(message)

    def displayEventInfo(self) -> None:
        currRowIndex = self.screen.eventList.currentRow()
        currEvent = self.eventList[currRowIndex]
        self.screen.eventDateTimeField.setText(f'{currEvent.date} {currEvent.time}')
        self.screen.eventNameField.setText(currEvent.name)
        self.screen.eventOwnerLabel.setText(currEvent.owner)

        if self.user.username == currEvent.owner or self.user.username == self.screen.calendarOwnerLabel.text():
            self.screen.modifyEventButton.setEnabled(True)
            self.screen.deleteEventButton.setEnabled(True)
        else:
            self.screen.modifyEventButton.setEnabled(False)
            self.screen.deleteEventButton.setEnabled(False)

    def displayEvents(self) -> None:
        self.screen.eventList.clear()
        self.eventList = self.server.getEvents(
            self.screen.calendarNameLabel.text(), 
            self.screen.calendar.selectedDate().toString(DATE_FORMAT)
        )
        if self.eventList is not None:
            self.eventList.sort(key=lambda x: x.time)
            for event in self.eventList:
                self.screen.eventList.addItem(f'{event.time}: {event.name}')

    def deleteEvent(self) -> None:
        currRowIndex = self.screen.eventList.currentRow()
        currEvent = self.eventList[currRowIndex]
        message = self.server.sendDeleteEventData(self.screen.calendarNameLabel.text(), self.user.username, currEvent)
        if message is None:
            self.displayEvents()
        else:
            print(message)

    def addEvent(self) -> None:
        name = self.screen.newEventNameField.text().strip()
        date, time = self.screen.newEventDateTime.text().split()

        if name == '':
            print('empty name')
        elif DATA_SEPARATOR in name:
            print('data separator in event name')
        else:
            message = self.server.sendNewEventData(
                self.screen.calendarNameLabel.text(), 
                Event(name, date, time, self.user.username)
            )
            if message is None:
                self.displayEvents()
            else:
                print(message)

    def displayUserOption(self) -> None:
        if self.user.username == self.screen.calendarOwnerLabel.text():
            self.screen.modifyUserListFrame.setEnabled(True)
        else:
            self.screen.modifyUserListFrame.setEnabled(False)

    def displayUsers(self) -> None:
        self.screen.userList.clear()
        message = self.server.getCalendarUserList(self.screen.calendarNameLabel.text())
        if isinstance(message, str):
            print(message)
        else:
            self.screen.userList.addItems(message)

    def deleteUser(self) -> None:
        name = self.screen.userList.currentItem.text()
        message = self.server.sendDeleteUserCalendarData(self.screen.calendarNameLabel.text(), name, self.user.username)
        if message is None:
            self.displayUsers()
        else:
            print(message)

    def addUser(self) -> None:
        name = self.screen.newUserNameField.text()
        if name == '' or DATA_SEPARATOR in name:
            print('wrong username')
        else:
            message = self.server.sendNewUserCalendarData(self.screen.calendarNameLabel.text(), name, self.user.username)
            if message is None:
                self.displayUsers()
            else:
                print(message)

    def newCalendarName(self) -> None:
        newName = self.screen.newCalendarNameField.text()
        if newName == '':
            print('pusto')
        elif DATA_SEPARATOR in newName:
            print('data separator')
        else:
            message = self.server.sendModifyCalendarData(self.screen.calendarNameLabel.text(), newName, self.user.username)
            if message is None:
                self.calendarWindow(newName, self.screen.calendarOwnerLabel.text())
            else:
                print(message)

