import threading


class CarBasicState:
    """
    This class is a State representation of the CarBasic car robot
    """
    def __init__(self):
        self.__x = 0.0
        self.__y = 0.0
        self.__orientation = 0.0
        self.__sensorReading = 0.0
        self.__sensorOrientation = 0.0
        self.__speed = 0.0
        self.__pwmLeft = 0
        self.__pwmRight = 0
        self.__updateLock = threading.Lock()

    # Getters (No class should be able to directly access the objects properties)
    def x(self):
        return self.__x

    def y(self):
        return self.__y

    def orientation(self):
        return self.__orientation

    def sensor_reading(self):
        return self.__sensorReading

    def sensor_orientation(self):
        return self.__sensorOrientation

    def speed(self):
        return self.__speed

    def pwm_left(self):
        return self.__pwmLeft

    def pwm_right(self):
        return self.__pwmRight

    # Mutex
    def lock(self):
        self.__updateLock.acquire()

    def unlock(self):
        self.__updateLock.release()

    # Setters (forcing the use of setters ensures thread safety)
    def set_state(self, x, y, orientation, sensor_reading, sensor_orientation, speed, pwm_left, pwm_right):
        self.lock()
        self.__x = x
        self.__y = y
        self.__orientation = orientation
        self.__sensorReading = sensor_reading
        self.__sensorOrientation = sensor_orientation
        self.__speed = speed
        self.__pwmLeft = pwm_left
        self.__pwmRight = pwm_right
        self.unlock()
