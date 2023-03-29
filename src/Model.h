
//
//  Model.h
//
//  Carl Johan Gribel 2016, cjgribel@gmail.com
//

#pragma once
#ifndef MODEL_H
#define MODEL_H

#include "stdafx.h"
#include <vector>
#include "vec\vec.h"
#include "vec\mat.h"
#include "ShaderBuffers.h"
#include "Drawcall.h"
#include "OBJLoader.h"
#include "Texture.h"
#include <functional>

using namespace linalg;

class Model
{
protected:
	// Pointers to the current device and device context
	ID3D11Device* const			dxdevice;
	ID3D11DeviceContext* const	dxdevice_context;

	// Pointers to the class' vertex & index arrays
	ID3D11Buffer* vertex_buffer = nullptr;
	ID3D11Buffer* index_buffer = nullptr;

	Material* material;
	
public:
	
	Model(
		ID3D11Device* dxdevice, 
		ID3D11DeviceContext* dxdevice_context) 
		:	dxdevice(dxdevice),
			dxdevice_context(dxdevice_context)
	{ 		
	}

	//
	// Abstract render method: must be implemented by derived classes
	//
	virtual void Render(std::function<void(vec4f, vec4f, vec4f, float)>) const = 0;

	void SetMaterial(Material m) 
	{
		*material = m;

		
			std::cout << "Loading cube textures..." << std::endl;
			HRESULT hr;

			// Load Diffuse texture
			//	
			if (material->Kd_texture_filename.size()) {

				hr = LoadTextureFromFile(
					dxdevice,
					material->Kd_texture_filename.c_str(),
					&material->diffuse_texture);
				std::cout << "\t" << material->Kd_texture_filename
					<< (SUCCEEDED(hr) ? " - OK" : "- FAILED") << std::endl;
			}
		
	}

	void compute_TB(Vertex& v0, Vertex& v1, Vertex& v2) 
	{

		/*vec3f tangent, binormal;
		vec3f normal = v0.Normal;

		vec2f texture1 = v0.TexCoord;
		vec2f texture2 = v1.TexCoord;
		vec2f texture3 = v2.TexCoord;

		float det = (texture1.x * texture2.y) - (texture1.y * texture2.x);

		det = 1.0f / det;

		tangent.x = det * ((texture2.y * (v0.Pos.x - v1.Pos.x)) + (texture1.y * (v1.Pos.x - v2.Pos.x)));
		tangent.y = det * ((texture2.y * (v0.Pos.y - v1.Pos.y)) + (texture1.y * (v1.Pos.y - v2.Pos.y)));
		tangent.z = det * ((texture2.y * (v0.Pos.z - v1.Pos.z)) + (texture1.y * (v1.Pos.z - v2.Pos.z)));

		binormal.x = det * ((texture1.x * (v1.Pos.x - v2.Pos.x)) + (texture2.x * (v0.Pos.x - v1.Pos.x)));
		binormal.y = det * ((texture1.x * (v1.Pos.y - v2.Pos.y)) + (texture2.x * (v0.Pos.y - v1.Pos.y)));
		binormal.z = det * ((texture1.x * (v1.Pos.z - v2.Pos.z)) + (texture2.x * (v0.Pos.z - v1.Pos.z)));

		v0.Tangent = tangent;
		v1.Tangent = tangent;
		v2.Tangent = tangent;
		
		v0.Binormal = binormal;
		v1.Binormal = binormal;
		v2.Binormal = binormal;*/

		vec3f tangent, binormal;

		float vector1[3], vector2[3];
		float tuVector[2], tvVector[2];
		float den;
		float length;


		// Calculate the two vectors for this face.
		vector1[0] = v1.Pos.x - v0.Pos.x;
		vector1[1] = v1.Pos.y - v0.Pos.y;
		vector1[2] = v1.Pos.z - v0.Pos.z;

		vector2[0] = v2.Pos.x - v0.Pos.x;
		vector2[1] = v2.Pos.y - v0.Pos.y;
		vector2[2] = v2.Pos.z - v0.Pos.z;

		// Calculate the tu and tv texture space vectors.
		tuVector[0] = v1.TexCoord.x - v0.TexCoord.x;
		tvVector[0] = v1.TexCoord.y - v0.TexCoord.y;

		tuVector[1] = v2.TexCoord.x - v0.TexCoord.x;
		tvVector[1] = v2.TexCoord.y - v0.TexCoord.y;

		// Calculate the denominator of the tangent/binormal equation.
		den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

		// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
		tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
		tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
		tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

		binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
		binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
		binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

		// Calculate the length of this normal.
		length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

		// Normalize the normal and then store it
		tangent.x = tangent.x / length;
		tangent.y = tangent.y / length;
		tangent.z = tangent.z / length;

		// Calculate the length of this normal.
		length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

		// Normalize the normal and then store it
		binormal.x = binormal.x / length;
		binormal.y = binormal.y / length;
		binormal.z = binormal.z / length;

		v0.Tangent = v1.Tangent = v2.Tangent = tangent;
		v0.Binormal = v1.Binormal = v2.Binormal = binormal;

	}

	//
	// Destructor
	//
	virtual ~Model()
	{ 
		SAFE_RELEASE(vertex_buffer);
		SAFE_RELEASE(index_buffer);
	}
};

class QuadModel : public Model
{
	unsigned nbr_indices = 0;

public:

	QuadModel(
		ID3D11Device* dx3ddevice,
		ID3D11DeviceContext* dx3ddevice_context);

	virtual void Render(std::function<void(vec4f, vec4f, vec4f, float)>) const;

	~QuadModel() { }
};

class OBJModel : public Model
{
	// index ranges, representing drawcalls, within an index array
	struct IndexRange
	{
		unsigned int start;
		unsigned int size;
		unsigned ofs;
		int mtl_index;
	};

	std::vector<IndexRange> index_ranges;
	std::vector<Material> materials;

	void append_materials(const std::vector<Material>& mtl_vec)
	{
		materials.insert(materials.end(), mtl_vec.begin(), mtl_vec.end());
	}

public:

	OBJModel(
		const std::string& objfile,
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context);

	virtual void Render(std::function<void(vec4f, vec4f, vec4f, float)>) const;

	~OBJModel();
};

class Cube : public Model 
{

	unsigned nbr_indices = 0;

public:

	Cube(
		ID3D11Device* dx3ddevice,
		ID3D11DeviceContext* dx33ddevice_context
	);
	
	virtual void Render(std::function<void(vec4f, vec4f, vec4f, float)>) const;

	~Cube() {}


};

#endif