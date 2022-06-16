#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include "boost_static_string.hpp"

#include "utility.hpp"

namespace detail
{
	constexpr size_t word_length = 5;

	constexpr char letter_a = 'A'; // These are here because some optimizations are case-sensitive.
	constexpr char letter_z = 'Z';
}

using string_t = boost::static_string<detail::word_length>;

std::vector<string_t> load_norvig_words(const size_t word_list_size)
{
	std::fstream word_file("../Wordle/norvig_count_1w.txt");

	std::string word;
	size_t count;

	std::vector<string_t> word_list;
	word_list.reserve(word_list_size);

	while (word_file >> word >> count && word_list.size() < word_list_size)
	{
		if (word.size() != detail::word_length) continue;

		transform(word.begin(), word.end(), word.begin(), ::toupper);

		word_list.push_back(word.c_str());
	}

	return word_list;
}

std::map<string_t, size_t> load_norvig_words_with_frequencies()
{
	std::fstream word_file("../Wordle/norvig_count_1w.txt");

	std::string word;
	size_t count;

	std::map<string_t, size_t> word_list;

	while (word_file >> word >> count)
	{
		if (word.size() != detail::word_length) continue;

		transform(word.begin(), word.end(), word.begin(), ::toupper);

		if (contains(word, 'A') ||
			contains(word, 'E') ||
			contains(word, 'I') ||
			contains(word, 'O') ||
			contains(word, 'U') ||
			contains(word, 'Y'))
		{
			word_list.insert({ string_t(word), count });
		}
	}

	return word_list;
}

std::vector<string_t> load_wordle_nyt_words()
{
	assert(detail::word_length == 5);

	std::fstream word_file("../Wordle/wordle_nyt.txt");

	std::string word;
	std::vector<string_t> word_list;
	while (word_file >> word)
	{
		if (word.size() != detail::word_length) continue;
		transform(word.begin(), word.end(), word.begin(), ::toupper);
		word_list.push_back(string_t(word));
	}

	return word_list;
}

std::vector<string_t> load_wordlist(const std::string& filename)
{
	assert(detail::word_length == 5);

	std::fstream word_file("../Wordle/" + filename);

	std::string word;
	std::vector<string_t> word_list;
	while (word_file >> word)
	{
		if (word.size() != detail::word_length) continue;
		transform(word.begin(), word.end(), word.begin(), ::toupper);
		word_list.push_back(string_t(word));
	}

	return word_list;
}

/*
1. If we find a correct letter, remove:
	- every candidate that does not have that letter in that position
*/
void green_filter(std::vector<string_t>& candidates, const char c, const size_t position)
{
	candidates.erase(std::remove_if(candidates.begin(),
		candidates.end(),
		[c, position](string_t x) { return x[position] != c; }),
		candidates.end());
}

/*
2. If we find a letter that is used, but in a different position, remove:
	- every candidate that uses that letter in that position, and
	- every candidate that does not use that letter
*/
void yellow_filter(std::vector<string_t>& candidates, const char c, const size_t position)
{
	candidates.erase(std::remove_if(candidates.begin(),
		candidates.end(),
		[c, position](string_t x) { return !contains(x, c) || x[position] == c; }),
		candidates.end());
}

/*
3. If we find a letter that is not used, remove:
	- every candidate that contains that letter
*/
void grey_filter(std::vector<string_t>& candidates, const char c)
{
	candidates.erase(std::remove_if(candidates.begin(),
		candidates.end(),
		[c](string_t x) { return contains(x, c); }),
		candidates.end());
}

string_t select_guess(const std::vector<string_t>& candidates, const std::vector<string_t>& dictionary)
{
	std::vector<size_t> letter_weights(detail::letter_z + 1, 0);

	// count how many words contain each letter (each letter only counted once per word)
	for (const auto& word : candidates)
	{
		for (auto i = 0; i < word.size(); ++i)
		{
			size_t score = 1;

			// check letter against all previous letters
			for (auto j = 0; j < i; ++j)
				if (word[i] == word[j])
					score = 0;

			// add 1 or 0 to the score
			letter_weights[word[i]] += score;
		}
	}

	// the score for containing a letter is the smaller of:
	// - the number of words containing the letter (what we just calculated)
	// - the number of words not containing the letter (ie, size - count)
	for (auto& w : letter_weights)
		w = std::min(w, candidates.size() - w);

	// find the word in the dictionary with the best score
	size_t best_weight = 0;
	string_t best_word = candidates[0]; // worst-case scenario, at least we pick a word from the list of candidates
	for (const auto& word : dictionary)
	{
		bool letters[26]{}; // array of false

		// score the word
		size_t weight = 0;
		for (auto i = 0; i < word.size(); ++i)
		{
			// skip if this letter has been scored
			if (letters[word[i] - detail::letter_a]) continue;

			// note that we have scored this letter
			letters[word[i] - detail::letter_a] = true;

			weight += letter_weights[word[i]];
		}

		// keep this word if it has the best score so far
		if (weight > best_weight)
		{
			best_weight = weight;
			best_word = word;
		}
	}

	return best_word;
}

std::map<size_t, string_t> select_guesses(const std::vector<string_t>& candidates, const std::vector<string_t>& wordlist)
{
	std::vector<size_t> letter_weights(detail::letter_z + 1, 0);

	// count how many words contain each letter (each letter only counted once per word)
	for (const auto& word : candidates)
	{
		for (auto i = 0; i < word.size(); ++i)
		{
			size_t score = 1;

			// check letter against all previous letters
			for (auto j = 0; j < i; ++j)
				if (word[i] == word[j])
					score = 0;

			// add 1 or 0 to the score
			letter_weights[word[i]] += score;
		}
	}

	// the score for containing a letter is the smaller of:
	// - the number of words containing the letter (what we just calculated)
	// - the number of words not containing the letter (ie, size - count)
	for (auto& w : letter_weights)
		w = std::min(w, candidates.size() - w);

	// find the word in the dictionary with the best score
	size_t best_weight = 0;
	string_t best_word = candidates[0]; // worst-case scenario, at least we pick a word from the list of candidates

	std::map<size_t, string_t> solutions;

	for (const auto& word : wordlist)
	{
		bool letters[26]{}; // array of false

		// score the word
		size_t weight = 0;

		for (auto i = 0; i < word.size(); ++i)
		{
			// skip if this letter has been scored
			if (letters[word[i] - detail::letter_a]) continue;

			// note that we have scored this letter
			letters[word[i] - detail::letter_a] = true;

			weight += letter_weights[word[i]];
		}

		if (weight > 0)
		{
			solutions.insert({ weight, word });
		}
	}

	return solutions;
}

void play(const std::vector<string_t>& dictionary, const string_t& answer, const string_t& first_guess = "")
{
	std::vector<string_t> candidates = dictionary; // mutable copy for thinkin'

	// std::cout << "\n\n" << answer << "\n\t\tGreen:\tYellow:\tGrey:\n";

	for (auto guess_n = 0; guess_n < 6; ++guess_n)
	{
		const string_t guess = (guess_n == 0 && !first_guess.empty()) ?
			first_guess :
			select_guess(candidates, dictionary);

		// std::cout << "Guess " << (guess_n + 1) << ": " << guess << '\t';

		if (guess == answer)
		{
			// std::cout << "Correct!" << std::endl;
			return;
		}

		// check for matching letters
		for (auto i = 0; i < guess.size(); ++i)
		{
			if (guess[i] == answer[i])
			{
				// std::cout << guess[i];

				green_filter(candidates, guess[i], i);
			}
			else
			{
				// std::cout << '.';
			}
		}

		// std::cout << '\t';

		// check for right letter, wrong place
		for (auto i = 0; i < guess.size(); ++i)
		{
			if (guess[i] != answer[i] && contains(answer, guess[i]))
			{
				// std::cout << guess[i];

				yellow_filter(candidates, guess[i], i);
			}
			else
			{
				// std::cout << '.';
			}
		}

		// check for unused letter
		for (auto i = 0; i < guess.size(); ++i)
		{
			if (!contains(answer, guess[i]))
			{
				// std::cout << guess[i];

				grey_filter(candidates, guess[i]);
			}
			else
			{
				// std::cout << '.';
			}
		}
	}

	std::cout << "Lost on " << answer << ", " << candidates.size() << " candidates left.\n";
}

void test()
{
	const std::vector<string_t> dictionary = load_wordle_nyt_words();

	std::cout << "Loaded " << dictionary.size() << " words." << std::endl;

	const auto start_time = current_time_in_us();

	// The first guess of every round is deterministic. Calculate it once, here.
	const string_t first_guess = select_guess(dictionary, dictionary);

	for (auto i = 0; i < dictionary.size(); ++i)
	{
		play(dictionary, dictionary[i], first_guess);
	}

	const auto elapsed_time = current_time_in_us() - start_time;

	std::cout << "\nPlayed " << dictionary.size() << " games, " << elapsed_time / 1'000 << " ms elapsed (" << (elapsed_time / dictionary.size()) << " us per game)\n";
}
