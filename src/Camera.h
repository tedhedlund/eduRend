
//
//  Camera.h
//
//	Basic camera class
//

#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "vec\vec.h"
#include "vec\mat.h"

using namespace linalg;

class Camera
{
public:
	// Aperture attributes
	float vfov, aspect;	
	
	// Clip planes in view space coordinates
	// Evrything outside of [zNear, zFar] is clipped away on the GPU side
	// zNear should be > 0
	// zFar should depend on the size of the scene
	// This range should be kept as tight as possibly to improve
	// numerical precision in the z-buffer
	float zNear, zFar;
	float yaw = 0.0f;
	float pitch = 0.0f;	
	
	vec3f position;	
	
	Camera(
		float vfov,
		float aspect,
		float zNear,
		float zFar) :		
		vfov(vfov), aspect(aspect), zNear(zNear), zFar(zFar)
	{
		position = {0.0f, 0.0f, 0.0f};
	}

	// Move to an absolute position
	//
	void moveTo(const vec3f& p)
	{
		position = p;
	}

	// Move relatively
	//
	void move(const vec3f& v)
	{
		
		position += v;
	}

	void moveForward(const float& v, float& dt) 
	{
		vec4f fwdView = { 0, 0, -1, 0 };
		mat4f rotationmatrix = mat4f::rotation(0, yaw, pitch);
		mat4f pos = mat4f::translation(position);
		mat4f viewtoworld = pos * rotationmatrix;
		vec4f fwdWorld = viewtoworld * fwdView;
		position += fwdWorld.xyz() * v * dt;
	}

	void moveBackward(const float& v, float& dt) 
	{
		vec4f bwdView = { 0, 0, 1, 0 };
		mat4f rotationmatrix = mat4f::rotation(0, yaw, pitch);
		mat4f pos = mat4f::translation(position);
		mat4f viewtoworld = pos * rotationmatrix;
		vec4f bwdWorld = viewtoworld * bwdView;
		position += bwdWorld.xyz() * v * dt;
	}

	void moveLeft(const float& v, float& dt) 
	{
		vec4f leftView = { 1, 0, 0, 0 };
		mat4f rotationmatrix = mat4f::rotation(0, yaw, pitch);
		mat4f pos = mat4f::translation(position);
		mat4f viewtoworld = pos * rotationmatrix;
		vec4f leftWorld = viewtoworld * leftView;
		position += leftWorld.xyz() * v * dt;
	}

	void moveRight(const float& v, float& dt) 
	{
		vec4f rightView = {-1, 0, 0, 0 };
		mat4f rotationmatrix = mat4f::rotation(0, yaw, pitch);
		mat4f pos = mat4f::translation(position);
		mat4f viewtoworld = pos * rotationmatrix;
		vec4f rightWorld = viewtoworld * rightView;
		position += rightWorld.xyz() * v * dt;
	}

	// Return World-to-View matrix for this camera
	//
	mat4f get_WorldToViewMatrix()
	{
		// Assuming a camera's position and rotation is defined by matrices T(p) and R,
		// the View-to-World transform is T(p)*R (for a first-person style camera).
		//
		// World-to-View then is the inverse of T(p)*R;
		//		inverse(T(p)*R) = inverse(R)*inverse(T(p)) = transpose(R)*T(-p)
		// Since now there is no rotation, this matrix is simply T(-p)
		
		mat4f rotationmatrix = mat4f::rotation(0, yaw, pitch);			
		rotationmatrix.transpose();

		return rotationmatrix * mat4f::translation(-position);		
	}

	// Matrix transforming from View space to Clip space
	// In a performance sensitive situation this matrix should be precomputed
	// if possible
	//
	mat4f get_ProjectionMatrix()
	{
		return mat4f::projection(vfov, aspect, zNear, zFar);
	}
};

#endif