from typing import Union

class UserInfo(object):
    def __init__(self, 
                 username: Union[None, str] = None, 
                 password: Union[None, str] = None) -> None:
        super().__init__()
        self.username: str = username
        self.password: str = password

    def setUserData(self, username: str, password: str) -> None:
        self.username = username
        self.password = password

    def deleteUserData(self) -> None:
        self.username = None
        self.password = None