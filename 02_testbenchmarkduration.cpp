#include <iostream>
#include <string>
#include <unistd.h>

#define BENCHMARK_DURATION_SCOPE(name) BenchmarkDuration _benchmarkDurationScope(name);

class BenchmarkDuration
{
	public:
	BenchmarkDuration(const std::string & name):
		m_name(name)
	{
		printSpaces(s_depth++);
		std::cout << "start " << name << std::endl;
		m_startTime = currentTime();
	}

	virtual ~BenchmarkDuration()
      	{
		double endTime = currentTime();
		double duration = endTime - m_startTime;
		printSpaces(--s_depth);
		std::cout << "end " << m_name << " duration = "<< duration << " seconds" <<std::endl;
      	}


	private:
	static double currentTime()
	{
  		struct timespec tp = {0, 0};
  		clock_gettime(CLOCK_REALTIME, &tp);
  		double _clock = tp.tv_sec + tp.tv_nsec * 0.001 * 0.001 * 0.001;
  		return _clock;
	};

	static void printSpaces(int n)
	{
		for (int i = 0; i < n; i++)
		{
			std::cout << " ";
		}
	}

	private:
	std::string m_name;
	double m_startTime;
	static int s_depth;
};

int BenchmarkDuration::s_depth = 0;

int main(int argc, char ** argv)
{
	BENCHMARK_DURATION_SCOPE("main()");

	{
		BENCHMARK_DURATION_SCOPE("sleep(4)");
		sleep(4);
	}

	{
		BENCHMARK_DURATION_SCOPE("sleep(3)");
		sleep(3);
	}

	return 0;
}

/* output : 
start main()
 start sleep(4)
 end sleep(4) duration = 4.00015 seconds
 start sleep(3)
 end sleep(3) duration = 3.00014 seconds
end main() duration = 7.00052 seconds
*/
