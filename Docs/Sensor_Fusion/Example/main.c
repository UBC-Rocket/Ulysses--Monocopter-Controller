#include "../Fusion/Fusion/Fusion.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// gcc -o fusion_test main.c ../Fusion/Fusion/*.c -lm

// compiled for python with
// gcc -shared -o fusion_test.so -fPIC main.c ../Fusion/Fusion/*.c

#define SAMPLE_RATE (100) // per s

static inline double timespec_to_seconds(const struct timespec *ts) {
    return ts->tv_sec + ts->tv_nsec * 1e-9;
}

int main() {

    // Define calibration (replace with actual calibration data if available)
    const FusionMatrix gyroscopeMisalignment = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const FusionVector gyroscopeSensitivity = {1.0f, 1.0f, 1.0f};
    const FusionVector gyroscopeOffset = {0.0f, 0.0f, 0.0f};
    const FusionMatrix accelerometerMisalignment = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const FusionVector accelerometerSensitivity = {1.0f, 1.0f, 1.0f};
    const FusionVector accelerometerOffset = {0.0f, 0.0f, 0.0f};
    const FusionMatrix softIronMatrix = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const FusionVector hardIronOffset = {0.0f, 0.0f, 0.0f};

    // Initialise algorithms
    FusionOffset offset;
    FusionAhrs ahrs;

    FusionOffsetInitialise(&offset, SAMPLE_RATE);
    FusionAhrsInitialise(&ahrs);

    // Set AHRS algorithm settings
    const FusionAhrsSettings settings = {
            .convention = FusionConventionNwu,
            .gain = 0.5f,
            .gyroscopeRange = 2000.0f, /* read from register 0x0F */
            .accelerationRejection = 10.0f,
            .magneticRejection = 10.0f,
            .recoveryTriggerPeriod = 5 * SAMPLE_RATE, /* 5 seconds */
    };
    FusionAhrsSetSettings(&ahrs, &settings);

    // timing stuff
    struct timespec t_prev, t_now;
    clock_gettime(CLOCK_MONOTONIC, &t_prev);

    double total_time = 0.0;
    int64_t iter_count = 0;
    double min_dt = 1e9, max_dt = 0.0;

    // This loop should repeat each time new gyroscope data is available
    while (true) {



        // Acquire latest sensor data

        // REPLACE WITH DATA TIMESTAMP
        const clock_t timestamp = clock(); 
        printf("timestamp: %ld", timestamp);

        // REPLACE WITH ACTUAL DATA
        FusionVector gyroscope = {0.0f, 0.0f, 0.0f}; 
        FusionVector accelerometer = {0.0f, 0.0f, 1.0f}; // replace this with actual accelerometer data in g

        // Apply calibration
        gyroscope = FusionCalibrationInertial(gyroscope, gyroscopeMisalignment, gyroscopeSensitivity, gyroscopeOffset);
        accelerometer = FusionCalibrationInertial(accelerometer, accelerometerMisalignment, accelerometerSensitivity, accelerometerOffset);

        // Update gyroscope offset correction algorithm
        gyroscope = FusionOffsetUpdate(&offset, gyroscope);

        // Calculate delta time (in seconds) to account for gyroscope sample clock error
        static clock_t previousTimestamp;
        const float deltaTime = (float) (timestamp - previousTimestamp) / (float) CLOCKS_PER_SEC;
        previousTimestamp = timestamp;

        FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);

        const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
        const FusionVector earth = FusionAhrsGetEarthAcceleration(&ahrs);

        printf("Roll %0.1f, Pitch %0.1f, Yaw %0.1f, X %0.1f, Y %0.1f, Z %0.1f\n",
               euler.angle.roll, euler.angle.pitch, euler.angle.yaw,
               earth.axis.x, earth.axis.y, earth.axis.z);

        clock_gettime(CLOCK_MONOTONIC, &t_now);
        double dT = timespec_to_seconds(&t_now) - timespec_to_seconds(&t_prev);
        t_prev = t_now;  // move current to previous

        total_time += dT;
        iter_count++;
        if (dT < min_dt) min_dt = dT;
        if (dT > max_dt) max_dt = dT;

        
        if (iter_count % 100 == 0) {
            double avg = total_time / (double)iter_count;
            printf("iter=%ld: avg=%0.6f s, min=%0.6f s, max=%0.6f s\n",
                   iter_count, avg, min_dt, max_dt);
            fflush(stdout);
        }
    }
}


void ten_sec_test(double time[10 * SAMPLE_RATE], double accel[10 * SAMPLE_RATE][3], double gyro[10 * SAMPLE_RATE][3], double output[10 * SAMPLE_RATE][3]) {

    // Define calibration (replace with actual calibration data if available)
    const FusionMatrix gyroscopeMisalignment = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const FusionVector gyroscopeSensitivity = {1.0f, 1.0f, 1.0f};
    const FusionVector gyroscopeOffset = {0.0f, 0.0f, 0.0f};
    const FusionMatrix accelerometerMisalignment = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const FusionVector accelerometerSensitivity = {1.0f, 1.0f, 1.0f};
    const FusionVector accelerometerOffset = {0.0f, 0.0f, 0.0f};
    const FusionMatrix softIronMatrix = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const FusionVector hardIronOffset = {0.0f, 0.0f, 0.0f};

    // Initialise algorithms
    FusionOffset offset;
    FusionAhrs ahrs;

    FusionOffsetInitialise(&offset, SAMPLE_RATE);
    FusionAhrsInitialise(&ahrs);

    // Set AHRS algorithm settings
    const FusionAhrsSettings settings = {
            .convention = FusionConventionNwu,
            .gain = 0.5f,
            .gyroscopeRange = 2000.0f, /* read from register 0x0F */
            .accelerationRejection = 10.0f,
            .magneticRejection = 10.0f,
            .recoveryTriggerPeriod = 5 * SAMPLE_RATE, /* 5 seconds */
    };
    FusionAhrsSetSettings(&ahrs, &settings);
    
    // This loop should repeat each time new gyroscope data is available
    for (int i = 0; i < (10 * SAMPLE_RATE); i++) {

        // Acquire latest sensor data

        const clock_t timestamp = time[i] * CLOCKS_PER_SEC; 

        FusionVector gyroscope = {gyro[i][0], gyro[i][1], gyro[i][2]}; 
        FusionVector accelerometer = {accel[i][0], accel[i][1], accel[i][2]};

        // Apply calibration
        gyroscope = FusionCalibrationInertial(gyroscope, gyroscopeMisalignment, gyroscopeSensitivity, gyroscopeOffset);
        accelerometer = FusionCalibrationInertial(accelerometer, accelerometerMisalignment, accelerometerSensitivity, accelerometerOffset);

        // Update gyroscope offset correction algorithm
        gyroscope = FusionOffsetUpdate(&offset, gyroscope);

        // Calculate delta time (in seconds) to account for gyroscope sample clock error
        static clock_t previousTimestamp;
        const float deltaTime = (float) (timestamp - previousTimestamp) / (float) CLOCKS_PER_SEC;
        previousTimestamp = timestamp;

        FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);

        const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
        const FusionVector earth = FusionAhrsGetEarthAcceleration(&ahrs);

        // printf("Roll %0.1f, Pitch %0.1f, Yaw %0.1f, X %0.1f, Y %0.1f, Z %0.1f\n",
        //        euler.angle.roll, euler.angle.pitch, euler.angle.yaw,
        //        earth.axis.x, earth.axis.y, earth.axis.z);

        output[i][0] = euler.angle.roll;
        output[i][1] = euler.angle.pitch;
        output[i][2] = euler.angle.yaw;        
    }
}

int add(int a, int b) {
    return a + b;
}