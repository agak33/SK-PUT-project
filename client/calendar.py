from typing import List

class Calendar(object):
    def __init__(self, name: str, owner: str, userList: List[str] = []) -> None:
        super().__init__()
        self.name:           str = name
        self.owner:          str = owner
        self.userList: List[str] = userList

    def addUser(self, username: str):
        self.userList.append(username)