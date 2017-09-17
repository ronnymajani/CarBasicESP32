import sys
from PyQt4 import QtCore, QtGui, uic
import logging
from PanelMap import Map


logging.basicConfig(level=logging.DEBUG)


qtCreatorFile = "host_panel.ui"

UI_MainWindow, QtBaseClass = uic.loadUiType(qtCreatorFile)


class HostPanelApp(QtGui.QMainWindow, UI_MainWindow):
    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        UI_MainWindow.__init__(self)
        self.setupUi(self)
        self.Map = Map(self.robot_map)

        self.logger = logging.getLogger(__name__)
        self.register_buttons()

    def register_buttons(self):
        """Register the callback functions for the GUI buttons"""
        # Controls Tab
        self.controls_widget.currentChanged.connect(self.controls_widget_tab_changed)
        # Manual Controls
        self.controls_button_motor_start_stop.clicked.connect(self.controls_button_motor_start_stop_clicked)
        self.controls_pwm_R.valueChanged.connect(self.controls_pwm_value_changed)
        self.controls_pwm_L.valueChanged.connect(self.controls_pwm_value_changed)
        # Auto Controls
        self.controls_auto_button_motor_stop.clicked.connect(self.auto_controls_stop_button_clicked)
        self.controls_button_motor_F.clicked.connect(lambda:
                                                     self.auto_controls_button_clicked(self.controls_button_motor_F))
        self.controls_button_motor_FR.clicked.connect(lambda:
                                                      self.auto_controls_button_clicked(self.controls_button_motor_FR))
        self.controls_button_motor_FL.clicked.connect(lambda:
                                                      self.auto_controls_button_clicked(self.controls_button_motor_FL))
        self.controls_button_motor_R.clicked.connect(lambda:
                                                     self.auto_controls_button_clicked(self.controls_button_motor_R))
        self.controls_button_motor_L.clicked.connect(lambda:
                                                     self.auto_controls_button_clicked(self.controls_button_motor_L))
        self.controls_button_motor_B.clicked.connect(lambda:
                                                     self.auto_controls_button_clicked(self.controls_button_motor_B))
        self.controls_button_motor_BR.clicked.connect(lambda:
                                                      self.auto_controls_button_clicked(self.controls_button_motor_BR))
        self.controls_button_motor_BL.clicked.connect(lambda:
                                                      self.auto_controls_button_clicked(self.controls_button_motor_BL))

    # Controls Tab Widget
    def controls_widget_tab_changed(self):
        index = self.controls_widget.currentIndex()
        self.logger.debug("Tab changed: index %d", index)
        # TODO: Stop Motors

    # Manual Controls Tab
    def controls_button_motor_start_stop_clicked(self):
        self.logger.debug("START/STOP button clicked")
        button = self.controls_button_motor_start_stop
        if button.text() == "START":
            self.logger.debug("Starting motors")
            button.setText("STOP")
            # TODO: Start Motor
        elif button.text() == "STOP":
            self.logger.debug("Stopping motors")
            button.setText("START")
            # TODO: Stop Motor

    def controls_pwm_value_changed(self):
        pwm_right = self.controls_pwm_R.value()
        pwm_left = self.controls_pwm_L.value()
        self.set_pwm(pwm_left, pwm_right)
        # TODO: Set PWM Duty Cycle of Motors

    # Auto Controls Tab
    def clear_auto_controls_buttons(self):
        """Unchecks all the buttons in the Auto Controls tab"""
        self.controls_button_motor_F.setChecked(False)
        self.controls_button_motor_FR.setChecked(False)
        self.controls_button_motor_FL.setChecked(False)
        self.controls_button_motor_R.setChecked(False)
        self.controls_button_motor_L.setChecked(False)
        self.controls_button_motor_B.setChecked(False)
        self.controls_button_motor_BR.setChecked(False)
        self.controls_button_motor_BL.setChecked(False)

    def auto_controls_button_clicked(self, button):
        is_checked = button.isChecked()
        self.clear_auto_controls_buttons()
        if is_checked:
            button.setChecked(True)
            direction = button.text()
            # TODO: Drive car in the Buttons direction
        else:
            self.auto_controls_stop_button_clicked()

    def auto_controls_stop_button_clicked(self):
        self.clear_auto_controls_buttons()
        # TODO: Stop Motors

    # Setters
    def set_position(self, x, y):
        self.position_X_value.setText(x)
        self.position_Y_value.setText(y)

    def set_orientation(self, degrees):
        self.orientation_value.setText(degrees)

    def set_speed(self, mps):
        self.speed_value.setText(mps + "m/s")

    def set_sensor_reading(self, distance_in_meters):
        self.sensor_reading_value.setText(distance_in_meters + "m")

    def set_pwm(self, pwm_left, pwm_right):
        self.pwm_L_value.setValue(pwm_left)
        self.pwm_R_value.setValue(pwm_right)


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    window = HostPanelApp()
    window.show()
    sys.exit(app.exec_())
