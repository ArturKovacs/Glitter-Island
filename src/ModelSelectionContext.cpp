#include "ModelSelectionContext.hpp"

#include "DemoCore.hpp"
#include "Utility.hpp"
#include "ContextManager.hpp"


ModelSelectionContext::ModelSelectionContext(ContextManager* pContextManager, DemoCore* pDemoCore) :
GUIContext(pContextManager, pDemoCore),
howeredModelID(0), selectedModelID(0)
{
	requireFocus = true;

	screenRectangle = Mesh::GenerateRectangle(2, 2);
}

ModelSelectionContext::~ModelSelectionContext()
{
}

void ModelSelectionContext::HandleWindowEvent(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed) {
		switch (event.key.code) {
		case sf::Keyboard::Up:
			howeredModelID--;
			howeredModelID = std::max(howeredModelID, 0);
			break;

		case sf::Keyboard::Down:
			howeredModelID++;
			howeredModelID = std::min<int>(howeredModelID, modelFileList.size()-1);
			break;

		case sf::Keyboard::Return:
			selectedModelID = howeredModelID;

			//Chech if we are on top, just for safety
			if (pContextManager->GetTopContext() == this) {
				pContextManager->PopContext();
			}
			break;

		case sf::Keyboard::Num6:
		case sf::Keyboard::Escape:
			if (pContextManager->GetTopContext() == this) {
				pContextManager->PopContext();
			}
			break;

		default:
			break;
		}
	}
}

void ModelSelectionContext::EnteringContext() 
{
	UpdateModelFileList();
	if (selectedModelID >= modelFileList.size()) {
		selectedModelID = 0;
	}
	
	howeredModelID = selectedModelID;
}

void ModelSelectionContext::LeavingContext()
{
}

void ModelSelectionContext::Update(float deltaSec) 
{
}

void ModelSelectionContext::Draw() 
{
}

void ModelSelectionContext::DrawOverlayElements()
{
	const int columnElementCount = 15;
	gl::Context& glContext = pDemoCore->GetGLContext();

	pDemoCore->simpleColoredDrawer.Draw(glContext, screenRectangle, gl::Mat4f(), gl::Vec4f(0, 0, 0, 0.7));

	if (modelFileList.size() > 0) {
		std::string visibleList;

		const int listFirstElementID = howeredModelID - howeredModelID % columnElementCount;

		for (int i = 0; i < columnElementCount && (listFirstElementID + i < modelFileList.size()); i++) {
			visibleList += modelFileList.at(listFirstElementID + i) + '\n';
		}
		visibleList.pop_back(); // Remove last line feed

		sf::Text text(visibleList, pDemoCore->overlayFont);
		text.setCharacterSize(16);

		text.setPosition(300, 50);
		pDemoCore->textDrawer.DrawBackground(glContext, text, sf::Color(100, 100, 100, 255), 5);
		pDemoCore->textDrawer.DrawAsList(glContext, text, howeredModelID - listFirstElementID, sf::Color::Magenta);
	}
}

void ModelSelectionContext::UpdateModelFileList()
{
	// Find all model files in models directory
	auto tmpList = util::GetFileNamesInDirectory("../models");

	modelFileList.clear();

	// only keep supported model files!
	for (int i = 0; i < tmpList.size(); i++) {
		const auto& current = tmpList[i];
		const std::string extention = ".obj";
		const bool correctExtension = current.rfind(extention) == current.length() - extention.length();
		if (correctExtension) {
			modelFileList.push_back(current);
		}
	}
}
