#include "cup.h"
Kasumi::CupObject::CupObject()
{
	NAME = "Cup";
	_shader = Shader::DefaultMeshShader;
	_init("cup", "");
}
