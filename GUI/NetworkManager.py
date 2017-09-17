import threading
import Queue
import socket
import logging


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

        self.socket = None
        self.messageQueue = Queue.Queue()
        self.running = True

    def set_socket(self, sock):
        self.socket = sock

    def get_message(self):
        return self.messageQueue.get()

    def data_available(self):
        return not self.messageQueue.empty()

    def stop(self):
        self.running = False

    def run(self):
        logger = logging.getLogger(__name__)
        buff = ""
        while self.running:
            buff += self.socket.recv(1024)
            tag_end_index = buff.find(CarBasicProtocol.TCP_TAG_END)
            if tag_end_index != -1:
                tag_start_index = buff.find(CarBasicProtocol.TCP_TAG_START)
                if tag_start_index != -1:
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

        self.socket = None
        self.outbox = Queue.Queue()
        self.running = True

    def set_socket(self, sock):
        self.socket = sock

    def send_message(self, msg):
        self.outbox.put(msg)

    def stop(self):
        self.running = False

    def run(self):
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