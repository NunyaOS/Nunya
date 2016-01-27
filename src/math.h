/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MATH_H
#define MATH_H

#define M_PI 3.14159265358979323846     /* pi */

/**
 * @brief A cosine approximation of x
 * @details This functions approximates a cosine value of the parameter
 * using polynomial approximations
 *
 * @param x The value, between 0 and 2 PI
 * @return The approximate cosine value
 */
double cos(double x);

/**
 * @brief A sine approximation of x
 * @details This functions approximates a sine value of the parameter
 * using polynomial approximations
 *
 * @param x The value, between 0 and 2 PI
 * @return The approximate sine value
 */
double sin(double x);

/**
 * @brief A tangent approximation of x
 * @details This approximates a tangent value of the parameter
 *
 * @param x The value, between 0 and 2 PI
 * @return The approximate tangent value
 */
double tan(double x);

/** 
 * @brief absolute value of x
 * 
 * @details returns -x if x is less than 0; x otherwise
 * 
 * @param x some number
 * 
 * @return absolute value of x
 */
inline double abs(double x);

/** 
 * @brief ceiling of x
 * 
 * @details returns the ceiling of a number. because of casting, this may not correctly grab the
 * ceiling of a number with no fractional part
 * 
 * @param x some number
 * 
 * @return ceiling of x
 */
inline double ceil(double x);

/** 
 * @brief floor of x
 * 
 * @details returns the floor of a number
 * 
 * @param x some number
 * 
 * @return floor of x
 */
inline double floor(double x);

#endif
