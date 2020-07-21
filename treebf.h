#ifndef TREEBF_HEADER
#define TREEBF_HEADER

#include "BOBHash32.h"
#include "utils.h"

class Node
{
public:
	int size, num_hash;
	Node *l, *r;
	double begin_ts, end_ts, unit_ts;

	int *bf;
	BOBHash32 *hash;

	Node(int size, int num_hash) : size(size), num_hash(num_hash)
	{
		bf = new int[size]; 
		memset(bf, 0, size * sizeof(int));
		l = r = NULL;
		hash = new BOBHash32[num_hash];
		for (int i = 0; i < num_hash; ++i)
			hash[i].initialize(rand() % 1000 + 1);
	}

	~Node()
	{
		delete [] bf;
		delete [] hash;
		if (l)
			delete l;
		if (r)
			delete r;
	}
};

class TreeBF
{
private:
	int row_size, beta, num_hash;
	double max_ts;

	Node *root;

	Node* recursive_init(int dep, int sz, double l, double r);
	void recursive_insert(int dep, int v, double t, Node* p, double l, double r);
	void recursive_query(int dep, int v, Node* p, double l, double r, vector<PDD>& ans);

public:
	TreeBF(int size, int beta, double max_ts, int num_hash)
	: row_size(size), beta(beta), max_ts(max_ts), num_hash(num_hash) {}
	~TreeBF();
	void init();
	void insert(int v, double t);
	vector<PDD> query(int v);
};

TreeBF::~TreeBF()
{
	delete root;
}

Node*
TreeBF::recursive_init(int dep, int sz, double l, double r)
{
	// autotuning of beta
	if (sz <= 1024)
	{
		beta = dep;
	}

	Node *t = new Node(sz, num_hash);
	t->begin_ts = l;
	t->end_ts = r;
	t->unit_ts = (r-l)/sz;

	if (dep < beta)
	{
		double mid = (r+l)/2;
		t->l = recursive_init(dep+1, sz/2, l, mid);
		t->r = recursive_init(dep+1, sz/2, mid, r);
	}
	return t;
}

void
TreeBF::init()
{
	root = recursive_init(1, row_size, 0, max_ts);
}

void
TreeBF::insert(int v, double t)
{
	if (t > max_ts + EPS)
	{
		printf("ERROR: timestamp is too big.\n");
		exit(0);
	}

	recursive_insert(1, v, t, root, 0, max_ts);
}

void
TreeBF::recursive_insert(int dep, int v, double t, Node* p, double l, double r)
{
	// printf("%d %d %d %d %d\n", dep, hsh, t, l, r);

	for (int i = 0; i < num_hash; ++i)
	{
		int pos = (uint32_t)p->hash[i].run((char*)&v, sizeof(int)) % p->size;
		p->bf[pos] = true;
	}

	if (dep < beta)
	{
		double mid = (r+l)/2;
		if (t < mid)
			recursive_insert(dep+1, v, t, p->l, l, mid);
		else
			recursive_insert(dep+1, v, t, p->r, mid, r);
	}

}

vector<PDD>
TreeBF::query(int v)
{
	vector<PDD> ans;

	recursive_query(1, v, root, 0, max_ts, ans);

	return ans;
}

void
TreeBF::recursive_query(int dep, int v, Node* p, double l, double r, vector<PDD>& ans)
{
	bool flag = true;
	for (int i = 0; i < num_hash; ++i)
	{
		int pos = (uint32_t)p->hash[i].run((char*)&v, sizeof(int)) % p->size;
		flag &= p->bf[pos];
	}

	if (!flag)
		return;
	if (dep == beta)
	{
		ans.push_back(mp(l,r));
		return;
	}

	double mid = (r+l)/2;
	recursive_query(dep+1, v, p->l, l, mid, ans);
	recursive_query(dep+1, v, p->r, mid, r, ans);
}

#endif