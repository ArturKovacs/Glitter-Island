#include "ModelSelectionContext.hpp"

#include "DemoCore.hpp"
#include "Utility.hpp"
#include "ContextManager.hpp"


ModelSelectionContext::ModelSelectionContext(ContextManager* pContextManager, DemoCore* pCore) :
GUIContext(pContextManager, pCore),
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

		case sf::Keyboard::R:
		case sf::Keyboard::Up:
			howeredModelID--;
			howeredModelID = howeredModelID < 0 ? modelFileList.size()-1 : howeredModelID;
			break;

		case sf::Keyboard::F:
		case sf::Keyboard::Down:
			howeredModelID++;
			howeredModelID = howeredModelID > modelFileList.size()-1 ? 0 : howeredModelID;
			break;

		case sf::Keyboard::Space:
			selectedModelID = howeredModelID;

			//Chech if we are on top, just for safety
			if (pContextManager->GetTopContext() == this) {
				pContextManager->PopContext();
			}
			break;

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
{}

void ModelSelectionContext::DrawOverlayElements()
{
	const int columnElementCount = 15;
	gl::Context& glContext = pCore->GetGraphicsEngine().GetGLContext();

	pCore->GetGraphicsEngine().GetSimpleColoredDrawer().Draw(glContext, screenRectangle, glm::mat4(), glm::vec4(0, 0, 0, 0.75));

	if (modelFileList.size() > 0) {
		std::string visibleList;

		const int listFirstElementID = howeredModelID - howeredModelID % columnElementCount;

		for (int i = 0; i < columnElementCount && (listFirstElementID + i < modelFileList.size()); i++) {
			visibleList += modelFileList.at(listFirstElementID + i) + '\n';
		}
		visibleList.pop_back(); // Remove last line feed

		sf::Text text(visibleList, pCore->overlayFont);
		text.setCharacterSize(16);
		float textWidth = TextDrawer::GetTextWidth(text);
		text.setColor(sf::Color::Black);
		text.setOrigin(std::floor(textWidth*0.5), 0);
		text.setPosition(std::floor(pCore->GetScreenWidth()*0.5), 50);
		pCore->textDrawer.DrawBackground(glContext, text, sf::Color(230, 230, 230, 255), 5);
		pCore->textDrawer.DrawAsList(glContext, text, howeredModelID - listFirstElementID, sf::Color(40, 180, 40, 255));
	}
}

std::string ModelSelectionContext::GetSelectedModelFilename() const
{
	return modelFileList.at(selectedModelID);
}

int ModelSelectionContext::GetModelFileCount()
{
	return modelFileList.size();
}

void ModelSelectionContext::ForceUpdateModelFileList()
{
	UpdateModelFileList();
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
