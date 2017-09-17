from CarBasicState import CarBasicState
from CarBasicGraphic import CarBasicGraphic
from CarBasicController import CarBasicController


class CarBasic:
    """
    This class contains all the modules needed for full functionality of the CarBasic interface
    """
    def __init__(self):
        self.state = CarBasicState()
        self.graphicDriver = CarBasicGraphic(self.state)
        self.controller = CarBasicController(self.state)
