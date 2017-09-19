import threading
import Queue
import socket
import logging
import json
import time


class CarBasicNetworkManager(object):
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.running = True
        self.socket = None
        self.receiverTask = _TCPReceiverTask()
        self.senderTask = _TCPSenderTask()

    def send_message(self, message):
        self.senderTask.send_message(message)

    def get_message(self):
        return self.receiverTask.messageQueue.get()

    def data_available(self):
        return not self.receiverTask.messageQueue.empty()

    def get_latency(self):
        """Returns the time difference (in seconds) between two received commands"""
        return self.receiverTask.latency

    def start(self, timeout=10):
        """Attempts to connect to this object attached IP and PORT then initializes the Receiver and Sender Tasks
        :returns True if connection succeeded
        :returns False if connection failed
        """
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(timeout)
        try:
            self.socket.connect((self.ip, self.port))
            self.socket.settimeout(None)
            # Set the sockets for the Receiver and Sender Tasks
            self.receiverTask.set_socket(self.socket)
            self.senderTask.set_socket(self.socket)
            # Start the receiver and sender tasks
            self.receiverTask.start()
            self.senderTask.start()
            return True
        except Exception, e:
            self.socket.close()
            return False

    def stop(self):
        self.receiverTask.stop()
        self.senderTask.stop()
        self.socket.close()
        self.receiverTask.socket.close()
        self.senderTask.socket.close()
        self.socket = None
        del self.receiverTask
        del self.senderTask
        self.receiverTask = _TCPReceiverTask()
        self.senderTask = _TCPSenderTask()


class _TCPReceiverTask(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.setName("TCP Receiver")
        self.daemon = True

        self.socket = None
        self.messageQueue = Queue.Queue()
        self.running = False
        self.latency = 0

    def set_socket(self, sock):
        self.socket = sock

    def get_message(self):
        return self.messageQueue.get()

    def data_available(self):
        return not self.messageQueue.empty()

    def stop(self):
        self.running = False

    def run(self):
        self.running = True
        logger = logging.getLogger(__name__)
        buff = ""
        start_time = time.time()
        while self.running:
            buff += self.socket.recv(1024)
            tag_end_index = buff.find(CarBasicProtocol.TCP_TAG_END)
            if tag_end_index != -1:
                tag_start_index = buff.find(CarBasicProtocol.TCP_TAG_START)
                if tag_start_index != -1:
                    # calculate latency
                    self.latency = time.time() - start_time
                    start_time = time.time()
                    # append buffer to queue
                    data = buff[tag_start_index:tag_end_index+1]
                    logger.debug("Received the following data:\n%s", data)
                    self.messageQueue.put(data)
                else:
                    logger.error("Junk message received:\n%s\nDiscarding...", buff[:tag_end_index+1])
                # Clear the part of the string we just processed
                if len(buff) > tag_end_index + 1:
                    buff = buff[tag_end_index + 1:]
                else:
                    buff = ""


class _TCPSenderTask(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.setName("TCP Sender")
        self.daemon = True

        self.socket = None
        self.outbox = Queue.Queue()
        self.running = False


    def set_socket(self, sock):
        self.socket = sock

    def send_message(self, msg):
        self.outbox.put(msg)

    def stop(self):
        self.running = False

    def run(self):
        self.running = True
        while self.running:
            out_msg = self.outbox.get()
            self.socket.send(out_msg)


class CarBasicProtocol(object):
    """
    This is a static class that defines the interactions between the Host and Robot
    """
    TCP_TAG_START = "{"
    TCP_TAG_END = "}"
    TCP_TAG_SEPARATOR = ","
    TCP_TAG_INVALID_COMMAND = "-"

    TCP_TAG_STATE_X = "x"
    TCP_TAG_STATE_Y = "y"
    TCP_TAG_STATE_ORIENTATION = "o"
    TCP_TAG_STATE_SENSOR_MEASUREMENT = "s"
    TCP_TAG_STATE_SENSOR_ORIENTATION = "z"
    TCP_TAG_STATE_SPEED = "v"
    TCP_TAG_STATE_PWM_LEFT = "l"
    TCP_TAG_STATE_PWM_RIGHT = "r"

    TCP_TAG_COMMAND_PWM_LEFT = "l"
    TCP_TAG_COMMAND_PWM_RIGHT = "r"
    TCP_TAG_COMMAND_MOTOR_RIGHT_ENABLE = "m"  # used to enable/disable right motor
    TCP_TAG_COMMAND_MOTOR_LEFT_ENABLE = "n"  # used to enable/disable left motor
    TCP_TAG_COMMAND_DIRECTION_RIGHT_FORWARD = "e"  # used to set the direction of the right motor
    TCP_TAG_COMMAND_DIRECTION_LEFT_FORWARD = "w"  # used to set the direction of the left motor
    TCP_TAG_COMMAND_SENSOR_ORIENTATION = "s"  # used to set the sensor orientation

    TCP_COMMAND_VALUE_DIRECTION_FORWARD = 1
    TCP_COMMAND_VALUE_DIRECTION_REVERSE = 0
    TCP_COMMAND_VALUE_ENABLE = 1
    TCP_COMMAND_VALUE_DISABLE = 0

    @staticmethod
    def generate_command_string(command):
        return json.dumps(command)

    @staticmethod
    def command_set_pwm(pwm_right, pwm_left):
        return {
            CarBasicProtocol.TCP_TAG_COMMAND_PWM_RIGHT: pwm_right,
            CarBasicProtocol.TCP_TAG_COMMAND_PWM_LEFT: pwm_left
        }

    @staticmethod
    def command_enable_motors(enable_right, enable_left):
        """Command to enable/disable the motors
        :param enable_right Enables the right motors if True; Disables them if False
        :param enable_left Enables the left motors if True; Disables them if False
        """
        right = CarBasicProtocol.TCP_COMMAND_VALUE_ENABLE if enable_right \
            else CarBasicProtocol.TCP_COMMAND_VALUE_DISABLE
        left = CarBasicProtocol.TCP_COMMAND_VALUE_ENABLE if enable_left \
            else CarBasicProtocol.TCP_COMMAND_VALUE_DISABLE

        return {
            CarBasicProtocol.TCP_TAG_COMMAND_MOTOR_RIGHT_ENABLE: right,
            CarBasicProtocol.TCP_TAG_COMMAND_MOTOR_LEFT_ENABLE: left
        }

    @staticmethod
    def command_set_motor_directions(motor_right_forward, motor_left_forward):
        """Command to set the direction of the motors to FORWARD/REVERSE
        :param motor_right_forward Sets the direction of the right motors to FORWARD if True; Sets them to REVERSE if False
        :param motor_left_forward Sets the direction of the left motors to FORWARD if True; Sets them to REVERSE if False
        """
        forward_right = CarBasicProtocol.TCP_COMMAND_VALUE_DIRECTION_FORWARD if motor_right_forward \
            else CarBasicProtocol.TCP_COMMAND_VALUE_DIRECTION_REVERSE
        forward_left = CarBasicProtocol.TCP_COMMAND_VALUE_DIRECTION_FORWARD if motor_left_forward \
            else CarBasicProtocol.TCP_COMMAND_VALUE_DIRECTION_REVERSE

        return {
            CarBasicProtocol.TCP_TAG_COMMAND_DIRECTION_RIGHT_FORWARD: forward_right,
            CarBasicProtocol.TCP_TAG_COMMAND_DIRECTION_LEFT_FORWARD: forward_left
        }

    @staticmethod
    def command_set_sensor_orientation(orientation):
        """Command to set the orientation of the sensor"""
        return {
            CarBasicProtocol.TCP_TAG_COMMAND_SENSOR_ORIENTATION: orientation
        }
