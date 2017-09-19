import sys
from PyQt4 import QtCore, QtGui, uic
import logging
from PanelMap import PanelMap
from CarBasic import CarBasic


logging.basicConfig(level=logging.DEBUG)

REFRESH_INTERVAL = 20  # refresh interval in ms
LATENCY_REFRESH_INTERVAL = 250  # refresh interval for Latency indicator
qtCreatorFile = "host_panel.ui"

UI_MainWindow, QtBaseClass = uic.loadUiType(qtCreatorFile)


class HostPanelApp(QtGui.QMainWindow, UI_MainWindow):
    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        UI_MainWindow.__init__(self)
        self.setupUi(self)

        self.car = CarBasic()
        self.panelMap = PanelMap(self.panel_canvas, self.car)
        self.logger = logging.getLogger(__name__)

        # Panel Updater timer
        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(REFRESH_INTERVAL)
        self.connect(self.timer, QtCore.SIGNAL("timeout()"), self.update_panel)
        self.timer.start()
        # Latency Updater timer
        self.latencyTimer = QtCore.QTimer(self)
        self.latencyTimer.setInterval(LATENCY_REFRESH_INTERVAL)
        self.connect(self.latencyTimer, QtCore.SIGNAL("timeout()"), self.update_latency)
        self.latencyTimer.start()

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
        self.controls_auto_pwm.valueChanged.connect(self.controls_auto_pwm_value_changed)
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
        # Controls Settings
        self.controls_settings_button_connect.clicked.connect(self.controls_settings_connect)
        self.controls_settings_button_disconnect.clicked.connect(self.controls_settings_disconnect)

    # Controls Tab Widget
    def controls_widget_tab_changed(self):
        index = self.controls_widget.currentIndex()
        self.logger.debug("Tab changed: index %d", index)

    # Manual Controls Tab
    def controls_button_motor_start_stop_clicked(self):
        self.logger.debug("START/STOP button clicked")
        button = self.controls_button_motor_start_stop
        if button.text() == "START":
            self.logger.debug("Starting motors")
            button.setText("STOP")
            self.controls_pwm_value_changed()
            self.car.controller.set_direction_forward()
            self.car.controller.start_car()
        elif button.text() == "STOP":
            self.logger.debug("Stopping motors")
            button.setText("START")
            self.car.controller.stop_car()

    def controls_pwm_value_changed(self):
        pwm_right = self.controls_pwm_R.value()
        pwm_left = self.controls_pwm_L.value()
        self.car.controller.set_car_pwm(pwm_right, pwm_left)

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

    def controls_auto_pwm_value_changed(self):
        pwm = self.controls_auto_pwm.value()
        self.car.controller.set_car_pwm(pwm, pwm)

    def auto_controls_button_clicked(self, button):
        is_checked = button.isChecked()
        self.clear_auto_controls_buttons()
        if is_checked:
            button.setChecked(True)
            direction = button.text()
            pwm = self.controls_auto_pwm.value()
            if direction == "F":
                self.car.controller.move_car_forward(pwm)
            if direction == "FR":
                self.car.controller.move_car_forward_right(pwm)
            if direction == "FL":
                self.car.controller.move_car_forward_left(pwm)
            if direction == "B":
                self.car.controller.move_car_reverse(pwm)
            if direction == "BL":
                self.car.controller.move_car_reverse_left(pwm)
            if direction == "BR":
                self.car.controller.move_car_reverse_right(pwm)
            if direction == "R":
                self.car.controller.rotate_car_clockwise(pwm)
            if direction == "L":
                self.car.controller.rotate_car_counter_clockwise(pwm)
        else:
            self.auto_controls_stop_button_clicked()

    def auto_controls_stop_button_clicked(self):
        self.clear_auto_controls_buttons()
        self.car.controller.stop_car()

    # Controls Settings
    def controls_settings_connect(self):
        ip = self.controls_settings_ip_value.text()
        port = self.controls_settings_port_value.value()
        timeout = self.controls_settings_timeout_value.value()

        self.controls_settings_ip_value.setEnabled(False)
        self.controls_settings_port_value.setEnabled(False)
        self.controls_settings_timeout_value.setEnabled(False)
        self.controls_settings_button_connect.setEnabled(False)
        self.connection_status_label.setText("Connecting...")

        if self.car.controller.connect(ip, port, timeout=timeout):
            self.connection_status_label.setEnabled(True)
            self.controls_settings_button_disconnect.setEnabled(True)
            self.controls_widget.setCurrentIndex(0)
            self.connection_status_label.setText("Connected!")
        else:
            self.controls_settings_ip_value.setEnabled(True)
            self.controls_settings_port_value.setEnabled(True)
            self.controls_settings_timeout_value.setEnabled(True)
            self.controls_settings_button_connect.setEnabled(True)
            self.connection_status_label.setText("Failed to connect to device!")

    def controls_settings_disconnect(self):
        if self.car.controller.is_connected():
            self.car.controller.disconnect()
            self.controls_settings_ip_value.setEnabled(True)
            self.controls_settings_port_value.setEnabled(True)
            self.controls_settings_timeout_value.setEnabled(True)
            self.controls_settings_button_disconnect.setEnabled(False)
            self.controls_settings_button_connect.setEnabled(True)
            self.connection_status_label.setEnabled(False)
            self.connection_status_label.setText("Disconnected from Device")


    # Setters
    def set_position(self, x, y):
        self.position_X_value.setText(str(x))
        self.position_Y_value.setText(str(y))

    def set_orientation(self, degrees):
        self.orientation_value.setText(str(degrees))

    def set_speed(self, mps):
        self.speed_value.setText(str(mps) + "m/s")

    def set_sensor_reading(self, distance_in_meters):
        self.sensor_reading_value.setText(str(distance_in_meters) + "m")

    def set_pwm(self, pwm_left, pwm_right):
        # Set indicators
        self.pwm_L_value.setValue(pwm_left)
        self.pwm_R_value.setValue(pwm_right)
        # # Move sliders
        # self.controls_pwm_L.setValue(pwm_left)
        # self.controls_pwm_R.setValue(pwm_right)

    def update_panel(self):
        if self.car.controller.data_available():
            # get new data
            self.car.controller.update_car()
            # update map
            self.panelMap.draw()
            # update widgets
            state = self.car.state
            self.set_position(state.x(), state.y())
            self.set_pwm(state.pwm_left(), state.pwm_right())
            self.set_orientation(state.orientation())
            self.set_sensor_reading(state.sensor_reading())
            self.set_speed(state.speed())
            self.repaint()

    def update_latency(self):
        self.network_latency_value.setText("%07.03f" % self.car.controller.get_latency())

    def closeEvent(self, event):
        self.car.close()


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    window = HostPanelApp()
    window.show()
    sys.exit(app.exec_())
