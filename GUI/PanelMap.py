from PyQt4 import QtGui
import pygame
import sys
import logging

logging.basicConfig(level=logging.DEBUG)

pygame.init()


class Map(QtGui.QWidget):
    def __init__(self, parent):
        QtGui.QWidget.__init__(self, parent)

        # Initiate Logger
        self.logger = logging.getLogger(__name__)

        # Set the width and height to match parent
        self.setFixedWidth(parent.width())
        self.setFixedHeight(parent.height())
        self.logger.debug("Canvas Width = %d, Height = %d", self.width(), self.height())

        # Attach pygame buffer to the QtImage widget (canvas)
        self.surface = pygame.Surface((self.width(), self.height()))
        data = self.surface.get_buffer().raw
        self.canvas = QtGui.QImage(data, self.width(), self.height(), QtGui.QImage.Format_RGB32)

        # Set BG Color
        self.background_color = (255, 255, 255, 255)
        self.surface.fill(self.background_color)
        self.draw()

    def paintEvent(self, QPaintEvent):
        self.logger.debug("painting")
        qp = QtGui.QPainter()
        qp.begin(self)
        qp.drawImage(0, 0, self.canvas)
        qp.end()

    def draw(self):
        data = self.surface.get_buffer().raw
        self.canvas = QtGui.QImage(data, self.width(), self.height(), QtGui.QImage.Format_RGB32)
        self.repaint()


