import threading


class CarBasic:
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
        self.__pwmX = 0
        self.__pwmY = 0
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

    def pwm_x(self):
        return self.__pwmX

    def pwm_y(self):
        return self.__pwmY

    # Mutex
    def lock(self):
        self.__updateLock.acquire()

    def unlock(self):
        self.__updateLock.release()

    # Setters (forcing the use of setters ensures thread safety)
    def set_state(self, x, y, orientation, sensor_reading, sensor_orientation, speed, pwm_x, pwm_y):
        self.lock()
        self.__x = x
        self.__y = y
        self.__orientation = orientation
        self.__sensorReading = sensor_reading
        self.__sensorOrientation = sensor_orientation
        self.__speed = speed
        self.__pwmX = pwm_x
        self.__pwmY = pwm_y
        self.unlock()
