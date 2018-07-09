#include <iostream>
#include <functional>

class TestExitCallback
{
public:
	TestExitCallback(){}

	virtual ~TestExitCallback()
	{
		if (m_callback)
		{
			m_callback(nullptr);
		}
	}

	std::function<void(void*)> m_callback;

} testExitCallback;

void print_exit(void*)
{
	std::cout << "print_exit : callback used" << std::endl;
}

int main(int argc, char ** argv)
{
	std::cout << "--- set exit callback ---" << std::endl;
	testExitCallback.m_callback = print_exit;
	std::cout << "--- exit ---" << std::endl;
	return 0;
}

/* output :
--- set exit callback ---
--- exit ---
print_exit : callback used 
*/
