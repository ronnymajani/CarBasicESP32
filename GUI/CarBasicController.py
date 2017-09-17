import json
from NetworkManager import CarBasicNetworkManager, CarBasicProtocol


class CarBasicController:
    def __init__(self, car):
        self.car = car
        self.networkManager = None

    def data_available(self):
        """Check if there is any previously received data that needs processing"""
        if self.networkManager is None:
            return False
        else:
            return self.networkManager.data_available()

    def update_car(self):
        """Processes the next data set received by the Network Manager and updates the CarBasic state"""
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

    def connect(self, ip, port, timeout=10):
        """Attempts to create a Network Manager and connect to a CarBasic robot at the given IP address
        :returns False if Network Manager failed to connect to device
        :returns True if Network Manager successfully connected to the device
        """
        self.networkManager = CarBasicNetworkManager(ip, port)
        if self.networkManager.tcp_connection_initialization(timeout=timeout):
            self.networkManager.start()
            return True
        else:
            self.networkManager = None
            return False

    def disconnect(self):
        """Disconnects from any previously connected device and ends the currently attached Network Manager task"""
        if self.is_connected():
            self.networkManager.stop()
            del self.networkManager
            self.networkManager = None

    def is_connected(self):
        return self.networkManager is not None
