#pragma once
#include<string>
#include "Model.h"
#include "SkyboxTexture.h"
class Skybox
{
public:
	Model model;
	SkyboxTexture texture;

	void load(std::string,std::string);
};