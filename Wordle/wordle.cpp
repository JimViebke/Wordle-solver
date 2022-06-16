
#include "wordle.hpp"
#include "solver.hpp"

int main()
{
	srand(4);

	// test();

	Solver solver{
		load_wordlist("word_master_answers.txt"),
		load_wordlist("word_master_words.txt") };

	solver.run();
}
