# tries to generate example data for the IMU
# tracks the 'true' angular position and generates accel and gyro data
# assumes constant double derivative

# potential velocity graph?
# \frac{\left(\sin x\right)}{\left(1+x^{4}\right)}

import random
import math
import matplotlib.pyplot as plt

def gaussian_noise(mean, std):
    u1 = random.random()
    u2 = random.random()
    z = math.sqrt(-2.0 * math.log(u1)) * math.cos(2.0 * math.pi * u2)
    return mean + std * z

def impulse_function(x, mean, std):
    y = (std * math.sqrt(2 * math.pi)) ** (-1) * (math.e) ** (-(1/2)*((x - mean) / std)**2)
    y = min(y, 0.02)
    return y

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

        B_eff = 40.0  # effective bandwidth in Hz (for ODR = 100 Hz, normal mode)
        noise_density_xy = 160e-6 * 9.81  # µg/√Hz → m/s²/√Hz
        noise_density_z  = 190e-6 * 9.81

        # RMS noise for each axis
        sigma_xy = noise_density_xy * math.sqrt(B_eff)
        sigma_z  = noise_density_z  * math.sqrt(B_eff)


        std = 0.1
        impulse_x = 4
        impulse_y = 7

        noisy_data = [
            ax + gaussian_noise(0.0, sigma_xy) + impulse_function(self.t, impulse_x, std), 
            ay + gaussian_noise(0.0, sigma_xy) + impulse_function(self.t, impulse_y, std), 
            az + gaussian_noise(0.0, sigma_z)
        ]

        tick = ((self.t) / (1 / self.rate))

        if (tick > 195 and tick < 205):
            print(noisy_data)

        return noisy_data

    def timestep(self):
        self.next_velocity()
        self.next_thetas()   
        self.t += (1 / self.rate)

        degrees_thetas = self.thetas[:]
        for i in range(3):
            degrees_thetas[i] = degrees_thetas[i] * 180 / math.pi 
        
        data = [self.reported_velocity(), self.get_accel(), degrees_thetas]

        self.write_data(self.t, data)

        # [noisy velocity, noisy accel, real theta]
        return data

    def write_data(self, t, data):
        with open(self.file, "a") as file:
            formatted = f"{t:.10f},{data[0][0]:.10f},{data[0][1]:.10f},{data[0][2]:.10f},{data[1][0]:.10f},{data[1][1]:.10f},{data[1][2]:.10f},{data[2][0]:.10f},{data[2][1]:.10f},{data[2][2]:.10f}\n"
            file.write(formatted)


def v1(t):
    v = [0, 0, 0]

    for i in range(2):
        v[i] = math.sin(t) / (1 + t ** (4))
    
    return v

def v2(t):
    v = [0, 0, 0]

    v[0] = math.sin(t) / max(t, 0.0004) / 20
    v[1] = math.sin(t) / max(t, 0.0004) / 20
   
    
    return v

d1 = DataGenerator(v1, 100, "Tests/test1.csv")
d2 = DataGenerator(v2, 100, "Tests/test2.csv")


for i in range(1000):
    d1.timestep()
    d2.timestep()

# debugging impulse function
fig, ax = plt.subplots()

ax.plot([x/100 for x in range(0,1000)],
        [impulse_function(x/100, 2, 0.01) for x in range(0,1000)])

plt.show()
