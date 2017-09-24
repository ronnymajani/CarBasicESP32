import json


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
    TCP_TAG_COMMAND_DIRECTION_RIGHT_FORWARD = "e"  # used to set the direction of the right motor
    TCP_TAG_COMMAND_DIRECTION_LEFT_FORWARD = "w"  # used to set the direction of the left motor
    TCP_TAG_COMMAND_SENSOR_ORIENTATION = "s"  # used to set the sensor orientation

    TCP_COMMAND_VALUE_DIRECTION_NONE = 2
    TCP_COMMAND_VALUE_DIRECTION_FORWARD = 1
    TCP_COMMAND_VALUE_DIRECTION_REVERSE = 0

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

    @staticmethod
    def command_disable_motors(disable_right=True, disable_left=True):
        command = {}
        if disable_right:
            command[CarBasicProtocol.TCP_TAG_COMMAND_DIRECTION_RIGHT_FORWARD] \
                = CarBasicProtocol.TCP_COMMAND_VALUE_DIRECTION_NONE
        if disable_left:
            command[CarBasicProtocol.TCP_TAG_COMMAND_DIRECTION_LEFT_FORWARD] \
                = CarBasicProtocol.TCP_COMMAND_VALUE_DIRECTION_NONE
        return command
