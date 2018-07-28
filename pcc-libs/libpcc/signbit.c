/*	$Id: signbit.c,v 1.1 2018/07/27 16:30:00 ragge Exp $	*/
/*
 * Simple signbit extraction code.
 * Written by Anders Magnusson.  Public domain.
 */

union sbit {
	float f;
	double d;
	long double l;
	unsigned u[4];
};

int
__signbitf(float x)
{
	union sbit s;

	s.f = x;
	return s.u[0] >> 31;
}

int
__signbitd(double x)
{
	union sbit s;

	s.d = x;
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return s.u[1] >> 31;
#else
	return s.u[0] >> 31;
#endif
}

int
__signbitl(long double x)
{
	union sbit s;

	s.l = x;
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#if __i386__ || __amd64__
	return (s.u[2] >> 15) & 1;
#else 
	return s.u[3] >> 31;
#endif
#else
	return s.u[0] >> 31;
#endif
}

