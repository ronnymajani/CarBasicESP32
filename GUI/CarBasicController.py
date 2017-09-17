import json
from NetworkManager import CarBasicNetworkManager


class CarBasicProtocol(object):
    """
    This is a static class that defines the interactions between the Host and Robot
    """
    TCP_TAG_START = "{"
    TCP_TAG_END = "}"
    TCP_TAG_SEPARATOR = ","

    TCP_TAG_STATE_X = "x"
    TCP_TAG_STATE_Y = "y"
    TCP_TAG_STATE_ORIENTATION = "o"
    TCP_TAG_STATE_SENSOR_MEASUREMENT = "s"
    TCP_TAG_STATE_SENSOR_ORIENTATION = "z"
    TCP_TAG_STATE_SPEED = "v"
    TCP_TAG_STATE_PWM_LEFT = "l"
    TCP_TAG_STATE_PWM_RIGHT = "r"

    @staticmethod
    def generate_command(tag, value):
        if isinstance(value, int):
            return "\"%s\":%d" % (tag, value)
        elif isinstance(value, float):
            return "\"%s\":%f" % (tag, value)


class CarBasicController:
    def __init__(self, car):
        self.car = car
        self.networkManager = None

    def data_available(self):
        if self.networkManager is None:
            return False
        else:
            return self.networkManager.data_available()

    def update_car(self):
        if self.data_available():
            data = self.networkManager.get_message()
            self.parse_message(data)

    def parse_message(self, msg):
        """Parses a message received from the Robot"""
        data = json.loads(msg)
        self.car.set_state(
            data[CarBasicProtocol.TCP_TAG_STATE_X],
            data[CarBasicProtocol.TCP_TAG_STATE_Y],
            data[CarBasicProtocol.TCP_TAG_STATE_ORIENTATION],
            data[CarBasicProtocol.TCP_TAG_STATE_SENSOR_MEASUREMENT],
            data[CarBasicProtocol.TCP_TAG_STATE_SENSOR_ORIENTATION],
            data[CarBasicProtocol.TCP_TAG_STATE_SPEED],
            data[CarBasicProtocol.TCP_TAG_STATE_PWM_LEFT],
            data[CarBasicProtocol.TCP_TAG_STATE_PWM_RIGHT]
        )

    def connect(self, ip, port):
        self.networkManager = CarBasicNetworkManager(ip, port)
        self.networkManager.start()
        # todo: implement a control to see if successfully connected
        return True

    def disconnect(self):
        if self.networkManager is not None:
            self.networkManager.stop()
            del self.networkManager
            self.networkManager = None
