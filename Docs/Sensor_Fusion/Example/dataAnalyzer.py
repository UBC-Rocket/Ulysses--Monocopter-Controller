import ctypes
import numpy as np
import csv
import time as timer
import math

import matplotlib.pyplot as plt


lib = ctypes.CDLL('./fusion_test.so')

SAMPLE_RATE = 100
N = 10 * SAMPLE_RATE

def test_with_data(f):
    Double3 = ctypes.c_double * 3
    Double3Array = Double3 * N

    lib.ten_sec_test.argtypes = [
        ctypes.POINTER(ctypes.c_double),  # time array
        ctypes.POINTER(Double3),          # accel[N][3]
        ctypes.POINTER(Double3),          # gyro[N][3]
        ctypes.POINTER(Double3),          # output[N][3]
    ]

    lib.ten_sec_test.restype = None       # technically returns nothing

    time = np.linspace(0, 10, N, dtype=np.float64)
    accel = np.zeros((N, 3), dtype=np.float64)
    gyro = np.zeros((N, 3), dtype=np.float64)
    output = np.zeros((N, 3), dtype=np.float64)

    real = np.zeros((N, 3), dtype=np.float64)

    with open(f, newline='') as csvfile:
        csv_reader = csv.reader(csvfile)    
        i = -1

        for row in csv_reader:
            if (i==-1):
                i=0
                continue

            time[i] = row[0]
            gyro[i,0] = row[1]
            gyro[i,1] = row[2]
            gyro[i,2] = row[3]
            accel[i,0] = row[4]
            accel[i,1] = row[5]
            accel[i,2] = row[6]

            real[i,0] = row[7]
            real[i,1] = row[8]
            real[i,2] = row[9]

            i+=1

            if (i >= N):
                break


    time_ptr = (ctypes.c_double * N)(*time)
    accel_ptr = accel.ctypes.data_as(ctypes.POINTER(Double3))
    gyro_ptr  = gyro.ctypes.data_as(ctypes.POINTER(Double3))
    out_ptr   = output.ctypes.data_as(ctypes.POINTER(Double3))

    before = timer.time()
    # running actual c function
    lib.ten_sec_test(time_ptr, accel_ptr, gyro_ptr, out_ptr)
    after = timer.time()

    delta = after - before
    print(delta)
    print(f"{(delta / N):.10f}")
    print(f"{0.01 / (delta / N)}")

    rollx = []
    rolly = []
    rollz = []
    t = np.linspace(0, 10, N, dtype=np.float64)

    for i in range(N):
        rollx.append(abs(output[i][0] - real[i][0]))
        rolly.append(abs(output[i][1] - real[i][1]))
        rollz.append(abs(output[i][2] - real[i][2]))

    fig, axs = plt.subplots(2, 2, figsize=(10, 8)) # 2 rows, 2 columns

    axs[0,0].plot(t, rollx)
    axs[0,0].set_title("Roll X Delta")

    axs[0,1].plot(t, rolly)
    axs[0,1].set_title("Pitch (Y) Delta")

    axs[1,0].plot(t, rollz)
    axs[1,0].set_title("Yaw (Z) Delta")

    axs[1,1].plot(t, rollx, color="red", label="x")
    axs[1,1].plot(t, rolly, color="orange", label="y")
    axs[1,1].plot(t, rollz, color="purple", label="z")
    axs[1,1].set_title("All imposed")

    plt.legend()

    plt.tight_layout()
    plt.show()

test_with_data("Tests/test2.csv")



