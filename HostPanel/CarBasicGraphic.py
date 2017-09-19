import pygame
import math
from CarBasicState import CarBasicState


class CarBasicGraphic:
    """
    A Wrapper for CarBasic that handles all the GUI related functionality
    All the variables in this class are stored in pixels
    Conversion takes place automatically using the given scale factor
    """
    def __init__(self, car, scale=1.0, width=10, length=20, color=(150, 150, 150), line_color=(0, 0, 0)):
        """
        :param car: The CarBasic object to attach this class to
        :param scale: The pixel to meter ratio (how many pixels represent a meter)
        :param width: The width of the car (in meters)
        :param length: The length of the car (in meters)
        :param color: The (R,G,B) color to draw the car with in the GUI
        :param line_color: The (R,G,B) color to draw the sensor reading line with
        """
        self.scale = scale  # defines the pixel:meter ratio (how many pixels represent one meter)
        self.width = width * scale
        self.length = length * scale
        self.color = color
        self.line_color = line_color
        self.car = car
        # History of all the previous measured points (to draw a map from these readings)
        self.history = []

    def draw(self, surface):
        surface_center_x = surface.get_width() / 2.0
        surface_center_y = surface.get_height() / 2.0
        points = self.get_coords(surface_center_x, surface_center_y)
        # self.history.append(points[5])
        pygame.draw.polygon(surface, self.color, points[0:4])  # draw the car's polygon (rectangle)
        pygame.draw.line(surface, self.line_color, points[4], points[5])  # draw the sensor reading's line

        for point in self.history:
            pygame.draw.circle(surface, self.line_color, (int(point[0]), int(point[1])), 3 * int(self.scale))

    def get_coords(self, canvas_center_x, canvas_center_y):
        """Returns a list of 6 coordinates,
         the first four form a rectangle rotated by this objects orientation
         the last two draw a straight line showing the distance measured by the sensor"""
        # half length of the rectangle that represents this car
        half_length = self.length / 2.0
        half_width = self.width / 2.0

        x = self.car.x() * self.scale
        y = self.car.y() * self.scale
        orientation = self.car.orientation()
        sensor_measured_distance = self.car.sensor_reading() * self.scale
        sensor_orientation = self.car.sensor_orientation()

        # create the rectangles coordinates
        tr = (x + half_length, y + half_width)
        br = (x + half_length, y - half_width)
        bl = (x - half_length, y - half_width)
        tl = (x - half_length, y + half_width)
        # create the distance line coordinates
        line_start = (x, y)
        line_end = (line_start[0] + sensor_measured_distance, line_start[1])

        # rotate the coordinates
        def rotate(point):
            return CarBasicGraphic.rotate_around_point(point, (x, y), orientation)

        tr = rotate(tr)
        br = rotate(br)
        bl = rotate(bl)
        tl = rotate(tl)

        def rotate(point):
            return CarBasicGraphic.rotate_around_point(point, (x, y), orientation + sensor_orientation)
        # line_start = rotate(line_start)  # redundant since the line start is rotate the origin
        line_end = rotate(line_end)

        # convert from object space to world space (pygame coordinates are relative to the top left corner (0, 0))
        # pygame Y positive is towards the bottom (opposite of object space)
        def obj2world(point):
            return canvas_center_x + point[0], canvas_center_y - point[1]

        tr = obj2world(tr)
        br = obj2world(br)
        bl = obj2world(bl)
        tl = obj2world(tl)
        line_start = obj2world(line_start)
        line_end = obj2world(line_end)

        return tl, tr, br, bl, line_start, line_end

    @staticmethod
    def rotate_around_point(point, origin, angle):
        x = point[0]
        y = point[1]
        cx = origin[0]
        cy = origin[1]
        angle_cos = math.cos(math.radians(angle))
        angle_sin = math.sin(math.radians(angle))

        # translate point to origin
        x -= cx
        y -= cy

        # rotate point
        nx = x * angle_cos - y * angle_sin
        ny = x * angle_sin + y * angle_cos

        # return point to original location
        x = nx + cx
        y = ny + cy

        return x, y
