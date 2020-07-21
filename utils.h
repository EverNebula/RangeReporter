#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <vector>
using std::vector;
using std::pair;

typedef pair<int, int> PII;
typedef pair<int, double> PID;
typedef pair<double, double> PDD;
#define mp std::make_pair
#define ft first
#define sc second
#define EPS 1e-6

#define ASSERT(condition)                                                     \
    if (!(condition)) {                                                       \
        fprintf(stderr, "Assertion failed: line %d, file \"%s\"\n",           \
                __LINE__, __FILE__);                                          \
		fflush(stderr);							      						  \
        exit(-1);                                                             \
    }

void dprintf(char *format, ...);
void vprintf(char *format, ...);
void panic(char *format, ...);

class Timer
{
public:
	Timer();
	~Timer();

	void Start();
	double StepTime(); // return step process time (sec)
	double Finish(bool force_update=false);
	double WholeTime();

private:
	struct timeval st_time, ed_time, ls_time;
	bool stopped;
};

bool verbose = false;
bool debug = false;

void 
dprintf(char *format, ...)
{
    if (debug)
    {
		va_list ap;
		va_start(ap, format);
		vfprintf(stdout, format, ap);
		va_end(ap);
		fflush(stdout);
    }
}

void
vprintf(char *format, ...)
{
    if (verbose)
    {
		va_list ap;
		va_start(ap, format);
		vfprintf(stdout, format, ap);
		va_end(ap);
		fflush(stdout);
    }
}

void
panic(char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
	fflush(stdout);
	exit(-1);
}


Timer::Timer()
{
	Start();
}

Timer::~Timer()
{

}

void
Timer::Start()
{
	gettimeofday(&st_time, NULL);
	ls_time.tv_sec = st_time.tv_sec;
	ls_time.tv_usec = st_time.tv_usec;
	stopped = false;
}

double 
Timer::StepTime()
{
	struct timeval tmp;
	gettimeofday(&tmp, NULL);
	double res = (tmp.tv_sec - ls_time.tv_sec) + (double)(tmp.tv_usec - ls_time.tv_usec) / 1000000;
	ls_time.tv_sec = tmp.tv_sec;
	ls_time.tv_usec = tmp.tv_usec;
	return res;
}

double 
Timer::Finish(bool force_update)
{
	if (!stopped || force_update)
	{
		stopped = true;
		gettimeofday(&ed_time, NULL);
	}
	return this -> WholeTime();
}

double 
Timer::WholeTime()
{
	double res = (ed_time.tv_sec - st_time.tv_sec) + (double)(ed_time.tv_usec - st_time.tv_usec) / 1000000;
	return res;
}

#endif