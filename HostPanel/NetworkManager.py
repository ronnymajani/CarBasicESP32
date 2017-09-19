import threading
import Queue
import socket
import logging
import time

from CarBasicProtocol import CarBasicProtocol


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
        # logger.setLevel(logging.INFO)
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


