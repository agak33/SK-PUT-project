from typing import Union, List

from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import QMessageBox
from setup import *
import socket
from calendar import Calendar
from dataModels.event import Event

APPLICATION_PORT = 1234
APPLICATION_HOST = 'localhost'

class ServerRequests():
    def __init__(self) -> None:
        super().__init__()
        self.socket:           socket   = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connectionStatus: bool     = False
        
        self.timer:            QTimer   = QTimer()

        self.messageToClient:   QMessageBox = QMessageBox()
        self.connect()

    def connect(self) -> Union[None, str]:
        try:
            print(f'Trying connect to {APPLICATION_HOST} port {APPLICATION_PORT}')
            self.socket.connect((APPLICATION_HOST, APPLICATION_PORT))
            self.connectionStatus = True
            print('Connected to serwer')
        except ConnectionRefusedError:
            self.connectionStatus = False
            self.messageToClient.about(self.messageToClient, 'ERROR OCCURED', 'Unable to connect to server')
            print('Unable to connect to server')

    def sendAndReceiveData(self, message: str) -> str:
        message += DATA_END
        if self.connectionStatus:
            print(f'Send to server: {message}')
            self.socket.sendall(message.encode())

            response = str(self.socket.recv(MAX_DATA_LENGTH))
            try:
                endIndex = response.index('\\x00')
                return response[2:endIndex]
            except ValueError:
                return response[2:-1]
        return ERROR_VALUE + DATA_SEPARATOR + 'Cannot connect to the server'

    def sendLoginData(self, login: str, passwd: str) -> Union[None, str]:
        return None
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([LOGIN_PREFIX, login, passwd]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendRegisterData(self, login: str, passwd: str) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([REGISTER_PREFIX, login, passwd]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendLogoutData(self, login: str) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([LOGOUT_PREFIX, login]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def closingApp(self, login: Union[str, None] = None) -> None:
        if login is None:
            self.socket.sendall(DATA_SEPARATOR.join([CLOSING_APP_PREFIX, ""]).encode())
        else:
            self.socket.sendall(DATA_SEPARATOR.join([CLOSING_APP_PREFIX, login]).encode())

    def sendNewCalendarData(self, name: str, owner: str, userList: List[str]) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_INSERT_PREFIX, name, owner, *userList]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendModifyCalendarData(self, oldName: str, newName: str, username: str) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_MODIFY_PREFIX, oldName, newName, username]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendDeleteCalendarData(self, name: str, username: str) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_DELETE_PREFIX, name, username]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendNewUserCalendarData(self, name: str, userToAdd: str, username: str) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_INSERT_USER_PREFIX, name, userToAdd, username]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendDeleteUserCalendarData(self, name: str, userToDelete:str, username: str) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_DELETE_USER_PREFIX, name, userToDelete, username]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def getCalendars(self, username: str) -> Union[str, List[str]]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_GET_NAMES_PREFIX, username]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        if len(response) < 3:
            return []
        return response[2:].split(DATA_SEPARATOR)

    def getCalendarInfo(self, name: str) -> Union[Calendar, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_GET_INFO_PREFIX, name]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]

        response = response[2:].split(DATA_SEPARATOR)
        if len(response) < 2:
            return 'Error occured'
        elif len(response) == 2:
            return Calendar(response[0], response[1])
        return Calendar(response[0], response[1], response[2:])

    def getEvents(self, calendar: str, date: str) -> Union[str, List[Event]]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_GET_EVENTS_PREFIX, calendar, date]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]

        if len(response) < 3:
            return []
        response = response[2:].split(DATA_SEPARATOR)
        result = []
        for i in range(0, len(response), 3):
            result.append(Event(response[i], date, response[i + 1], response[i + 2]))
        return result

    def getCalendarUserList(self, name: str) -> Union[str, List[str]]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([CALENDAR_GET_USERS_PREFIX, name]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        if len(response) < 3:
            return []
        return response[2:].split(DATA_SEPARATOR)

    def sendNewEventData(self, calendarName: str, event: Event) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([
                EVENT_INSERT_PREFIX, calendarName, event.owner, event.name, event.date, event.time
            ]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None
        
    def sendModifyEventData(self, calendarName: str, username: str, oldEvent: Event, newEvent: Event) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([
                EVENT_MODIFY_PREFIX, calendarName, username,
                oldEvent.name, oldEvent.date, oldEvent.time, oldEvent.owner,
                newEvent.name, newEvent.date, newEvent.time
            ]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendDeleteEventData(self, calendarName: str, username: str, event: Event) -> Union[None, str]:
        response = self.sendAndReceiveData(DATA_SEPARATOR.join([
                EVENT_DELETE_PREFIX, calendarName, username,
                event.name, event.date, event.time, event.owner
            ]))
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

