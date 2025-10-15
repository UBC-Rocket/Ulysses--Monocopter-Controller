# tries to generate example data for the IMU
# tracks the 'true' angular position and generates accel and gyro data
# assumes constant double derivative

# potential velocity graph?
# \frac{\left(\sin x\right)}{\left(1+x^{4}\right)}

import random
import math

class DataGenerator:
    def __init__(self, next_velocity_function, rate):
        self.next_velocity_function = next_velocity_function 
        self.rate = rate # per s
        self.t = 0
        self.thetas = [0, 0, 0]
        self.velocities = [0, 0, 0]

    def next_velocity(self):
        self.velocities = self.next_velocity_function(self.t)
        
    def next_thetas(self):
        for i in range(3):
            self.thetas[i] += (1 / self.rate) * self.velocities[i]
    
    def reported_data(self):
        sensor_data = self.thetas[:]

        for i in range(3):
            parity = random.choice([1,-1])
            noise = parity * random.random() / 10 
            sensor_data[i] += noise

        return sensor_data

            #print(noise)

    def timestep(self):
        self.next_velocity()
        self.next_thetas()
        self.t += (1 / self.rate)

        # [real data, noisy data]
        return [self.thetas, self.reported_data()]


def v1(t):
    v = [0, 0, 0]

    for i in range(2):
        v[i] = math.sin(t) / (1 + t ** (4))
    
    return v

d1 = DataGenerator(v1, 200)

while True:
    print(d1.timestep())