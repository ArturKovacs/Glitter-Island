#include <iostream>
#include <string>

#include "DemoCore.hpp"

/*
static void CheckGLError()
{
	GLenum e = glGetError();
	switch (e)
	{
	case GL_NO_ERROR:
		break;

	case GL_INVALID_ENUM:
		throw std::exception("GL_INVALID_ENUM");
		break;
	case GL_INVALID_VALUE:
		throw std::exception("GL_INVALID_VALUE");
		break;
	case GL_INVALID_OPERATION:
		throw std::exception("GL_INVALID_OPERATION");
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		throw std::exception("GL_INVALID_FRAMEBUFFER_OPERATION");
		break;
	case GL_OUT_OF_MEMORY:
		throw std::exception("GL_OUT_OF_MEMORY");
		break;
	case GL_STACK_UNDERFLOW:
		throw std::exception("GL_STACK_UNDERFLOW");
		break;
	case GL_STACK_OVERFLOW:
		throw std::exception("GL_STACK_OVERFLOW");
		break;

	default:
		throw std::exception((std::string("Noooooooo... ") + std::to_string(e)).c_str());
		break;
	}
}*/

int main()
{
	try {
		sf::Window window(sf::VideoMode(800, 600), "Glitter-Island", sf::Style::Default, sf::ContextSettings(24));

		///////////////////////////////////////////////
		//These functions are called intentionally!
		//This is a hack to solve unexpected context change while the application is running.
		//The context change causes vertex arrays to become invalid.
		//see: http://www.sfml-dev.org/documentation/2.2/classsf_1_1Texture.php#a0bf905d487b104b758549c2e9e20a3fb
		//and: http://www.sfml-dev.org/documentation/2.2/classsf_1_1Shader.php#ad22474690bafe4a305c1b9826b1bd86a
		sf::Texture::getMaximumSize();
		sf::Shader::isAvailable();

		window.setActive(true);
		///////////////////////////////////////////////


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
