#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class Terminal {
	private:
		std::string current_dir;
		std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commands;

		void init_commands() {
			commands = {
				{
					"ls", [this](auto & args) {
						list_directory(args);
					}
				},
				{
					"cd", [this](auto & args) {
						change_directory(args);
					}
				},
				{
					"pwd", [](auto &) {
						std::cout << fs::current_path() << '\n';
					}
				},
				{
					"mkdir", [](auto & args) {
						make_directory(args);
					}
				},
				{
					"rm", [](auto & args) {
						remove_target(args);
					}
				},
				{
					"cp", [](auto & args) {
						copy_file(args);
					}
				},
				{
					"mv", [](auto & args) {
						move_file(args);
					}
				},
				{
					"touch", [](auto & args) {
						create_file(args);
					}
				},
				{
					"cat", [](auto & args) {
						display_file(args);
					}
				},
				{
					"echo", [this](auto & args) {
						echo_content(args);
					}
				},
				{
					"clear", [](auto &) {
						system("cls||clear");
					}
				},
				{
					"help", [this](auto &) {
						show_help();
					}
				}
			};
		}

		void list_directory(const std::vector<std::string> &args) {
			bool show_hidden = std::find(args.begin(), args.end(), "-a") != args.end();
			for (const auto &entry : fs::directory_iterator(current_dir)) {
				std::string filename = entry.path().filename().string();
				if (!show_hidden && filename[0] == '.')
					continue;
				std::cout << (entry.is_directory() ? "[DIR] " : "[FILE] ") << filename << '\n';
			}
		}

		void change_directory(const std::vector<std::string> &args) {
			if (args.empty())
				return;
			try {
				fs::current_path(args[0]);
				current_dir = fs::current_path().string();
			} catch (...) {
				std::cerr << "Error: Invalid directory\n";
			}
		}

		static void make_directory(const std::vector<std::string> &args) {
			if (args.empty())
				return;
			fs::create_directories(args[0]);
		}

		static void remove_target(const std::vector<std::string> &args) {
			if (args.empty())
				return;
			bool recursive = std::find(args.begin(), args.end(), "-r") != args.end();
			for (const auto &path : args) {
				if (path == "-r")
					continue;
				if (recursive)
					fs::remove_all(path);
				else
					fs::remove(path);
			}
		}

		static void copy_file(const std::vector<std::string> &args) {
			if (args.size() < 2)
				return;
			fs::copy(args[0], args[1], fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		}

		static void move_file(const std::vector<std::string> &args) {
			if (args.size() < 2)
				return;
			fs::rename(args[0], args[1]);
		}

		static void create_file(const std::vector<std::string> &args) {
			if (args.empty())
				return;
			std::ofstream(args[0]).close();
		}

		static void display_file(const std::vector<std::string> &args) {
			if (args.empty())
				return;
			std::ifstream file(args[0]);
			std::cout << file.rdbuf() << '\n';
		}

		void echo_content(const std::vector<std::string> &args) {
			if (args.empty())
				return;
			auto it = std::find(args.begin(), args.end(), ">");
			if (it != args.end() && ++it != args.end()) {
				std::ofstream out(*it);
				for (auto cit = args.begin(); cit != it - 1; ++cit)
					out << *cit << ' ';
			} else {
				for (const auto &arg : args)
					std::cout << arg << ' ';
				std::cout << '\n';
			}
		}

		void show_help() {
			std::cout << "Supported commands:\n"
			          << "ls [-a]        List directory contents\n"
			          << "cd <path>      Change directory\n"
			          << "pwd            Print working directory\n"
			          << "mkdir <dir>    Create directory\n"
			          << "rm [-r] <path> Remove file/directory\n"
			          << "cp <src> <dst> Copy file/directory\n"
			          << "mv <src> <dst> Move/rename file\n"
			          << "touch <file>   Create empty file\n"
			          << "cat <file>     Display file content\n"
			          << "echo [> file]  Print/write text\n"
			          << "clear          Clear screen\n"
			          << "help           Show this help\n"
			          << "exit           Quit terminal\n";
		}

	public:
		Terminal() : current_dir(fs::current_path().string()) {
			init_commands();
		}

		void run() {
			std::string input;
			while (true) {
				std::cout << "\033[32m" << current_dir << " $ \033[0m";
				std::getline(std::cin, input);
				if (input == "exit")
					break;

				std::vector<std::string> args;
				std::istringstream iss(input);
				std::string token;
				while (iss >> token)
					args.push_back(token);

				if (args.empty())
					continue;

				try {
					if (commands.find(args[0]) != commands.end()) {
						std::vector<std::string> cmd_args(args.begin() + 1, args.end());
						commands[args[0]](cmd_args);
					} else {
						std::cerr << "Command not found: " << args[0] << '\n';
					}
				} catch (const std::exception &e) {
					std::cerr << "Error: " << e.what() << '\n';
				}
			}
		}
};

int main() {
	Terminal term;
	term.run();
	return 0;
}