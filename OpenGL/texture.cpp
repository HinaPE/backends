#include "glad/glad.h"
#include "../texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <iostream>
#include <utility>

Kasumi::Texture::Texture(const std::string &path) : _path(path)
{
	_width = _height = _nr_channels = 0;
	_data = stbi_load(path.c_str(), &_width, &_height, &_nr_channels, 0);

	update();
}

Kasumi::Texture::Texture(unsigned char *data, int width, int height, int channels) : _data(data), _width(width), _height(height), _nr_channels(channels) { update(); }

Kasumi::Texture::~Texture()
{
	glDeleteTextures(1, &ID);
	std::cout << "delete texture: " << _path << std::endl;
}

void Kasumi::Texture::bind(int texture_idx) const
{
	switch (texture_idx)
	{
		case 0:
			glActiveTexture(GL_TEXTURE0);
			break;
		case 1:
			glActiveTexture(GL_TEXTURE1);
			break;
		case 2:
			glActiveTexture(GL_TEXTURE2);
			break;
		case 3:
			glActiveTexture(GL_TEXTURE3);
			break;
		case 4:
			glActiveTexture(GL_TEXTURE4);
			break;
		case 5:
			glActiveTexture(GL_TEXTURE5);
			break;
		default: // TOO MUCH TEXTURES
			break;
	}
	glBindTexture(GL_TEXTURE_2D, ID);
}
void Kasumi::Texture::update()
{
	if (!_data)
		return;

	auto format = GL_RED;
	if (_nr_channels == 1)
		format = GL_RED;
	else if (_nr_channels == 3)
		format = GL_RGB;
	else if (_nr_channels == 4)
		format = GL_RGBA;

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, format, GL_UNSIGNED_BYTE, _data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}
auto Kasumi::Texture::get(int x, int y) const -> mVector4
{
	// get pixel color from texture data
	mVector4 color;

	auto red = (int) _data[(_width * y + x) * _nr_channels];
	auto green = (int) _data[(_width * y + x) * _nr_channels + 1];
	auto blue = (int) _data[(_width * y + x) * _nr_channels + 2];
	auto alpha = (int) _data[(_width * y + x) * _nr_channels + 3];

	color = {red, green, blue, alpha};

	return color;
}
void Kasumi::Texture::set(int x, int y, const mVector4 &pixel)
{
	// set pixel color to texture data
	_data[(_width * y + x) * _nr_channels] = (unsigned char) pixel.x();
	_data[(_width * y + x) * _nr_channels + 1] = (unsigned char) pixel.y();
	_data[(_width * y + x) * _nr_channels + 2] = (unsigned char) pixel.z();
	_data[(_width * y + x) * _nr_channels + 3] = (unsigned char) pixel.w();
}
