#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <sstream>

#include "wordle.hpp"

namespace detail
{
	constexpr size_t window_width = 1500;
	constexpr size_t window_height = 900;

	constexpr size_t rows = 6;

	constexpr size_t tile_size_px = 70;
	constexpr size_t tile_padding_px = 7;

	constexpr size_t board_x =
		window_width / 2 -
		(word_length * tile_size_px + (word_length - 1) * tile_padding_px) / 2;
	constexpr size_t board_y =
		window_height / 2 -
		(rows * tile_size_px + (rows - 1) * tile_padding_px) / 2;

	const sf::Color background = sf::Color::Black;
	const sf::Color grey = { 54, 54, 54 }; // grey (ripped from ye internet)
	const sf::Color yellow = { 250, 166, 19 }; // yellow (ripped from ye internet)
	const sf::Color green = { 104, 142, 38 }; // green (ripped from ye internet)

	enum class tile_color { grey, yellow, green };
}


class Tile
{
public:
	explicit Tile() { reset(); }

	bool is_blank() const { return c == ' '; }

	void reset()
	{
		c = ' ';
		tile_color = detail::tile_color::grey;
	}

	char c;
	detail::tile_color tile_color;
};

class Guess
{
public:
	explicit Guess(const size_t length) : guess(length, Tile()) {}
	std::vector<Tile> guess;
};

class Board
{
public:
	explicit Board(const size_t guesses, const size_t length) : board(guesses, Guess(length)) {}
	std::vector<Guess> board;
};

class Solver
{
public:
	Solver(const std::vector<string_t>& set_answer_list, const std::vector<string_t>& set_word_list) :
		board{ detail::rows, detail::word_length },
		answer_list{ set_answer_list },
		word_list{ set_word_list }
	{
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;

		window = std::make_unique<sf::RenderWindow>(
			sf::VideoMode((uint32_t)detail::window_width, (uint32_t)detail::window_height),
			"Wordle Solver",
			sf::Style::Titlebar | sf::Style::Close,
			settings);

		window->setFramerateLimit(60);

		const std::string ARIAL_LOCATION = "C:/Windows/Fonts/Arial.ttf";
		if (!arial.loadFromFile(ARIAL_LOCATION))
		{
			std::cout << "Could not load " << ARIAL_LOCATION << '\n';
			abort();
		}

		overlay.setFont(arial);
		overlay.setCharacterSize(25);
		overlay.setFillColor(sf::Color::White);
		overlay.setPosition({ 100, float(detail::board_y / 2) });

		overlay_right.setFont(arial);
		overlay_right.setCharacterSize(35);
		overlay_right.setFillColor(sf::Color::White);
		overlay_right.setPosition({ float(detail::window_width - 450), float(detail::board_y / 2) });

		letter.setFont(arial);
		letter.setCharacterSize(70);
		letter.setFillColor(sf::Color::White);

		update_solutions();
	}

public:
	void on_click()
	{
		using namespace detail;

		if (mouse_x > board_x &&
			mouse_y > board_y &&
			mouse_x <= board_x + word_length * tile_size_px + (word_length - 1) * tile_padding_px &&
			mouse_y <= board_y + rows * tile_size_px + (rows - 1) * tile_padding_px)
		{
			if (mouse_tile_x > word_length - 1 || mouse_tile_y > rows - 1)
			{
				std::cout << "bad coords: " << mouse_tile_x << ", " << mouse_tile_y << std::endl;
				return;
			}

			if (board.board[mouse_tile_y].guess[mouse_tile_x].is_blank()) return;

			tile_color& tile_color = board.board[mouse_tile_y].guess[mouse_tile_x].tile_color;

			switch (tile_color)
			{
			case tile_color::grey:
				tile_color = tile_color::yellow;
				break;

			case tile_color::yellow:
				tile_color = tile_color::green;
				break;

			case tile_color::green:
				// deliberate fallthrough

			default:
				tile_color = tile_color::grey;
				break;
			}

			update_solutions();
		}
	}
	void on_backspace_pressed()
	{
		using namespace detail;

		if (board.board[0].guess[0].is_blank()) return;

		// search for the first empty tile
		for (size_t i = 0; i < rows; ++i)
		{
			for (size_t j = 0; j < word_length; ++j)
			{
				if (board.board[i].guess[j].is_blank())
				{
					// Erase the previous letter in this word, if one exists...
					if (j > 0)
					{
						board.board[i].guess[j - 1].reset();
					}
					else // ...otherwise, erase the last letter of the previous word
					{
						board.board[i - 1].guess[word_length - 1].reset();
					}

					update_solutions();
					return;
				}
			}
		}

		// Special case; the board is full. Erase the last letter.
		board.board[rows - 1].guess[word_length - 1].reset();

		update_solutions();
	}
	void on_ctrl_c()
	{
		for (Guess& guess : board.board)
			for (Tile& tile : guess.guess)
				tile.reset();
		update_solutions();
	}
	void on_letter_pressed(const sf::Keyboard::Key key)
	{
		// When the user types a letter, search for the first empty tile
		for (Guess& guess : board.board)
		{
			for (Tile& t : guess.guess)
			{
				if (t.is_blank())
				{
					t.c = key - sf::Keyboard::Key::A + (int)'A';
					update_solutions();
					return;
				}
			}
		}
	}
	void on_key_pressed(const sf::Event::KeyEvent key)
	{
		using namespace detail;

		if (key.code == sf::Keyboard::Backspace)
		{
			on_backspace_pressed();
		}
		else if (key.control && key.code == sf::Keyboard::Key::C)
		{
			on_ctrl_c();
		}
		else if (key.code >= sf::Keyboard::Key::A && key.code <= sf::Keyboard::Key::Z)
		{
			on_letter_pressed(key.code);
		}
	}

	void handle_events()
	{
		using namespace detail;

		sf::Event event;
		while (window->pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::MouseMoved:
				mouse_x = event.mouseMove.x;
				mouse_y = event.mouseMove.y;
				break;

			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left)
					on_click();
				break;

			case sf::Event::KeyPressed:
				on_key_pressed(event.key);
				break;

			case sf::Event::Closed:
				window->close();
				break;
			}
		}
	}

	void draw_box(const size_t x, const size_t y, const size_t width, const size_t height, const sf::Color color,
		bool outline = false, const sf::Color outline_color = sf::Color::Black, const float outline_thickness = -2.f)
	{
		sf::RectangleShape box{ { (float)width, (float)height } };
		box.setPosition({ (float)x, (float)y });
		box.setFillColor(color);

		if (outline)
		{
			box.setOutlineColor(outline_color);
			box.setOutlineThickness(outline_thickness);
		}

		window->draw(box);
	}

	void update_solutions()
	{
		using namespace detail;

		auto candidates = answer_list;

		for (size_t i = 0; i < rows; ++i)
		{
			for (size_t j = 0; j < word_length; ++j)
			{
				const Tile tile = board.board[i].guess[j];
				if (tile.is_blank()) break;

				switch (tile.tile_color)
				{
				case tile_color::grey:
					grey_filter(candidates, tile.c);
					break;
				case tile_color::yellow:
					yellow_filter(candidates, tile.c, j);
					break;
				case tile_color::green:
					green_filter(candidates, tile.c, j);
					break;
				default:
					std::cout << "Invalid color??" << std::endl;
					break;
				}
			}
		}

		std::stringstream solutions;

		if (candidates.size() == 0)
		{
			solutions << "(no solutions)";
		}
		else if (candidates.size() < 3)
		{
			solutions << "try: \n\n";
			solutions << "  " << candidates[0] << "\n\n";

			if (candidates.size() == 2)
			{
				solutions << "or: \n\n";
				solutions << "  " << candidates[1];
			}
		}
		else // >2 valid answers, assess them
		{
			const std::map<size_t, string_t> guesses = select_guesses(candidates, word_list);

			if (guesses.size() == 0)
			{
				// Uncommon scenario where all answers use same letters
				solutions << "try any of: \n\n";
				for (auto word : candidates)
					solutions << "  " << word << '\n';
			}
			else if (guesses.size() == 1)
			{
				auto it = guesses.rbegin();

				solutions << "try: \n\n";
				solutions << "  " << it->second << "\n\n";
			}
			else
			{
				auto it = guesses.rbegin();

				solutions << "try: \n\n";
				solutions << "  " << it->second << "\n\n";
				solutions << "or: \n\n";

				++it;
				for (size_t i = 0; i < 100 && it != guesses.rend(); ++i, ++it)
				{
					solutions << "  " << it->second << "\n";
				}
			}
		}

		overlay_right.setString(solutions.str());

		std::stringstream answers;
		answers << candidates.size() << " candidate" << (candidates.size() == 1 ? "" : "s") << ":\n\n";
		for (size_t i = 0; i < 100 && i < candidates.size(); ++i)
		{
			answers << "  " << candidates[i] << '\n';
		}

		overlay.setString(answers.str());
	}

	void tick()
	{
		/*
		This function contains things that need to happen once per tick, and are not directly related to
		event handling or rendering.
		*/

		mouse_tile_x = (mouse_x - detail::board_x) / (detail::tile_size_px + detail::tile_padding_px);
		mouse_tile_y = (mouse_y - detail::board_y) / (detail::tile_size_px + detail::tile_padding_px);
	}

	void render_game_board()
	{
		using namespace detail;

		for (size_t i = 0; i < rows; ++i)
		{
			for (size_t j = 0; j < word_length; ++j)
			{
				const Tile tile = board.board[i].guess[j];
				sf::Color tile_color = grey;

				switch (tile.tile_color)
				{
				case tile_color::grey:
					tile_color = grey;
					break;
				case tile_color::yellow:
					tile_color = yellow;
					break;
				case tile_color::green:
					tile_color = green;
					break;
				default:
					std::cout << "Invalid color??" << std::endl;
					break;
				}

				const size_t tile_x = board_x + (j * (tile_size_px + tile_padding_px));
				const size_t tile_y = board_y + (i * (tile_size_px + tile_padding_px));
				draw_box(tile_x, tile_y, tile_size_px, tile_size_px, tile_color);

				// draw the letter

				letter.setString(tile.c);

				sf::FloatRect bounds = letter.getLocalBounds();
				letter.setPosition(
					tile_x + tile_size_px / 2.f - bounds.width / 2.f - bounds.left,
					tile_y + tile_size_px / 2.f - bounds.height / 2.f - bounds.top);

				window->draw(letter);
			}
		}
	}

	void render()
	{
		window->clear(detail::background);

		render_game_board();

		window->draw(overlay);
		window->draw(overlay_right);
	}

public:
	void run()
	{
		while (window->isOpen())
		{
			handle_events();
			tick();
			render();

			window->display();
		}
	}

private:
	size_t frame_counter = 0;

	int32_t mouse_x = 0, mouse_y = 0;

	size_t mouse_tile_x = 0, mouse_tile_y = 0; // the tile under the mouse

	std::unique_ptr<sf::RenderWindow> window;
	sf::Font arial;
	sf::Text overlay;
	sf::Text overlay_right;
	sf::Text letter;

	Board board;

	const std::vector<string_t> answer_list;
	const std::vector<string_t> word_list;
};
