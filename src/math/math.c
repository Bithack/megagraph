#include "vector.h"
#include "matrix.h"
#include "misc.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

void
tmat4_dump(float *m)
{
    fprintf(stdout, "mat4(%p) = {\n" \
           "  %f, %f, %f, %f\n" \
           "  %f, %f, %f, %f\n" \
           "  %f, %f, %f, %f\n" \
           "  %f, %f, %f, %f\n" \
           "}\n",
           m,
           m[0], m[1], m[2], m[3],
           m[4], m[5], m[6], m[7],
           m[8], m[9], m[10], m[11],
           m[12], m[13], m[14], m[15]
           );
    fflush(stdout);
}

#ifdef TMS_FAST_MATH

static const float __sincosf_rng[2] = {
	2.0 / M_PI,
	M_PI / 2.0
};

static const float __sincosf_lut[8] = {
	-0.00018365f,
	-0.00018365f,
	+0.00830636f,
	+0.00830636f,
	-0.16664831f,
	-0.16664831f,
	+0.99999661f,
	+0.99999661f,
};

void tmath_sincos( float x, float *r0, float *r1)
{
	union {
		float 	f;
		int 	i;
	} ax, bx;
	
	float y;
	float a, b, c, d, xx, yy;
	int m, n, o, p;
	
	y = x + __sincosf_rng[1];
	ax.f = fabsf(x);
	bx.f = fabsf(y);
	
	m = (int) (ax.f * __sincosf_rng[0]);	
	o = (int) (bx.f * __sincosf_rng[0]);	
	ax.f = ax.f - (((float)m) * __sincosf_rng[1]);
	bx.f = bx.f - (((float)o) * __sincosf_rng[1]);
	
	n = m & 1;
	p = o & 1;
	ax.f = ax.f - n * __sincosf_rng[1];	
	bx.f = bx.f - p * __sincosf_rng[1];	
	m = m >> 1;
	o = o >> 1;
	n = n ^ m;
	p = p ^ o;
	m = (x < 0.0);
	o = (y < 0.0);
	n = n ^ m;	
	p = p ^ o;	
	n = n << 31;
	p = p << 31;
	ax.i = ax.i ^ n; 
	bx.i = bx.i ^ p; 

	xx = ax.f * ax.f;	
	yy = bx.f * bx.f;
	*r0 = __sincosf_lut[0];
	*r1 = __sincosf_lut[1];
	*r0 = (*r0) * xx + __sincosf_lut[2];
	*r1 = (*r1) * yy + __sincosf_lut[3];
	*r0 = (*r0) * xx + __sincosf_lut[4];
	*r1 = (*r1) * yy + __sincosf_lut[5];
	*r0 = (*r0) * xx + __sincosf_lut[6];
	*r1 = (*r1) * yy + __sincosf_lut[7];
	*r0 = (*r0) * ax.f;
	*r1 = (*r1) * bx.f;

}

static const float __sinf_rng[2] = {
	2.0 / M_PI,
	M_PI / 2.0
};

static const float __sinf_lut[4] = {
	-0.00018365f,
	-0.16664831f,
	+0.00830636f,
	+0.99999661f,
};

float tmath_sin(float x)
{
	union {
		float 	f;
		int 	i;
	} ax;
	
	float r, a, b, xx;
	int m, n;
	
	ax.f = fabsf(x);

	m = (int) (ax.f * __sinf_rng[0]);	
	ax.f = ax.f - (((float)m) * __sinf_rng[1]);

	n = m & 1;
	ax.f = ax.f - n * __sinf_rng[1];	
	m = m >> 1;
	n = n ^ m;
	m = (x < 0.0);
	n = n ^ m;	
	n = n << 31;
	ax.i = ax.i ^ n; 

	xx = ax.f * ax.f;	
	a = (__sinf_lut[0] * ax.f) * xx + (__sinf_lut[2] * ax.f);
	b = (__sinf_lut[1] * ax.f) * xx + (__sinf_lut[3] * ax.f);
	xx = xx * xx;
	r = b + a * xx;

	return r;
}

static const float __powf_rng[2] = {
	1.442695041f,
	0.693147180f
};

static const float __powf_lut[16] = {
	-2.295614848256274, 
	-2.470711633419806, 
	-5.686926051100417, 
	-0.165253547131978, 
	+5.175912446351073, 
	+0.844006986174912, 
	+4.584458825456749, 
	+0.014127821926000,	
	0.9999999916728642,	
	0.04165989275009526,
	0.5000006143673624, 
	0.0014122663401803872, 
	1.000000059694879, 	
	0.008336936973260111, 
	0.16666570253074878, 
	0.00019578093328483123
};

float tmath_pow(float x, float n)
{
	float a, b, c, d, xx;
	int m;
	
	union {
		float   f;
		int 	i;
	} r;
	
	r.f = x;
	m = (r.i >> 23);
	m = m - 127;
	r.i = r.i - (m << 23);
	
	xx = r.f * r.f;
	a = (__powf_lut[4] * r.f) + (__powf_lut[0]);
	b = (__powf_lut[6] * r.f) + (__powf_lut[2]);
	c = (__powf_lut[5] * r.f) + (__powf_lut[1]);
	d = (__powf_lut[7] * r.f) + (__powf_lut[3]);
	a = a + b * xx;
	c = c + d * xx;
	xx = xx * xx;
	r.f = a + c * xx;

	r.f = r.f + ((float) m) * __powf_rng[1];

	r.f = r.f * n;


	m = (int) (r.f * __powf_rng[0]);
	r.f = r.f - ((float) m) * __powf_rng[1];	
	
	a = (__powf_lut[12] * r.f) + (__powf_lut[8]);
	b = (__powf_lut[14] * r.f) + (__powf_lut[10]);
	c = (__powf_lut[13] * r.f) + (__powf_lut[9]);
	d = (__powf_lut[15] * r.f) + (__powf_lut[11]);
	xx = r.f * r.f;
	a = a + b * xx; 
	c = c + d * xx;
	xx = xx* xx;
	r.f = a + c * xx; 
	
	m = m << 23;
	r.i = r.i + m;

	return r.f;
}

static const float __atan2f_lut[4] = {
	-0.0443265554792128,
	-0.3258083974640975,
	+0.1555786518463281,
	+0.9997878412794807 
}; 
 
static const float __atan2f_pi_2 = M_PI_2;

float tmath_atan2(float y, float x)
{
	float a, b, c, r, xx;
	int m;
	union {
		float f;
		int i;
	} xinv;

	xx = fabs(x);
	xinv.f = xx;
	m = 0x3F800000 - (xinv.i & 0x7F800000);
	xinv.i = xinv.i + m;
	xinv.f = 1.41176471f - 0.47058824f * xinv.f;
	xinv.i = xinv.i + m;
	b = 2.0 - xinv.f * xx;
	xinv.f = xinv.f * b;	
	b = 2.0 - xinv.f * xx;
	xinv.f = xinv.f * b;
	
	c = fabs(y * xinv.f);

	xinv.f = c;
	m = 0x3F800000 - (xinv.i & 0x7F800000);
	xinv.i = xinv.i + m;
	xinv.f = 1.41176471f - 0.47058824f * xinv.f;
	xinv.i = xinv.i + m;
	b = 2.0 - xinv.f * c;
	xinv.f = xinv.f * b;	
	b = 2.0 - xinv.f * c;
	xinv.f = xinv.f * b;
	
	xinv.f = xinv.f + c;
	a = (c > 1.0f);
	c = c - a * xinv.f;
	r = a * __atan2f_pi_2;
	
	xx = c * c;	
	a = (__atan2f_lut[0] * c) * xx + (__atan2f_lut[2] * c);
	b = (__atan2f_lut[1] * c) * xx + (__atan2f_lut[3] * c);
	xx = xx * xx;
	r = r + a * xx; 
	r = r + b;

	b = M_PI;
	b = b - 2.0f * r;
	r = r + (x < 0.0f) * b;
	b = (fabs(x) < 0.000001f);
	c = !b;
	r = c * r;
	r = r + __atan2f_pi_2 * b;
	b = r + r;
	r = r - (y < 0.0f) * b;
	
	return r;
}

float tmath_sqrt(float x)
{
	float b, c;
	int m;
	union {
		float 	f;
		int 	i;
	} a;
	
	a.f = x;
	a.i = 0x5F3759DF - (a.i >> 1);
	c = x * a.f;
	b = (3.0f - c * a.f) * 0.5;	
	a.f = a.f * b;
	c = x * a.f;
	b = (3.0f - c * a.f) * 0.5;
    a.f = a.f * b;

	/*fast inverse approx*/
	x = a.f;
	m = 0x3F800000 - (a.i & 0x7F800000);
	a.i = a.i + m;
	a.f = 1.41176471f - 0.47058824f * a.f;
	a.i = a.i + m;
	b = 2.0 - a.f * x;
	a.f = a.f * b;
	b = 2.0 - a.f * x;
	a.f = a.f * b;

	return a.f;
}

#endif

/**
 * Invert the given matrix in-place.
 * @returns 0 if the matrix could not be inverted, 1 otherwise
 *
 * @relates tmatN
 **/
int
tmat4_invert(float *out)
{
    float m[16];
    tmat4_copy(m, out);

    float a0 = m[0]*m[5] - m[1]*m[4],
          a1 = m[0]*m[6] - m[2]*m[4],
          a2 = m[0]*m[7] - m[3]*m[4],
          a3 = m[1]*m[6] - m[2]*m[5],
          a4 = m[1]*m[7] - m[3]*m[5],
          a5 = m[2]*m[7] - m[3]*m[6],
          b0 = m[8]*m[13] - m[9]*m[12],
          b1 = m[8]*m[14] - m[10]*m[12],
          b2 = m[8]*m[15] - m[11]*m[12],
          b3 = m[9]*m[14] - m[10]*m[13],
          b4 = m[9]*m[15] - m[11]*m[13],
          b5 = m[10]*m[15] - m[11]*m[14] ;

    float det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;

    if (fabsf(det) > 0.00001f) {
        out[0] = m[5]*b5 - m[6]*b4 + m[7]*b3;
        out[4] = -m[4]*b5 + m[6]*b2 - m[7]*b1;
        out[8] = m[4]*b4 - m[5]*b2 + m[7]*b0;
        out[12] = -m[4]*b3 + m[5]*b1 - m[6]*b0;
        out[1] = -m[1]*b5 + m[2]*b4 - m[3]*b3;
        out[5] = m[0]*b5 - m[2]*b2 + m[3]*b1;
        out[9] = -m[0]*b4 + m[1]*b2 - m[3]*b0;
        out[13] = m[0]*b3 - m[1]*b1 + m[2]*b0;
        out[2] = m[13]*a5 - m[14]*a4 + m[15]*a3;
        out[6] = -m[12]*a5 + m[14]*a2 - m[15]*a1;
        out[10] = m[12]*a4 - m[13]*a2 + m[15]*a0;
        out[14] = -m[12]*a3 + m[13]*a1 - m[14]*a0;
        out[3] = -m[9]*a5 + m[10]*a4 - m[11]*a3;
        out[7] = m[8]*a5 - m[10]*a2 + m[11]*a1;
        out[11] = -m[8]*a4 + m[9]*a2 - m[11]*a0;
        out[15] = m[8]*a3 - m[9]*a1 + m[10]*a0;

        float invdet = 1.f/det;
        for (int x=0; x<16; x++)
            out[x] *= invdet;

        return 1;
    }

    return 0;
}

void
tmat4_transpose(float *m)
{
    float tmp[16];
    tmat4_copy(tmp, m);
    for (int r=0; r<4; r++)
        for (int c=0; c<4; c++)
            m[r*4+c] = tmp[c*4+r];
}

/* gives 0 if f == 0, -1 if f < 0 and 1 if f > 0 */
#define TSIGN(f) (float)((f > 0) - (f < 0))

/**
 * Set projection matrix `m`'s near plane to `plane`, to clip
 * the scene against that plane.
 * Assumes `plane` is in clip-space.
 *
 * @relates tmatN
 **/
void
tmat4_set_near_plane(float *m, tvec4 *plane)
{
    float mn[] = TMAT4_IDENTITY;
/*
    float dot;
    tvec4 p = {
        .x = (TSIGN(plane->x) + m[8]) / m[0],
        .y = (TSIGN(plane->y) + m[9]) / m[5],
        .z = -1.f,
        .w = (1.f + m[10]) / m[14]
    };

    dot = tvec4_dot(&p, plane);
*/

    float fz = fabsf(plane->z);

    tvec4 p = (tvec4){
        plane->x / fz, plane->y / fz,
        plane->z / fz, plane->w / fz
    };
    p.w -= 1;
    ////if (p.z < 0) tvec4_mul(&p,-1);

    mn[2] = p.x;
    mn[6] = p.y;
    mn[10] = p.z;
    mn[14] = p.w;

    tmat4_multiply(mn, m);
    tmat4_copy(m, mn);

/*
    m[2] = p.x * (2.f/dot);
    m[6] = p.y * (2.f/dot);
    m[10] = p.z * (2.f/dot) + 1.f;
    m[14] = p.w * (2.f/dot);
*/
}

/** 
 * Generate perspective projection matrix.
 *
 * @relates tmatN
 **/
void tmat4_perspective(float *result, float fovy, float aspect, float zNear, float zFar)
{
    float xmin, xmax, ymin, ymax;

    ymax = zNear * tanf(fovy * 3.141592653589793f / 360.0f);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    tmat4_frustum(result, xmin, xmax, ymin, ymax, zNear, zFar);
}

void
tmat4_set_ortho(float *result,
                     float left, float right, float bottom, float top,
                     float _near, float _far)
{
    tmat4_load_identity(result);

    float x_o = 2.f / (right - left);
    float y_o = 2.f / (top - bottom);
	float z_o = -2.f / (_far - _near);

    float tx = -(right+left)/(right-left);
    float ty = -(top+bottom)/(top-bottom);
	float tz = -(_far + _near) / (_far - _near);

    result[0] = x_o;
    result[1] = 0;
    result[2] = 0;
    result[3] = 0;

    result[4] = 0;
    result[5] = y_o;
    result[6] = 0;
    result[7] = 0;

    result[8] = 0;
    result[9] = 0;
    result[10] = z_o;
    result[11] = 0;

    result[12] = tx;
    result[13] = ty;
    result[14] = tz;
    result[15] = 1;
}

void
tmat4_frustum(float *result,
               float left, float right, float bottom, float top,
               float nearVal, float farVal)
{
    result[0] = 2.0f*nearVal/(right-left);
    result[1] = 0.0f;
    result[2] = 0.0f;
    result[3] = 0.0f;
    result[4] = 0.0f;
    result[5] = 2.0f*nearVal/(top-bottom);
    result[6] = 0.0f;
    result[7] = 0.0f;
    result[8] = (right+left)/(right-left);
    result[9] = (top+bottom)/(top-bottom);
    result[10] = -(farVal+nearVal)/(farVal-nearVal);
    result[11] = -1.0f;
    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = -(2.0f*farVal*nearVal)/(farVal-nearVal);
    result[15] = 0.0f;
}

void
tmat4_load_identity(float *m)
{
    static float __identity[16] = TMAT4_IDENTITY;
    memcpy(m, __identity, TMAT4_SIZE);
}

void
tmat3_load_identity(float *m)
{
    static float __identity[9] = TMAT3_IDENTITY;
    memcpy(m, __identity, TMAT3_SIZE);
}

void
tmat4_multiply_reverse(float m2[16], float m1[16])
{
    float tmp[16];
    int x,y;

    for (x=0; x<16; x++) {
        tmp[x] = 0.f;

        for (y=0; y<4; y++)
            tmp[x] += m1[(x%4) + (y*4)] * m2[y + ((x/4)*4)];
    }
    memcpy(m2, tmp, TMAT4_SIZE);
    /*
    float tmp[16];
    int x,y;

    for (x=0; x<4; x++) {
        for (y=0; y<4; y++) {
            tmp[y+(x*4)] = (m1[x*4]*m2[y])+ (m1[x*4+1] * m2[y+4])
                          + (m1[x*4+2] * m2[y+8]) + (m1[x*4+3] * m2[y+12]);
        }
    }
    memcpy(m1, tmp, TMAT4_SIZE);
    */
}

void
tmat4_multiply(float m1[16], float m2[16])
{
    float tmp[16];
    int x,y;

    for (x=0; x<16; x++) {
        tmp[x] = 0.f;

        for (y=0; y<4; y++)
            tmp[x] += m1[(x%4) + (y*4)] * m2[y + ((x/4)*4)];
    }
    memcpy(m1, tmp, TMAT4_SIZE);
    /*
    float tmp[16];
    int x,y;

    for (x=0; x<4; x++) {
        for (y=0; y<4; y++) {
            tmp[y+(x*4)] = (m1[x*4]*m2[y])+ (m1[x*4+1] * m2[y+4])
                          + (m1[x*4+2] * m2[y+8]) + (m1[x*4+3] * m2[y+12]);
        }
    }
    memcpy(m1, tmp, TMAT4_SIZE);
    */
}

void
tmat4_scale(float *m, float x, float y, float z)
{
    float mm[16] = TMAT4_IDENTITY;
    mm[0] = x;
    mm[5] = y;
    mm[10] = z;
    tmat4_multiply(m, mm);
}

void
tmat4_translate(float *m, float x, float y, float z)
{
    float mm[16] = TMAT4_IDENTITY;
    mm[12] = x;
    mm[13] = y;
    mm[14] = z;
    tmat4_multiply(m, mm);
}

void
tmat4_rotate(float *m, float a, float x,float y, float z)
{
    float angle = a*(3.1415926/180);
    float m2[16] = {0};

    m2[0] = 1+(1-cos(angle))*(x*x-1);
    m2[1] = -z*sin(angle)+(1-cos(angle))*x*y;
    m2[2] = y*sin(angle)+(1-cos(angle))*x*z;
    m2[3] = 0;

    m2[4] = z*sin(angle)+(1-cos(angle))*x*y;
    m2[5] = 1+(1-cos(angle))*(y*y-1);
    m2[6] = -x*sin(angle)+(1-cos(angle))*y*z;
    m2[7] = 0;

    m2[8] = -y*sin(angle)+(1-cos(angle))*x*z;
    m2[9] = x*sin(angle)+(1-cos(angle))*y*z;
    m2[10] = 1+(1-cos(angle))*(z*z-1);
    m2[11] = 0;

    m2[12] = 0;
    m2[13] = 0;
    m2[14] = 0;
    m2[15] = 1;

    tmat4_multiply(m, m2);
}

/* lookat taken from GLUS */
void tmat4_lookat(float *result,
                   float eyeX, float eyeY, float eyeZ,
                   float centerX, float centerY, float centerZ,
                   float upX, float upY, float upZ)
{
    tvec3 forward, side, up;
    float matrix[16];

    forward.x = centerX - eyeX;
    forward.y = centerY - eyeY;
    forward.z = centerZ - eyeZ;
    tvec3_normalize(&forward);

    up.x = upX;
    up.y = upY;
    up.z = upZ;
    
    tvec3_cross(&side, forward, up);
    tvec3_normalize(&side);
    tvec3_cross(&up, side, forward);

    matrix[0] = side.x;
    matrix[1] = up.x;
    matrix[2] = -forward.x;
    matrix[3] = 0.0f;
    matrix[4] = side.y;
    matrix[5] = up.y;
    matrix[6] = -forward.y;
    matrix[7] = 0.0f;
    matrix[8] = side.z;
    matrix[9] = up.z;
    matrix[10] = -forward.z;
    matrix[11] = 0.0f;
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;

    tmat4_copy(result, matrix);
    tmat4_translate(result, -eyeX, -eyeY, -eyeZ);
}

/**
 * Multiply the vector by the matrix, dividing the
 * components by w 
 *
 * @relates tvec3
 **/
void
tvec3_project_mat4(tvec3 *v, float *m)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;
    float w = x * m[3] + y*m[7] + z * m[11] + m[15];

    v->x = (x*m[0] + y*m[4] + z*m[8] + m[12]) / w;
    v->y = (x*m[1] + y*m[5] + z*m[9] + m[13]) / w;
    v->z = (x*m[2] + y*m[6] + z*m[10] + m[14]) / w;
}

void
tvec4_mul_mat4(tvec4 *v, float *m)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;
    float w = v->w;

    v->x = (x*m[0] + y*m[4] + z*m[8] + w*m[12]);
    v->y = (x*m[1] + y*m[5] + z*m[9] + w*m[13]);
    v->z = (x*m[2] + y*m[6] + z*m[10] + w*m[14]);
    v->w = (x*m[3] + y*m[7] + z*m[11] + w*m[15]);
}

void
tvec3_mul_mat3(tvec3 *v, float *m)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;

    v->x = (x*m[0] + y*m[4] + z*m[8]);
    v->y = (x*m[1] + y*m[5] + z*m[9]);
    v->z = (x*m[2] + y*m[6] + z*m[10]);
}
