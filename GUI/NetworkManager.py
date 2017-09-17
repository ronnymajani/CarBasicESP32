import threading
import Queue
import socket


class CarBasicNetworkManager(threading.Thread):
    def __init__(self, ip, port):
        threading.Thread.__init__(self)
        self.setName("CarBasic Network Manager")
        self.outbox = Queue.Queue()
        self.messageQueue = Queue.Queue()
        self.ip = ip
        self.port = port
        self.running = True

    def run(self):
        # initialize connection
        self.tcp_connection_initialization()
        # start TCP service
        self.tcp_service()

    def get_message(self):
        return self.messageQueue.get()

    def data_available(self):
        return not self.messageQueue.empty()

    def send_message(self, message):
        self.outbox.put(message)

    def tcp_connection_initialization(self):
        # todo
        pass

    def tcp_service(self):
        # todo
        while self.running:
            pass

    def stop(self):
        self.running = False
