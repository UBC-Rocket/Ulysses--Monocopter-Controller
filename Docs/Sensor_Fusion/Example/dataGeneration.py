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
            parity = random.choice([1,-1])
            noise = parity * random.random() / 10 
            sensor_data[i] += noise

        return sensor_data

    def get_accel(self):
        # MICHAEL JRDAN!!!!

        x = self.thetas[0]
        y = self.thetas[1]

        a1 = math.cos(x) * math.cos(y)
        a2 = math.sin(x) * math.cos(y)
        a3 = math.sin(y)

        # normalize vector
        # b^2 * |a| = g
        # b = sqrt (g / |a|)
        g = 9.80665
        coefficient = g / (math.sqrt(a1*a1 + a2*a2 + a3*a3))

        accel_values = [coefficient * a1, coefficient * a2, coefficient * a3]

        return accel_values

    def timestep(self):
        self.next_velocity()
        self.next_thetas()   
        self.t += (1 / self.rate)
        
        data = [self.reported_velocity(), self.get_accel(), self.thetas]

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