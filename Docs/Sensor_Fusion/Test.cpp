#include <bits/stdc++.h>
using namespace std;

int main() {
    double g = 9.81;

    double pitch = 0.5;  // ~28.6°
    double roll  = 0.3;  // ~17.2°

    double acc_x = g * sin(pitch);
    double acc_y = -g * sin(roll) * cos(pitch);
    double acc_z = -g * cos(roll) * cos(pitch);

    cout << "acc_x: " << acc_x << endl;
    cout << "acc_y: " << acc_y << endl;
    cout << "acc_z: " << acc_z << endl;

    double calc_pitch = atan2(acc_x, sqrt(acc_y * acc_y + acc_z * acc_z));
    double calc_roll  = atan2(-acc_y, -acc_z);

    cout << "\ncalc_pitch: " << calc_pitch << " rad" << endl;
    cout << "calc_roll: "  << calc_roll  << " rad" << endl;
}
