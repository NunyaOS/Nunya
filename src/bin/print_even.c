void main();
void _start() {
    main();
}

#include "syscall.h"

// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -------- Below is dummy code to make the process larger than one page
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------

#define M_PI 3.1415926

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

int abs(int x) {
  if (x < 0) x *= -1;
  return x;
}

void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
  struct graphics_color draw_color = {0,255,0};
  draw_line(x1,y1,x2,y2,&draw_color);
  draw_line(x2,y2,x3,y3,&draw_color);
  draw_line(x3,y3,x1,y1,&draw_color);

  return;
}

void drawSquare(int x, int y, int s) {
  struct graphics_color draw_color = {0,255,0};
  draw_line(x,y,x,y+s,&draw_color);
  draw_line(x,y,x+s,y,&draw_color);
  draw_line(x+s,y,x+s,y+s,&draw_color);
  draw_line(x,y+s,x+s,y+s,&draw_color);

  return;
}

void drawCircle(int x, int y, int r) {
  struct graphics_color draw_color = {0,255,0};
  float t = 0.0;

  for(t = 0.0; t < 2*M_PI; t+=.03) {
    int x1 = r*cos(t) + x;
    int y1 = r*sin(t) + y;

    int x2 = r*cos(t+.03) + x;
    int y2 = r*sin(t+.03) + y;

    draw_line(x1,y1,x2,y2,&draw_color);
  }

  return;
}

void drawFlake(int x, int y, int r) {
  struct graphics_color draw_color = {0,255,0};
  float t = 0.0;
  for(t = 0.0; t < 2*M_PI; t+=(2*M_PI/5)) {
    int x2 = r*cos(t) + x;
    int y2 = r*sin(t) + y;

    draw_line(x, y, x2, y2,&draw_color);
  }

  return;
}

void drawFern(int x, int y, int r, float t) {
  struct graphics_color draw_color = {0,255,0};
  //flip the sign and cos so that angle 0 is straight up instead of to right
  int x1 = r*sin(t) + x;
  int y1 = r*cos(t) + y;
  draw_line(x, y, x1, y1,&draw_color);

  int i = 1;
  for(i = 1; i <= 4; i++) {
    int nx = (r/4*i)*sin(t) + x;
    int ny = (r/4*i)*cos(t) + y;

    //points going off of that spot on trunk
    int nx2 = (r/4)*sin(t+.6) + nx;
    int ny2 = (r/4)*cos(t+.6) + ny;
    draw_line(nx,ny,nx2,ny2,&draw_color);

    int nx3 = (r/4)*sin(t-.6) + nx;
    int ny3 = (r/4)*cos(t-.6) + ny;
    draw_line(nx,ny,nx3,ny3,&draw_color);
  }

  return;
}

void drawSpiral(int x, int y, int sr, float st) {
  struct graphics_color draw_color = {0,255,0};
  float t = 0.0;

  for(t = 0.0; t < 4*M_PI; t+=.01) {
    //the radius is proportional to the theta, so that it grows
    float r = sr*t/12;
    int x1 = r*cos(t+st) + x;
    int y1 = r*sin(t+st) + y;
    draw_line(x1,y1,x1,y1,&draw_color);
  }

  return;
}

//draws the sierpinski triangle
void fractal_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
  if(abs(x2-x1) < 3)
    return;

  //drawing case
  drawTriangle(x1,y1,x2,y2,x3,y3);

  //recurse
  fractal_triangle(x1,y1,(x1+x2)/2,(y1+y2)/2,(x1+x3)/2,(y1+y3)/2);
  fractal_triangle(x2,y2,(x1+x2)/2,(y1+y2)/2,(x2+x3)/2,(y2+y3)/2);
  fractal_triangle(x3,y3,(x2+x3)/2,(y2+y3)/2,(x1+x3)/2,(y1+y3)/2);

  return;
}

//draws a square with a new square at each of its 4 corners
void fractal_square(int x, int y, int s) {
  if(s < 2)
    return;

  //drawing case
  drawSquare(x,y,s);

  //recurse
  fractal_square(x-(s/5),y-(s/5),(2*s/5));
  fractal_square(x+(4*s/5),y-(s/5),(2*s/5));
  fractal_square(x-(s/5),y+(4*s/5),(2*s/5));
  fractal_square(x+(4*s/5),y+(4*s/5),(2*s/5));

  return;

}

//draws squares of ever-decreasing size that spiral towards the center
void spiral_squares(int cx, int cy, int s, int r, float t) {
  if(r < 10)
    return;

  //drawing case
  float x = r*cos(t) + cx;
  float y = r*sin(t) + cy;

  drawSquare(x,y,s);

  //prevent theta from being greater than 2PI
  //probably unnecessary
  float nt = t+.75 > 2*M_PI ? t+.75-(2*M_PI) : t+.75;

  //recurse
  spiral_squares(cx,cy,9*s/10, 9*r/10, nt);

  return;

}

//draws circles that have 6 circles along their arc
void circularLace(int cx, int cy, int r) {
  if (r < 2) {
    return;
  }

  //drawing case
  drawCircle(cx, cy, r);

  //recurse
  float t = 0.0f;
  //go from 0 to 2PI by 1/3PI. --> 6 circles
  for(t = 0.0; t < 2*M_PI-(M_PI/3); t+=(M_PI/3)) {
    int x = r*cos(t) + cx;
    int y = r*sin(t) + cy;

    circularLace(x, y, r/3);
  }

  return;
}

//draws a five-sided snowflake where each point is the location of a smaller one
void snowflake(int cx, int cy, int r) {
  if(r <= 1)
    return;

  //drawing case
  drawFlake(cx, cy, r);

  //recurse
  float t = 0.0;
  for(t = 0.0; t < 2*M_PI; t+=(2*M_PI/5)) {
    int x = r*cos(t) + cx;
    int y = r*sin(t) + cy;

    snowflake(x, y, r/3);
  }

  return;

}

//draws a binary tree
void tree(int cx, int cy, int r, float t) {
  struct graphics_color draw_color = {0,255,0};
  if(r <= 1)
    return;

  //drawing case
  int x = r*cos(t) + cx;
  int y = r*sin(t) + cy;
  draw_line(cx,cy,x,y,&draw_color);

  //recurse
  tree(x,y,r/1.5,t-.62);
  tree(x,y,r/1.5,t+.62);

  return;
}

void fern(int cx, int cy, int r, float t) {
  if(r <= 5)
    return;

  //drawing case
  drawFern(cx,cy,r,t);

  //recurse
  int i = 1;
  for(i=1; i <= 4; i++) {
    int x = (r/4*i)*sin(t) + cx;
    int y = (r/4*i)*cos(t) + cy;
    fern(x,y,r/3,t+.6);
    fern(x,y,r/3,t-.6);
  }

  return;
}

//draws the spiral of spirals!
void spirals(int cx, int cy, int r, float t) {
  if(r <= 6) {
    //base case IS the drawing case
    //we wont actually draw any spiral > r=6
    drawSpiral(cx, cy, r, t);
    return;
  }

  //the main spiral recurse
  float nt = t+.65 > 2*M_PI ? t+.65-(2*M_PI) : t+.65;
  spirals(cx,cy, 11*r/12, nt);

  //recurse at each point to draw a new, smaller spiral there
  float x = r*cos(t) + cx;
  float y = r*sin(t) + cy;
  spirals(x,y,r/3,t);

}

int pie() {

  //constant screen size values
  const int xc = 350;
  const int yc = 350;

  const int x_size = 400;
  const int y_size = 400;

  // gfx_open(x_size,y_size,"Fractals!");
  create_window(10, 10, 400, 400);

  // gfx_color(255,255,255);

  char c;
  int isRunning = 1;

  struct event e;

  while(isRunning) {
    // if(gfx_event_waiting()) {
    //   c = gfx_wait();

    if(get_event(&e) == 0) {
      c = e.character;

      switch(c) {
      case '1':
  clear();;
  fractal_triangle(100,y_size-200,xc,20,x_size-100,y_size-200);

      break;
    case '2':
      clear();;
      fractal_square(200,200,300);
      break;
    case '3':
      clear();;
      spiral_squares(xc,yc,70, 300, 0);
      break;
      case '4':
  clear();;
        circularLace(xc,yc,200);
  break;
      case '5':
  clear();;
  snowflake(xc,yc,200);
  break;
      case '6':
  clear();;
  tree(xc,y_size-100,200,-M_PI/2);

  break;
      case '7':
  clear();;
  fern(xc,y_size,450,M_PI);;

  break;
      case '8':
  clear();;
  spirals(xc,yc,300,0.0f);

  break;
    case 'q':
      isRunning = 0;
      break;
    default:
      break;
      }
    }
  }

  exit(1);
  return 1;
}


// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// ---------- End the dummy code to make the process larger than one page
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------


void main() {
    int i = 0;
    for (i = 0; i < 11; i+=2) {
        debug_print(i);
        int j;
        for (j = 0; j < 500000000; j++) {} // busywait
    }

    exit(0);
}
