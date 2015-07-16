
#include "DemoCore.hpp"

int main()
{
	try {
		sf::Window window(sf::VideoMode(800, 600), "Island", sf::Style::Default, sf::ContextSettings(24));

		if (glewInit() != GLEW_OK) {
			throw std::exception("Can not initialize GLEW");
		}

		DemoCore demo(&window);
		return demo.Start();
	}
	catch (gl::Error& err) {
		std::cout << std::endl << "Exception: " << std::endl;
		std::cout << err.what() << std::endl;
		std::cout << "File: " << (err.SourceFile() == nullptr ? "" : err.SourceFile()) << std::endl;
		std::cout << "Line: " << err.SourceLine() << std::endl;
		std::cout << "Log:" << std::endl;
		std::cout << err.Log() << std::endl;
		system("PAUSE");
	}
	catch (std::exception& ex) {
		std::cout << std::endl << "Exception: " << std::endl;
		std::cout << ex.what() << std::endl;
		system("PAUSE");
	}
	catch (...) {
		std::cout << "Uknown exception occured!" << std::endl;
		system("PAUSE");
	}

	return EXIT_FAILURE;
}
