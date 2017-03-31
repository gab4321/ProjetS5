#include <math.h>

/* generate real and imaginary twiddle
table of size n/2 complex numbers */

gen_w_r2(float* w, int n)
{
    int i;
    float pi = 4.0*atan(1.0);
    float e = pi*2.0/n;
    for(i=0; i < ( n>>1 ); i++)
    {
        w[2*i] = cos(i*e);
        w[2*i+1] = sin(i*e);
    }
}
