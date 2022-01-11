
class Event(object):
    def __init__(self, name: str, date: str, time: str, owner: str) -> None:
        self.name:  str = name
        self.date:  str = date
        self.time:  str = time
        self.owner: str = owner

    def __eq__(self, event) ->  bool:
        return  self.name == event.name and\
                self.date == event.date and\
                self.time == event.time and\
                self.owner == event.owner


