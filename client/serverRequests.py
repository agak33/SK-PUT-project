from typing import Union, List
from setup import *
import socket
from calendar import Calendar

class ServerRequests(object):
    def __init__(self) -> None:
        super().__init__()
        self.socket:           socket   = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connectionStatus: bool     = True
        self.errorMessage:     str      = None;

        try:
            self.socket.connect((APPLICATION_HOST, APPLICATION_PORT))
        except ConnectionRefusedError:
            self.errorMessage     = 'Cannot connect to the server'
            self.connectionStatus = False

    def sendLoginData(self, login: str, passwd: str) -> Union[None, str]:
        if self.connectionStatus:
            self.socket.sendall(
                f'L {login} {passwd}'.encode()
            )
            response = self.socket.recv(1024).decode()
            if response[0] != '1':
                return response[2:]
            return None
        return None
        return 'Cannot connect to the server'

    def sendRegisterData(self, login: str, passwd: str) -> Union[None, str]:
        if self.connectionStatus:
            self.socket.sendall(
                f'R {login} {passwd}'.encode()
            )
            response = self.socket.recv(1024).decode()
            if response[0] != '1':
                return response[2:]
            return None
        return None
        return 'Cannot connect to the server'

    def sendLogoutData(self, login: str) -> Union[None, str]:
        if self.connectionStatus:
            self.socket.sendall(
                f'{login}'.encode()
            )
            response = self.socket.recv(1024).decode()
            if response[0] != '1':
                return response[2:]
            return None
        return None
        return 'Cannot connect to the server'

    def sendNewCalendarData(self, name: str, userList: str):
        userList = userList.replace(' ', '')
        userList = userList.replace(',', ' ')
        if self.connectionStatus:
            self.socket.sendall(
                f'{name} {userList}'.encode()
            )
            response = self.socket.recv(1024).decode()
            if response[0] != '1':
                return response[2:]
            return None
        return None
        return 'Cannot connect to the server'


    def getCalendars(self) -> List[Calendar]:
        #amount = int(self.socket.recv(10).decode())
        amount = 10
        calendarList = []
        for _ in range(amount):
            #calendarData = self.socket.recv(2048).decode().split(';')
            calendarData = ['calendar 1', 'aaa', 'user1', 'user2']
            calendarList.append(Calendar(calendarData[0], calendarData[1], calendarData[2:]))
        return calendarList

