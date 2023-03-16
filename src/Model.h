
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

		vec3f tangent, binormal;
	
		vec3f d = v1.Pos - v0.Pos;
		vec3f e = v2.Pos - v0.Pos;

		vec2f f = v1.TexCoord - v0.TexCoord;
		vec2f g = v2.TexCoord - v0.TexCoord;

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