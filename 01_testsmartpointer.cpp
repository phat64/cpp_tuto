#include <iostream>
#include <string>
#include <vector>
#include <memory> // unique_ptr, shared_ptr

using namespace std;

class A
{
public:
	A(int id): m_id(id)
	{
		cout << "A::ctor" << m_id <<endl;
	}

	virtual ~A()
	{
		cout << "A::dtor" << m_id <<endl;
		m_id = -1;
	}

	void print()
	{
		cout << "id == " << m_id <<endl;		
	}

	int m_id = -1;
};

void test_unique()
{
	std::unique_ptr<A> p0(new A(0));	
	p0->print();
}

void test_shared()
{
	shared_ptr<A> a(new A(1));
	shared_ptr<A> b(new A(2));

	auto c = a;
	auto d = b;
	auto e = d;

	a->print();
	b->print();
	c->print();
	d->print();
	e->print();

	// shared_ptr<A> z; // crash cuz z is empty (contains nullptr)
	// z->print();
}

void test_shared_vector()
{
	shared_ptr<A> a(new A(99));
	vector<shared_ptr<A>> v;
	const int N = 5;

	for (int i = 0; i < N; i++)
	{
		v.push_back(a);
	}
	cout << "use_count == " << a.use_count() <<endl; // use_count == 5 + 1

	for (int i = 0; i < N; i++)
	{
		v[i]->print();
	}
}

int main(int argc, char ** argv)
{
	cout << "--- test_unique : start ---" << endl;
	{
		test_unique();
	}
	cout << "--- test_unique : end ---" << endl;

	cout << "--- test_shared : start ---" << endl;
	{
		test_shared();
	}
	cout << "--- test_shared : end ---" << endl;

	cout << "--- test_shared_vector : start ---" << endl;
	{
		test_shared_vector();
	}
	cout << "--- test_shared_vector : end ---" << endl;

	return 0;
}

/* output : 
--- test_unique : start ---
A::ctor0
id == 0
A::dtor0
--- test_unique : end ---
--- test_shared : start ---
A::ctor1
A::ctor2
id == 1
id == 2
id == 1
id == 2
id == 2
A::dtor2
A::dtor1
--- test_shared : end ---
--- test_shared_vector : start ---
A::ctor99
use_count == 6
id == 99
id == 99
id == 99
id == 99
id == 99
A::dtor99
--- test_shared_vector : end ---
*/
