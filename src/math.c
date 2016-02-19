/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "math.h"

// This approximation from http://lab.polygonal.de/?p=205
double cos(double x) {
    double cos;
    // Round x to -PI to PI
    if (x < -3.14159265) {
        x += 6.28318531;
    } else if (x > 3.14159265) {
        x -= 6.28318531;
    }

    // cosine is sin(X + PI/2)
    x += 1.57079632;
    if (x > 3.14159265) {
        x -= 6.28318531;
    }

    if (x < 0) {
        cos = 1.27323954 * x + 0.405284735 * x * x;

        if (cos < 0) {
            cos = .225 * (cos * -cos - cos) + cos;
        } else {
            cos = .225 * (cos * cos - cos) + cos;
        }
    } else {
        cos = 1.27323954 * x - 0.405284735 * x * x;

        if (cos < 0) {
            cos = .225 * (cos * -cos - cos) + cos;
        } else {
            cos = .225 * (cos * cos - cos) + cos;
        }
    }
    return cos;
}

// This approximation from http://lab.polygonal.de/?p=205
double sin(double x) {
    double sin;
    // Round x to -PI to PI
    if (x < -3.14159265) {
        x += 6.28318531;
    } else if (x > 3.14159265) {
        x -= 6.28318531;
    }

    if (x < 0) {
        sin = 1.27323954 * x + .405284735 * x * x;

        if (sin < 0) {
            sin = .225 * (sin * -sin - sin) + sin;
        } else {
            sin = .225 * (sin * sin - sin) + sin;
        }
    } else {
        sin = 1.27323954 * x - 0.405284735 * x * x;

        if (sin < 0) {
            sin = .225 * (sin * -sin - sin) + sin;
        } else {
            sin = .225 * (sin * sin - sin) + sin;
        }
    }

    return sin;
}

double tan(double x) {
    return sin(x) / cos(x);
}


double ceil(double x) {
    int intnum = (int)x;
    // if x has no floating point part
    if (x == (double)intnum) {
        return x;
    }
    return x < 0 ? (double)intnum : (double)(intnum + 1);
}

double floor(double x) {
    return (double)((int)x);
}


