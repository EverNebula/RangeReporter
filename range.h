#ifndef RANGE_HEADER
#define RANGE_HEADER

#include "BOBHash32.h"
#include "utils.h"

class RangeBF
{
private:
	int size, beta;
	double max_ts, unit_ts;

	int *bf;
	BOBHash32 hash;

	void recursive_insert(int dep, uint32_t hsh, double t, int l, int r);
	void recursive_query(int dep, uint32_t hsh, int l, int r, vector<PDD>& ans);

public:
	RangeBF(int size, int beta, double max_ts)
	: size(size), beta(beta), max_ts(max_ts) { unit_ts = max_ts / size; }
	~RangeBF();
	void init();
	void insert(int v, double t);
	vector<PDD> query(int v);
};

RangeBF::~RangeBF()
{
	if (bf)
		delete [] bf;
}

void
RangeBF::init()
{
	hash.initialize(2020);
	bf = new int[size];
	memset(bf, 0, size * sizeof(int));
}

void
RangeBF::insert(int v, double t)
{
	if (t > max_ts + EPS)
	{
		printf("ERROR: timestamp is too big.\n");
		exit(0);
	}

	uint32_t hsh = hash.run((char*)&v, sizeof(int));
	recursive_insert(0, hsh, t, 0, size);
}

void
RangeBF::recursive_insert(int dep, uint32_t hsh, double t, int l, int r)
{
	// printf("%d %d %d %d %d\n", dep, hsh, t, l, r);
	if (dep == beta || r - l <= 1)
		return;

	if (l > r)
	{
		printf("ERROR: insert bound error.\n");
		exit(0);
	}

	int len = r - l;
	int pos = hsh % len + l;
	double pos_ts = pos * unit_ts;
	// printf("%d\n", pos);

	if (pos_ts <= t)
	{
		bf[pos] |= 1;
		recursive_insert(dep+1, hsh, t, pos, r);
	}
	else
	{
		bf[pos] |= 2;
		recursive_insert(dep+1, hsh, t, l, pos+1);
	}
}

vector<PDD>
RangeBF::query(int v)
{
	uint32_t hsh = hash.run((char*)&v, sizeof(int));
	vector<PDD> ans;

	recursive_query(0, hsh, 0, size, ans);

	return ans;
}

void
RangeBF::recursive_query(int dep, uint32_t hsh, int l, int r, vector<PDD>& ans)
{
	// printf("%d %d %d %d\n", dep, hsh, l, r);

	if (dep == beta || r - l <= 1)
	{
		ans.push_back(mp(unit_ts*l, unit_ts*r));
		return;
	}

	if (l > r)
	{
		printf("ERROR: query bound error.\n");
		exit(0);
	}

	int len = r - l;
	int pos = hsh % len + l;

	if (bf[pos] & 2)
		recursive_query(dep+1, hsh, l, pos+1, ans);
	if (bf[pos] & 1)
		recursive_query(dep+1, hsh, pos, r, ans);
}

#endif