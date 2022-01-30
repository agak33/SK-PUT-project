from typing import Union, List

from PyQt5.QtCore import pyqtSignal, QTimer, QObject
from PyQt5.QtNetwork import QTcpSocket
from PyQt5.QtWidgets import QMessageBox
from setup import *
import socket
from dataModels.calendar import Calendar
from dataModels.event import Event

TIMER_TIME = 3000

class Client(QObject):
    dataToRead = pyqtSignal(str)

    userLogged = pyqtSignal()
    userLoggedOut = pyqtSignal()
    gotCalendarNames    = pyqtSignal(list)
    gotCalendarInfo     = pyqtSignal(Calendar)

    gotEventList        = pyqtSignal(str, list)
    gotCalendarUserList = pyqtSignal(list)

    calendarInsert      = pyqtSignal()
    calendarModify      = pyqtSignal()
    calendarDelete      = pyqtSignal()

    calendarUserInsert  = pyqtSignal()
    calendarUserDelete  = pyqtSignal()

    eventModify         = pyqtSignal()
    eventDelete         = pyqtSignal()
    eventInsert         = pyqtSignal()

    def __init__(self, APPLICATION_PORT: int, APPLICATION_HOST: str) -> None:
        super().__init__()
        self.socket:            QTcpSocket  = QTcpSocket()
        self.messageToClient:   QMessageBox = QMessageBox()
        self.timer:             QTimer      = QTimer()

        self.messageFromServer: str         = None
        self.message:           str         = ''

        self.signals = {
            LOGIN_PREFIX:    self.logged,
            REGISTER_PREFIX: self.logged,

            CALENDAR_INSERT_PREFIX: self.calendarInserted,
            CALENDAR_MODIFY_PREFIX: self.calendarModified,
            CALENDAR_DELETE_PREFIX: self.calendarDeleted,

            CALENDAR_INSERT_USER_PREFIX: self.calendarUserInserted,
            CALENDAR_DELETE_USER_PREFIX: self.calendarUserDeleted,

            CALENDAR_GET_NAMES_PREFIX:  self.calendarNames,
            CALENDAR_GET_INFO_PREFIX:   self.calendarInfo,
            CALENDAR_GET_EVENTS_PREFIX: self.eventList,
            CALENDAR_GET_USERS_PREFIX: self.calendarUsers,

            EVENT_INSERT_PREFIX: self.eventInserted,
            EVENT_MODIFY_PREFIX: self.eventModified,
            EVENT_DELETE_PREFIX: self.eventDeleted,
            
            LOGOUT_PREFIX: self.logoutUser
        }

        self.setupTimer()
        self.setupSocket(APPLICATION_PORT, APPLICATION_HOST)

    def setupTimer(self):
        self.timer.timeout.connect(self.timeout)
    
    def setupSocket(self, APPLICATION_PORT: int, APPLICATION_HOST: str):
        self.socket.connectToHost(APPLICATION_HOST, APPLICATION_PORT)
        self.timer.start(TIMER_TIME)

        self.socket.error.connect(self.error)
        self.socket.readyRead.connect(self.readData)
        self.socket.disconnected.connect(self.disconnected)
        self.socket.connected.connect(self.connected)

    def timeout(self):
        self.messageToClient.about( self.messageToClient, 
                                    'Server error', 
                                    'Cannot reach the server. The app will close...')
        exit(0)

    def error(self):
        self.messageToClient.about( self.messageToClient, 
                                    'Error',
                                    'Cannot connect with the serwer')
        exit(0)

    def disconnected(self):
        print('DISCONNECTED')
        exit(0)

    def connected(self):
        self.timer.stop()
        print('CONNECTED TO SERVER')

    def getMessage(self) -> Union[str, None]:
        try:
            dataEndIndex = self.message.index(DATA_END)
            self.timer.stop()
            result = self.message[:dataEndIndex]
            self.message = self.message[dataEndIndex + 1 :]
            return result
        except ValueError:
            return None

    def readData(self) -> None:
        response = list(str(self.socket.readAll(), encoding='utf-8'))
        for char in response:
            if char != '\x00':
                self.message += char
        result = self.getMessage()
        while result is not None:
            prefix = result[:result.index(DATA_SEPARATOR)]
            result = result[result.index(DATA_SEPARATOR) + 1:]

            if result[0] == ERROR_VALUE:
                self.messageToClient.about( self.messageToClient,
                                            'Error',
                                            result[2:])
            else:
                self.messageFromServer = result[2:]
                self.signals[prefix]()
            result = self.getMessage()

    def writeData(self, prefix, *args):
        self.socket.write(
            (DATA_SEPARATOR.join([prefix, *args]) + DATA_END).encode()
        )
        self.timer.start(TIMER_TIME)
        print('data sent:', prefix, *args)

    def logged(self):
        self.userLogged.emit()
        self.messageFromServer = ''

    def calendarNames(self):
        self.gotCalendarNames.emit(
            self.messageFromServer.split(DATA_SEPARATOR)
        )
        self.messageFromServer = ''

    def calendarInserted(self):
        self.calendarInsert.emit()
        self.messageFromServer = ''

    def calendarDeleted(self):
        self.calendarDelete.emit()
        self.messageFromServer = ''

    def calendarInfo(self):
        result = self.messageFromServer.split(DATA_SEPARATOR)        
        
        self.gotCalendarInfo.emit(Calendar(result[0], result[1], result[2:]))
        self.messageFromServer = ''

    def eventList(self):
        result = self.messageFromServer.split(DATA_SEPARATOR)
        eventList = []
        print(result)
        for i in range(1, len(result), 3):
            eventList.append(Event(result[i], result[0], result[i + 1], result[i + 2]))
            
        self.gotEventList.emit(result[0], eventList)
        self.messageFromServer = ''
        
    def eventInserted(self):
        self.eventInsert.emit()
        self.messageFromServer = ''
        
    def eventModified(self):
        self.eventModify.emit()
        self.messageFromServer = ''
        
    def eventDeleted(self):
        self.eventDelete.emit()
        self.messageFromServer = ''
        
    def calendarUsers(self):
        self.gotCalendarUserList.emit(self.messageFromServer.split(DATA_SEPARATOR))
        self.messageFromServer = ''
        
    def calendarUserInserted(self):
        self.calendarUserInsert.emit()
        
    def calendarUserDeleted(self):
        self.calendarUserDelete.emit()
        
    def calendarModified(self):
        self.calendarModify.emit()
        self.messageFromServer = ''
        
    def logoutUser(self):
        self.userLoggedOut.emit()

