/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "math.h"

// This approximation from http://lab.polygonal.de/?p=205
double cos( double x ) {
	// Round x to -PI to PI
	if (x < -3.14159265)
		x += 6.28318531;
	else if (x >  3.14159265)
		x -= 6.28318531;

	// cosine is sin(X + PI/2)
	x += 1.57079632;
	if (x >  3.14159265)
		x -= 6.28318531;

	if (x < 0)
		return 1.27323954 * x + 0.405284735 * x * x;
	return 1.27323954 * x - 0.405284735 * x * x;
}

// This approximation from http://lab.polygonal.de/?p=205
double sin( double x ) {
	// Round x to -PI to PI
	if (x < -3.14159265)
		x += 6.28318531;
	else if (x >  3.14159265)
		x -= 6.28318531;

	if (x < 0)
		return 1.27323954 * x + 0.405284735 * x * x;
	return 1.27323954 * x - 0.405284735 * x * x;
}

// This approximation from http://lab.polygonal.de/?p=205
double tan( double x ) {
	// Round x to -PI to PI
	if (x < -3.14159265)
		x += 6.28318531;
	else if (x >  3.14159265)
		x -= 6.28318531;

	double val = 0.0;

	// First calculate the sine value
	if (x < 0)
		val = 1.27323954 * x + 0.405284735 * x * x;
	val = 1.27323954 * x - 0.405284735 * x * x;

	// cosine is sin(X + PI/2)
	x += 1.57079632;
	if (x >  3.14159265)
		x -= 6.28318531;

	if (x < 0)
		return val / (1.27323954 * x + 0.405284735 * x * x);
	return val / (1.27323954 * x - 0.405284735 * x * x);
}

