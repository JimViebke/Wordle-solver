
#include "solver.hpp"
#include "wordle.hpp"

int main()
{
	srand(4);

	// test();

	Solver solver{
		load_wordlist("wordle_nyt.txt"),
		load_wordlist("wordle_nyt.txt") };

	solver.run();
}
