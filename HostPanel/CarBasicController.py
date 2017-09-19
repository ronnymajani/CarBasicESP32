import json
from NetworkManager import CarBasicNetworkManager
from CarBasicProtocol import CarBasicProtocol


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

        def get_data(tag):
            if tag in data:
                return data[tag]
            else:
                return None

        self.car.set_state(
            get_data(CarBasicProtocol.TCP_TAG_STATE_X),
            get_data(CarBasicProtocol.TCP_TAG_STATE_Y),
            get_data(CarBasicProtocol.TCP_TAG_STATE_ORIENTATION),
            get_data(CarBasicProtocol.TCP_TAG_STATE_SENSOR_MEASUREMENT),
            get_data(CarBasicProtocol.TCP_TAG_STATE_SENSOR_ORIENTATION),
            get_data(CarBasicProtocol.TCP_TAG_STATE_SPEED),
            get_data(CarBasicProtocol.TCP_TAG_STATE_PWM_LEFT),
            get_data(CarBasicProtocol.TCP_TAG_STATE_PWM_RIGHT)
        )

    def connect(self, ip, port, timeout=10):
        """Attempts to create a Network Manager and connect to a CarBasic robot at the given IP address
        :returns False if Network Manager failed to connect to device
        :returns True if Network Manager successfully connected to the device
        """
        self.networkManager = CarBasicNetworkManager(ip, port)
        if self.networkManager.start(timeout=timeout):
            return True
        else:
            self.networkManager = None
            return False

    def disconnect(self):
        """Disconnects from any previously connected device and ends the currently attached Network Manager task"""
        if self.is_connected():
            self.networkManager.stop()
            self.networkManager = None

    def is_connected(self):
        return self.networkManager is not None

    def get_latency(self):
        """Returns the time difference (in milliseconds) between two received commands as a string!"""
        if self.networkManager is not None:
            return self.networkManager.get_latency() * 1000
        else:
            return 0

    def move(self, pwm_right, pwm_left, dir_right, dir_left):
        """Tells the car to move with the given pmw values and directions"""
        if self.is_connected():
            command = CarBasicProtocol.command_set_pwm(pwm_right, pwm_left)
            command.update(CarBasicProtocol.command_set_motor_directions(dir_right, dir_left))
            command.update(CarBasicProtocol.command_enable_motors(True, True))
            self.networkManager.send_message(CarBasicProtocol.generate_command_string(command))

    def move_car_forward(self, pwm):
        """Tells the car to move in a straight line"""
        self.move(pwm, pwm, True, True)

    def move_car_forward_right(self, pwm):
        """Tells the car to move forward while turning to the right"""
        self.move(pwm/2.0, pwm, True, True)

    def move_car_forward_left(self, pwm):
        """Tells the car to move forward while turning to the left"""
        self.move(pwm, pwm/2.0, True, True)

    def rotate_car_clockwise(self, pwm):
        """Tells the car to rotate in place clockwise (right motor reverse, left motor forward)"""
        self.move(pwm, pwm, False, True)

    def rotate_car_counter_clockwise(self, pwm):
        """Tells the car to rotate in place counter clockwise (right motor forward, left motor reverse)"""
        self.move(pwm, pwm, True, False)

    def move_car_reverse(self, pwm):
        """Tells the car to move in a straight line"""
        self.move(pwm, pwm, False, False)

    def move_car_reverse_right(self, pwm):
        """Tells the car to move in reverse while turning to the right (right motor moves slower then left)"""
        self.move(pwm/2.0, pwm, False, False)

    def move_car_reverse_left(self, pwm):
        """Tells the car to move in reverse while turning to the left (right motor moves faster then left)"""
        self.move(pwm, pwm/2.0, False, False)

    def stop_car(self):
        if self.is_connected():
            command = CarBasicProtocol.command_enable_motors(False, False)
            self.networkManager.send_message(CarBasicProtocol.generate_command_string(command))

    def start_car(self):
        if self.is_connected():
            command = CarBasicProtocol.command_enable_motors(True, True)
            self.networkManager.send_message(CarBasicProtocol.generate_command_string(command))

    def set_direction_forward(self):
        if self.is_connected():
            self.networkManager.send_message(CarBasicProtocol.generate_command_string(
                CarBasicProtocol.command_set_motor_directions(True, True)
            ))

    def set_car_pwm(self, pwm_right, pwm_left):
        if self.is_connected():
            command = CarBasicProtocol.command_set_pwm(pwm_right, pwm_left)
            self.networkManager.send_message(CarBasicProtocol.generate_command_string(command))
