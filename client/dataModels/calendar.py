from typing import List

class Calendar(object):
    def __init__(self, name, owner, userList = []) -> None:
        self.name:           str = name
        self.owner:          str = owner
        self.userList: List[str] = userList
