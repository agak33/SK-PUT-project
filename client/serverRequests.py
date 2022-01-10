from typing import Union, List
from setup import *
import socket
from calendar import Calendar


class ServerRequests():
    def __init__(self) -> None:
        super().__init__()
        self.socket:           socket   = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connectionStatus: bool     = False
        self.connect()

    def connect(self) -> Union[None, str]:
        try:
            print(f'Trying connect to {APPLICATION_HOST} at the port {APPLICATION_PORT}')
            self.socket.connect((APPLICATION_HOST, APPLICATION_PORT))
            self.connectionStatus = True
            print('Connected to serwer')
        except ConnectionRefusedError:
            self.connectionStatus = False
            print('Unable to connect to server')

    def sendLoginData(self, login: str, passwd: str) -> Union[None, str]:
        if not self.connectionStatus:
            self.connect()

        if self.connectionStatus:
            message: str = DATA_SEPARATOR.join([LOGIN_PREFIX, login, passwd])
            print(f'Send to server: {message}')
            self.socket.sendall(message.encode())

            response = str(self.socket.recv(MAX_DATA_LENGTH))
            endIndex = response.index('\\x00')
            response = response[2:endIndex]
            print(f'Got response from server: {response}')
            if response[0] == ERROR_VALUE:
                return response[2:]
            return None
        return 'Cannot connect to the server'

    def sendRegisterData(self, login: str, passwd: str) -> Union[None, str]:
        if not self.connectionStatus:
            self.connect()

        if self.connectionStatus:
            message: str = DATA_SEPARATOR.join([REGISTER_PREFIX, login, passwd])
            print(f'Send to server: {message}')
            self.socket.sendall(message.encode())

            response = str(self.socket.recv(MAX_DATA_LENGTH))
            endIndex = response.index('\\x00')
            response = response[2:endIndex]
            print(f'Got response from server: {response}')
            if response[0] == ERROR_VALUE:
                return response[2:]
            return None
        return 'Cannot connect to the server'

    def sendLogoutData(self, login: str) -> Union[None, str]:
        message = DATA_SEPARATOR.join([LOGOUT_PREFIX, login])
        print(f'Send to server: {message}')
        self.socket.sendall(message.encode())

        response = str(self.socket.recv(MAX_DATA_LENGTH))
        endIndex = response.index('\\x00')
        response = response[2:endIndex]
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def closingApp(self, login: Union[str, None] = None) -> None:
        if login is None:
            self.socket.sendall(DATA_SEPARATOR.join([CLOSING_APP_PREFIX, ""]).encode())
        else:
            self.socket.sendall(DATA_SEPARATOR.join([CLOSING_APP_PREFIX, login]).encode())

    def sendNewCalendarData(self, name: str, owner: str, userList: List[str]):
        if self.connectionStatus:
            message = DATA_SEPARATOR.join([CALENDAR_INSERT_PREFIX, name, owner, *userList])
            print(f'Send to server: {message}')
            self.socket.sendall(message.encode())

            response = str(self.socket.recv(MAX_DATA_LENGTH))
            endIndex = response.index('\\x00')
            response = response[2:endIndex]
            print(f'Got response from server: {response}')
            if response[0] == ERROR_VALUE:
                return response[2:]
            return None
        return 'Cannot connect to the server'

    def sendModifyCalendarData(self, oldName: str, newName: str, username: str):
        message = DATA_SEPARATOR.join([CALENDAR_MODIFY_PREFIX, oldName, newName, username])
        print(f'Send to server: {message}')
        self.socket.sendall(message.encode())

        response = str(self.socket.recv(MAX_DATA_LENGTH))
        endIndex = response.index('\\x00')
        response = response[2:endIndex]
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendNewUserCalendarData(self, name: str, userToAdd: str, username: str):
        message = DATA_SEPARATOR.join([CALENDAR_INSERT_USER_PREFIX, name, userToAdd, username])
        self.socket.sendall(message.encode())

        response = str(self.socket.recv(MAX_DATA_LENGTH))
        endIndex = response.index('\\x00')
        response = response[2:endIndex]
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendDeleteUserCalendarData(self, name: str, userToDelete:str, username: str):
        message = DATA_SEPARATOR.join([CALENDAR_DELETE_USER_PREFIX, name, userToDelete, username])
        print(f'Send to server: {message}')
        self.socket.sendall(message.encode())

        response = str(self.socket.recv(MAX_DATA_LENGTH))
        endIndex = response.index('\\x00')
        response = response[2:endIndex]
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def sendDeleteCalendarData(self, name: str, username: str):
        message = DATA_SEPARATOR.join([CALENDAR_DELETE_PREFIX, name, username])
        print(f'Send to server: {message}')
        self.socket.sendall(message.encode())

        response = str(self.socket.recv(MAX_DATA_LENGTH))
        endIndex = response.index('\\x00')
        response = response[2:endIndex]
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return response[2:]
        return None

    def getCalendars(self, username: str) -> List[str]:
        message = DATA_SEPARATOR.join([CALENDAR_GET_NAMES, username])
        print(f'Send to server: {message}')
        self.socket.sendall(message.encode())

        response = str(self.socket.recv(MAX_DATA_LENGTH))
        endIndex = response.index('\\x00')
        response = response[2:endIndex]
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return []
        return response[2:].split(DATA_SEPARATOR)

    def getCalendarInfo(self, name: str) -> Union[Calendar, None]:
        message = DATA_SEPARATOR.join([CALENDAR_GET_CALENDAR_INFO, name])
        print(f'Send to server: {message}')
        self.socket.sendall(message.encode())

        response = str(self.socket.recv(MAX_DATA_LENGTH))
        endIndex = response.index('\\x00')
        response = response[2:endIndex]
        print(f'Got response from server: {response}')
        if response[0] == ERROR_VALUE:
            return None

        response = response[2:].split(DATA_SEPARATOR)
        if len(response) < 2:
            return None
        elif len(response) == 2:
            return Calendar(response[0], response[1])
        return Calendar(response[0], response[1], response[2:])

