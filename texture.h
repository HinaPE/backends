#ifndef BACKENDS_TEXTURE_H
#define BACKENDS_TEXTURE_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"

#include <string>
#include <memory>

namespace Kasumi
{
class Texture
{
public:
	explicit Texture(const std::string &path);
	Texture(unsigned char *data, int width, int height, int channels);
	~Texture();

	void bind(int texture_idx = 0) const;
	void update();
	auto get(int x, int y) const -> mVector4;
	void set(int x, int y, const mVector4& pixel);

public:
	unsigned int ID = 0;
	std::string _path;
	int _width, _height, _nr_channels;
	unsigned char *_data;
};
using TexturePtr = std::shared_ptr<Texture>;
} // namespace Kasumi

#endif //BACKENDS_TEXTURE_H
