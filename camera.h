#ifndef BACKENDS_CAMERA_H
#define BACKENDS_CAMERA_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"

namespace Kasumi
{
class Camera final : public HinaPE::CopyDisable
{
public:
	static void Init();
	static std::shared_ptr<Camera> MainCamera;

	// static
	static auto project_matrix(real fov, real aspect_ratio, real near, real far) -> mMatrix4x4;
	static auto view_matrix(const mVector3 &position, const mQuaternion &rotation) -> mMatrix4x4;

	auto get_projection() const -> mMatrix4x4;
	auto get_view() const -> mMatrix4x4;
	auto screen_to_world(const mVector2 &screen_pos) const -> mVector3; // NOT COMPLETE YET
	auto get_ray(const mVector2 &screen_pos) const -> mRay3; // NOT COMPLETE YET

	// callbacks
	void key(int key, int scancode, int action, int mods);
	void mouse_button(int button, int action, int mods);
	void mouse_scroll(double x_offset, double y_offset);
	void mouse_cursor(double x_pos, double y_pos);

public:
	struct Opt
	{
		// for view
		mVector3 position; // auto calculated: _opt.position = _opt.look_at + _opt.rotation * mVector3(0, 0, _opt.radius);
		mQuaternion rotation = mQuaternion{1, 0, 0, 0};
		mVector3 look_at = mVector3(0, 0, 0);
		real radius = 5;

		// for projection
		real width = 1024;
		real height = 768;
		real vertical_fov = 45;
		real aspect_ratio = 1024.f / 768.f;
		real near_plane = 0.01;
		real far_plane = 100;

		// mouse control
		real orbit_sens = 0.01;
		real move_sens = 0.015;
		real radius_sens = 0.25;

		// lens parameters
		real aperture = 0; // not used yet
		real focal_dist = 1; // not used yet
		bool orbit_flip_vertical = false;

		// 2D mode
		bool is2D = false;
	} _opt;
	Camera();
	void _rebuild_();

public:
	auto _up() const -> mVector3;
	auto _front() const -> mVector3;
	auto _right() const -> mVector3;
	auto _distance() const -> real;
	void _loot_at(const mVector3 &focus_point);

private:
	mMatrix4x4 _projection, _view;
};
using CameraPtr = std::shared_ptr<Camera>;
} // namespace Kasumi

#endif //BACKENDS_CAMERA_H
