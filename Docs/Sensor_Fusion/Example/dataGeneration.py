# tries to generate example data for the IMU
# tracks the 'true' angular position and generates accel and gyro data
# assumes constant double derivative

# potential velocity graph?
# \frac{\left(\sin x\right)}{\left(1+x^{4}\right)}

import random
import math

class DataGenerator:
    def __init__(self, next_velocity_function, rate, file):
        self.next_velocity_function = next_velocity_function 
        self.rate = rate # per s
        self.file = file

        with open(self.file, "w") as file:
            # CSV format is {time | gyrox | gyroy | gyroz | accelx | accely | accelz | realx | realy | realz}   
            file.write("time,gyrox,gyroy,gyroz,accelx,accely,accelz,realx,realy,realz\n")

        self.t = 0
        self.thetas = [0, 0, 0]
        self.velocities = [0, 0, 0]

    def next_velocity(self):
        self.velocities = self.next_velocity_function(self.t)
        
    def next_thetas(self):
        for i in range(3):
            self.thetas[i] += (1 / self.rate) * self.velocities[i]
    
    def reported_velocity(self):
        sensor_data = self.velocities[:]

        for i in range(3):
            # rad to deg
            sensor_data[i] *= 180 / math.pi

            parity = random.choice([1,-1])
            noise = parity * random.random() / 10 
            sensor_data[i] += noise

        return sensor_data

    def get_accel(self):
        # # MICHAEL JRDAN!!!!

        # x = self.thetas[0]
        # y = self.thetas[1]

        # a1 = math.cos(x) * math.cos(y)
        # a2 = math.sin(x) * math.cos(y)
        # a3 = math.sin(y)

        # accel_values = [a1, a2, a3]

        # return accel_values
        # R_world_to_body = Rx * Ry * Rz
        roll = self.thetas[0]
        pitch = self.thetas[1]
        yaw = self.thetas[2]

        cR, sR = math.cos(roll), math.sin(roll)
        cP, sP = math.cos(pitch), math.sin(pitch)
        cY, sY = math.cos(yaw), math.sin(yaw)
        R11 = cP * cY
        R12 = cP * sY
        R13 = -sP
        R21 = sR * sP * cY - cR * sY
        R22 = sR * sP * sY - cR * (-cY)  # = sR*sP*sY + cR*cY
        R23 = sR * cP
        R31 = cR * sP * cY + sR * sY
        R32 = cR * sP * sY - sR * (-cY)  # = cR*sP*sY + sR*cY
        R33 = cR * cP

        # g_world = [0, 0, -1]
        ax = R13
        ay = R23
        az = R33

        return [ax, ay, az]

    def timestep(self):
        self.next_velocity()
        self.next_thetas()   
        self.t += (1 / self.rate)

        degrees_thetas = self.thetas[:]
        for i in range(3):
            degrees_thetas[i] = degrees_thetas[i] * 180 / math.pi 
        
        data = [self.reported_velocity(), self.get_accel(), degrees_thetas]

        self.write_data(data, self.t)

        # [real theta, noisy velocity]
        return data

    def write_data(self, data, t):
        with open(self.file, "a") as file:
            formatted = f"{t:.10f},{data[0][0]:.10f},{data[0][1]:.10f},{data[0][2]:.10f},{data[1][0]:.10f},{data[1][1]:.10f},{data[1][2]:.10f},{data[2][0]:.10f},{data[2][1]:.10f},{data[2][2]:.10f}\n"
            file.write(formatted)


def v1(t):
    v = [0, 0, 0]

    for i in range(2):
        v[i] = math.sin(t) / (1 + t ** (4))
    
    return v

d1 = DataGenerator(v1, 200, "Tests/Test 1/data.csv")

while True:
    print(d1.timestep())
    print(math.sin(360))