
from dataModels.userInfo import UserInfo
from dataModels.event import Event
from calendar import Calendar
from client import Client
from setup import *

from PyQt5.QtCore import QDate, QDateTime, QLocale, QTime, pyqtSignal, QObject
from PyQt5 import QtWidgets, uic
from datetime import date

from typing import Union, Any, List

LOGIN_WIDGET = 'login_widget'
CALENDAR_LIST_WIDGET = 'calendar_list_widget'
NEW_CALENDAR_WIDGET = 'new_calendar_widget'
CALENDAR_WIDGET = 'calendar_widget'

class App(QObject):
    def __init__(self, APPLICATION_PORT: int, APPLICATION_HOST: str) -> None:
        super().__init__()
        
        self.app            = QtWidgets.QApplication([])
        self.mainWindow     = QtWidgets.QMainWindow()
        self.screen         = QtWidgets.QWidget()
        self.messageBox     = QtWidgets.QMessageBox()
        self.app.aboutToQuit.connect(self.exit)       

        self.client         = Client(APPLICATION_PORT, APPLICATION_HOST)
        self.client.dataToRead.connect(self.readData)

        self.client.userLogged.connect(self.calendarListWindow)
        self.client.userLoggedOut.connect(self.loginWindow)
        self.client.gotCalendarNames.connect(self.displayCalendarList)

        self.client.calendarInsert.connect(self.newCalendarInserted)
        self.client.calendarModify.connect(self.calendarListWindow)
        self.client.calendarDelete.connect(self.refreshCalendarList)

        self.client.gotCalendarInfo.connect(self.displayCalendarInfo)
        self.client.gotEventList.connect(self.displayEvents)
        
        self.client.eventInsert.connect(self.getEventsForDay)
        self.client.eventModify.connect(self.getEventsForDay)
        self.client.eventDelete.connect(self.getEventsForDay)
        
        self.client.gotCalendarUserList.connect(self.displayUsers)
        
        self.client.calendarUserDelete.connect(self.getCalendarUserList)
        self.client.calendarUserInsert.connect(self.getCalendarUserList)

        self.user           = UserInfo()
        self.calendarList   = []
        self.eventList      = []

        self.currentWidget: str = ''

        self.setupMainWindow()
        self.loginWindow()

    def readData(self, message: str):
        print(message)

    def loadScreen(self, path: str) -> None:
        self.screen = QtWidgets.QWidget()
        uic.loadUi(path, self.screen)

    def setupMainWindow(self) -> None:
        uic.loadUi(f'{SCREEN_MODELS_FOLDER}/{MAIN_APP_WINDOW}', self.mainWindow)
        self.mainWindow.actionCalendarList.triggered.connect(self.calendarListWindow)
        self.mainWindow.actionAddNewCalendar.triggered.connect(self.newCalendarWindow)
        self.mainWindow.actionLogout.triggered.connect(self.logout)

    #################################### LOGIN WINDOW ####################################

    def loginWindow(self) -> None:
        self.mainWindow.close()
        self.currentWidget = LOGIN_WIDGET
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_LOGIN_REGISTER}')
        self.screen.loginButton.clicked.connect(self.login)
        self.screen.registerButton.clicked.connect(self.register)
        self.screen.show()

    def login(self) -> None:
        login, passwd = self.screen.loginLoginField.text(), self.screen.passwordLoginField.text()
        if login == '' or passwd == '':
            self.messageBox.about(
                self.messageBox, 'Error',
                'Login and password have to be non - empty strings.'
            )
        elif DATA_SEPARATOR in login:
            self.messageBox.about(
                self.messageBox, 'Error',
                f'Login cannot contains {DATA_SEPARATOR} characters.'
            )
        elif len(login.strip()) != len(login):
            self.messageBox.about(
                self.messageBox, 'Error',
                'Login cannot contains white characters at the begginning and at the end.'
            )
        else:
            self.client.writeData(LOGIN_PREFIX, login, passwd)
            self.user.setUserData(login, passwd)

    def register(self) -> None:
        login, passwd = self.screen.loginRegisterField.text(), self.screen.passwordRegisterField.text()
        if login == '' or passwd == '':
            self.messageBox.about(
                self.messageBox, 'Error',
                'Login and password have to be non - empty strings.'
            )
        elif DATA_SEPARATOR in login:
            self.messageBox.about(
                self.messageBox, 'Error',
                f'Login cannot contains {DATA_SEPARATOR} characters.'
            )
        elif len(login.strip()) != len(login):
            self.messageBox.about(
                self.messageBox, 'Error',
                'Login cannot contains white characters at the begginning and at the end.'
            )
        else:
            self.client.writeData(REGISTER_PREFIX, login, passwd)
            self.user.setUserData(login, passwd)

    ################################ CALENDAR LIST WINDOW ################################

    def newCalendarInserted(self):
        self.messageBox.about(
            self.messageBox, 'Success',
            'New calendar has been inserted successfully.'
        )
        if self.currentWidget == NEW_CALENDAR_WIDGET:
            self.screen.newCalendarNameField.setText('')
            self.screen.newCalendarUserListField.setText('')
        self.refreshCalendarList()

    def displayCalendarList(self, calendars: List[str]):
        if self.currentWidget == CALENDAR_LIST_WIDGET:
            self.screen.calendarList.clear()
            self.screen.calendarList.addItems(calendars)
            
            if self.screen.calendarNameLabel.text() not in calendars:
                self.screen.calendarInfoFrame.setEnabled(False)

    def refreshCalendarList(self):
        if self.currentWidget == CALENDAR_LIST_WIDGET:
            self.client.writeData(CALENDAR_GET_NAMES_PREFIX, self.user.username)

    def calendarListWindow(self) -> None:
        self.currentWidget = CALENDAR_LIST_WIDGET
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_CALENDAR_LIST}')  
        self.client.writeData(CALENDAR_GET_NAMES_PREFIX, self.user.username)      

        self.screen.calendarList.itemClicked.connect(
            lambda: self.client.writeData(
                CALENDAR_GET_INFO_PREFIX, 
                self.screen.calendarList.currentItem().text()
                )
            )
        self.screen.calendarOpenButton.clicked.connect(self.openCalendar)
        self.screen.calendarDeleteButton.clicked.connect(self.deleteCalendar)

        self.mainWindow.setCentralWidget(self.screen)
        self.mainWindow.show()

    def displayCalendarInfo(self, calendar: Calendar) -> None:
        if self.currentWidget == CALENDAR_LIST_WIDGET:
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
                print('Calendar not exists')
                calendarList = self.server.getCalendars(self.user.username)
                self.screen.calendarList.clear()
                self.screen.calendarList.addItems(calendarList)

    def openCalendar(self) -> None:
        self.calendarWindow(
            self.screen.calendarNameLabel.text(),
            self.screen.calendarOwnerLabel.text()
        )

    def deleteCalendar(self) -> None:
        self.client.writeData(
            CALENDAR_DELETE_PREFIX,
            self.screen.calendarNameLabel.text(),
            self.user.username
        )

    def newCalendarWindow(self) -> None:
        self.currentWidget = NEW_CALENDAR_WIDGET
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_NEW_CALENDAR}')

        self.screen.newCalendarConfirmButton.clicked.connect(self.newCalendar)

        self.mainWindow.setCentralWidget(self.screen)
        self.mainWindow.show()

    def newCalendar(self) -> None:
        name = self.screen.newCalendarNameField.text()
        userList = self.screen.newCalendarUserListField.text()
        if name == '':
            self.messageBox.about(
                self.messageBox, 'Error',
                'Calendar name cannot be empty'
            )
        elif DATA_SEPARATOR in name:
            self.messageBox.about(
                self.messageBox, 'Error',
                f'Calendar cannot contains {DATA_SEPARATOR} characters.'
            )
        elif len(name.strip()) != len(name):
            self.messageBox.about(
                self.messageBox, 'Error',
                'Calendar name cannot contains white characters at the begginning and at the end.'
            )
        else:
            userList = userList.split(DATA_SEPARATOR)
            for i in range(len(userList)):
                userList[i] = userList[i].strip()
            try:
                while len(userList) > 0:
                    userList.remove('')
            except ValueError:
                pass
            self.client.writeData(
                CALENDAR_INSERT_PREFIX, name, self.user.username, *userList
            )

    ################################### CALENDAR VIEW ###################################

    def getEventsForDay(self) -> None:
        self.client.writeData(  CALENDAR_GET_EVENTS_PREFIX,
                                self.screen.calendarNameLabel.text(), 
                                self.screen.calendar.selectedDate().toString(DATE_FORMAT))

    def getCalendarUserList(self):
        if self.currentWidget == CALENDAR_WIDGET:
            self.client.writeData(CALENDAR_GET_USERS_PREFIX, self.screen.calendarNameLabel.text())

    def calendarWindow(self, name: str, owner: str) -> None:
        self.currentWidget = CALENDAR_WIDGET
        self.loadScreen(f'{SCREEN_MODELS_FOLDER}/{SCREEN_CALENDAR_VIEW}')

        # calendar setup
        today = date.today().strftime('%Y %m %d').split()
        self.screen.calendar.setLocale(QLocale(QLocale.English))
        self.screen.calendar.setSelectedDate(QDate(*[int(x) for x in today]))
        
        self.screen.calendar.clicked.connect(self.getEventsForDay)
        self.screen.calendarNameLabel.setText(name)
        self.screen.calendarOwnerLabel.setText(owner)

        # event tab setup
        self.getEventsForDay()
        self.screen.eventFrame.setEnabled(False)
        self.screen.deleteEventButton.setEnabled(False)

        self.screen.eventList.itemClicked.connect(self.displayEventInfo)
        self.screen.modifyEventButton.clicked.connect(self.modifyEvent)
        self.screen.deleteEventButton.clicked.connect(self.deleteEvent)

        # event edit setup
        self.screen.newEventConfirmButton.clicked.connect(self.addEvent)

        # # calendar user list setup
        self.screen.userList.clear()
        self.getCalendarUserList()
        if self.screen.calendarOwnerLabel.text() == self.user.username:
            self.screen.modifyUserListFrame.setEnabled(True)
            self.screen.deleteUserButton.setEnabled(False)
        else:
            self.screen.modifyUserListFrame.setEnabled(False)

        self.screen.userList.itemClicked.connect(self.displayUserOption)
        self.screen.deleteUserButton.clicked.connect(self.deleteUser)
        self.screen.addUserButton.clicked.connect(self.addUser)

        # # tab 4
        if owner != self.user.username:
            self.screen.calendarTabs.setTabEnabled(3, False)
            self.screen.calendarTabs.setTabEnabled(1, False)
        self.screen.newCalendarNameButton.clicked.connect(self.newCalendarName)

        self.mainWindow.setCentralWidget(self.screen)
        self.mainWindow.show()
        
    def displayEvents(self, date: str, events: List[Event]) -> None:
        self.eventList = events
        if self.currentWidget == CALENDAR_WIDGET and date == self.screen.calendar.selectedDate().toString(DATE_FORMAT):
            self.screen.eventList.clear()
            self.screen.eventFrame.setEnabled(False)
            self.screen.deleteEventButton.setEnabled(False)
            self.eventList.sort(key=lambda x: x.time)
            for event in self.eventList:
                self.screen.eventList.addItem(f'{event.time}: {event.name}')
                
            date = [int(x) for x in date.split('.')]
            self.screen.newEventDateTime.setDateTime(
                QDateTime(QDate(date[2], date[1], date[0]), QTime(0, 0))
            )
            self.mainWindow.setCentralWidget(self.screen)
            self.mainWindow.show()
    
    def modifyEvent(self) -> None:
        currRowIndex = self.screen.eventList.currentRow()
        oldEvent = self.eventList[currRowIndex]
        date, time = self.screen.eventDateTimeField.text().split()
        newEvent = Event(self.screen.eventNameField.text().strip(),
                         date, time, self.screen.eventOwnerLabel.text())

        if newEvent == oldEvent:
            self.messageBox.about(
                self.messageBox, 'Error',
                'To modify event you need to change its data.'
            )
        elif newEvent.name == '':
            self.messageBox.about(
                self.messageBox, 'Error',
                'Event name cannot be empty.'
            )
        elif DATA_SEPARATOR in newEvent.name:
            self.messageBox.about(
                self.messageBox, 'Error',
                f'Event name cannot contains {DATA_SEPARATOR} characters.'
            )
        else:
            self.client.writeData(
                EVENT_MODIFY_PREFIX, self.screen.calendarNameLabel.text(), self.user.username, 
                oldEvent.name, oldEvent.date, oldEvent.time, oldEvent.owner,
                newEvent.name, newEvent.date, newEvent.time
            )

    def displayEventInfo(self) -> None:
        currRowIndex = self.screen.eventList.currentRow()
        currEvent = self.eventList[currRowIndex]
        date = [int(x) for x in currEvent.date.split('.')]
        time = [int(x) for x in currEvent.time.split(':')]
        self.screen.eventDateTimeField.setDateTime(
            QDateTime(
                QDate(date[2], date[1], date[0]), 
                QTime(*time)
            ))
        self.screen.eventFrame.setEnabled(True)
        self.screen.eventNameField.setText(currEvent.name)
        self.screen.eventOwnerLabel.setText(currEvent.owner)

        if self.user.username == self.screen.calendarOwnerLabel.text():
            self.screen.modifyEventButton.setEnabled(True)
            self.screen.deleteEventButton.setEnabled(True)
        else:
            self.screen.modifyEventButton.setEnabled(False)
            self.screen.deleteEventButton.setEnabled(False)

    def deleteEvent(self) -> None:
        currRowIndex = self.screen.eventList.currentRow()
        currEvent = self.eventList[currRowIndex]

        self.client.writeData(
            EVENT_DELETE_PREFIX, self.screen.calendarNameLabel.text(), self.user.username,
            currEvent.name, currEvent.date, currEvent.time, currEvent.owner
        )

    def addEvent(self) -> None:
        name = self.screen.newEventNameField.text().strip()
        date, time = self.screen.newEventDateTime.text().split()

        if name == '':
            self.messageBox.about(
                self.messageBox, 'Error',
                'Event name cannot be empty.'
            )
        elif DATA_SEPARATOR in name:
            self.messageBox.about(
                self.messageBox, 'Error',
                f'Event name cannot contains {DATA_SEPARATOR} characters.'
            )
        else:
            self.client.writeData(EVENT_INSERT_PREFIX, 
                                  self.screen.calendarNameLabel.text(),
                                  self.user.username,
                                  name, date, time)
    
    def displayUserOption(self) -> None:
        if self.user.username == self.screen.calendarOwnerLabel.text():
            self.screen.deleteUserButton.setEnabled(True)
        else:
            self.screen.deleteUserButton.setEnabled(False)

    def displayUsers(self, userList: List[str]) -> None:
        if self.currentWidget == CALENDAR_WIDGET:
            self.screen.userList.clear()
            self.screen.userList.addItems(sorted(userList, key=lambda x: x.upper()))
            if self.screen.calendarOwnerLabel.text() == self.user.username:
                self.screen.modifyUserListFrame.setEnabled(True)
                self.screen.deleteUserButton.setEnabled(False)
            else:
                self.screen.modifyUserListFrame.setEnabled(False)
            self.mainWindow.setCentralWidget(self.screen)
            self.mainWindow.show()

    def deleteUser(self) -> None:
        name = self.screen.userList.currentItem().text()
        self.client.writeData(CALENDAR_DELETE_USER_PREFIX, 
                              self.screen.calendarNameLabel.text(),
                              name, self.user.username)

    def addUser(self) -> None:
        name = self.screen.newUserNameField.text().strip()
        if name == '' or DATA_SEPARATOR in name:
            self.messageBox.about(
                self.messageBox, 'Error',
                'Wrong username.'
            )
        else:
            self.client.writeData(CALENDAR_INSERT_USER_PREFIX, 
                                  self.screen.calendarNameLabel.text(),
                                  name, self.user.username)

    def newCalendarName(self) -> None:
        newName = self.screen.newCalendarNameField.text().strip()
        if newName == '':
            self.messageBox.about(
                self.messageBox, 'Error',
                'Calendar name cannot be empty.'
            )
        elif DATA_SEPARATOR in newName:
            self.messageBox.about(
                self.messageBox, 'Error',
                f'Calendar name cannot contains {DATA_SEPARATOR} characters.'
            )
        else:
            self.client.writeData(CALENDAR_MODIFY_PREFIX,
                                  self.screen.calendarNameLabel.text(),
                                  newName,
                                  self.user.username)

    ####################################### OTHER #######################################

    def logout(self) -> None:
        if self.user.username is None:
            self.client.writeData(LOGOUT_PREFIX)
        else:
            self.client.writeData(LOGOUT_PREFIX, self.user.username)
            self.user.deleteUserData()

    def exit(self) -> None:
        if self.user.username is None:
            self.client.writeData(CLOSING_APP_PREFIX)
        else:
            self.client.writeData(CLOSING_APP_PREFIX, self.user.username)
        self.client.socket.close()
        self.screen.close()
        self.mainWindow.close()

    

