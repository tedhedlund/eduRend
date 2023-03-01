
#include "Scene.h"
#include <cmath>
#include <chrono>

vec4f light = vec4f{ 0, 1000, 0, 0 };

Scene::Scene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	dxdevice(dxdevice),
	dxdevice_context(dxdevice_context),
	window_width(window_width),
	window_height(window_height)
{ }

void Scene::WindowResize(
	int window_width,
	int window_height)
{
	this->window_width = window_width;
	this->window_height = window_height;
}

OurTestScene::OurTestScene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	Scene(dxdevice, dxdevice_context, window_width, window_height)
{ 
	InitTransformationBuffer();
	// + init other CBuffers
	InitLightAndCameraBuffer();
	InitMaterialBuffer();
}

//
// Called once at initialization
//
void OurTestScene::Init()
{
	camera = new Camera(
		45.0f * fTO_RAD,		// field-of-view (radians)
		(float)window_width / window_height,	// aspect ratio
		1.0f,					// z-near plane (everything closer will be clipped/removed)
		500.0f);				// z-far plane (everything further will be clipped/removed)

	// Move camera to (0,0,5)
	camera->moveTo({ 0, 0, 5 });

	// Create objects
	quad = new QuadModel(dxdevice, dxdevice_context);
	cube = new Cube(dxdevice, dxdevice_context);
	cube1 = new Cube(dxdevice, dxdevice_context);
	cube2 = new Cube(dxdevice, dxdevice_context);
	sponza = new OBJModel("assets/crytek-sponza/sponza.obj", dxdevice, dxdevice_context);
	spaceship = new OBJModel("assets/Spaceship/spaceship.obj", dxdevice, dxdevice_context);
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void OurTestScene::Update(
	float dt,
	InputHandler* input_handler)
{
	
	float sensitivty = 0.5f * dt;
	long mousedx = input_handler->GetMouseDeltaX();
	long mousedy = input_handler->GetMouseDeltaY();

	camera->pitch -= mousedy * sensitivty;
	camera->yaw -= mousedx * sensitivty;
	
	if(camera->pitch > PI / 2) 
	{
		camera->pitch = PI / 2;
	}
	else if(camera->pitch < -PI / 2)
	{
		camera->pitch = -PI / 2;
	}

	// Basic camera control
	/*if (input_handler->IsKeyPressed(Keys::Up) || input_handler->IsKeyPressed(Keys::W))
		camera->move({ 0.0f, 0.0f, -camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Down) || input_handler->IsKeyPressed(Keys::S))
		camera->move({ 0.0f, 0.0f, camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Right) || input_handler->IsKeyPressed(Keys::D))
		camera->move({ camera_vel * dt, 0.0f, 0.0f });
	if (input_handler->IsKeyPressed(Keys::Left) || input_handler->IsKeyPressed(Keys::A))
		camera->move({ -camera_vel * dt, 0.0f, 0.0f });*/
	
	if (input_handler->IsKeyPressed(Keys::Up) || input_handler->IsKeyPressed(Keys::W))
		camera->moveForward(camera_vel, dt);
	if (input_handler->IsKeyPressed(Keys::Down) || input_handler->IsKeyPressed(Keys::S))
		camera->moveBackward(camera_vel, dt);
	if (input_handler->IsKeyPressed(Keys::Right) || input_handler->IsKeyPressed(Keys::D))
		camera->moveLeft(camera_vel, dt);
	if (input_handler->IsKeyPressed(Keys::Left) || input_handler->IsKeyPressed(Keys::A))
		camera->moveRight(camera_vel, dt);


	// Now set/update object transformations
	// This can be done using any sequence of transformation matrices,
	// but the T*R*S order is most common; i.e. scale, then rotate, and then translate.
	// If no transformation is desired, an identity matrix can be obtained 
	// via e.g. Mquad = linalg::mat4f_identity; 

	// Quad model-to-world transformation
	Mquad = mat4f::translation(0, 0, 0) *			// No translation
		mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) *	// Rotate continuously around the y-axis
		mat4f::scaling(1.5, 1.5, 1.5);				// Scale uniformly to 150%

	Mcube = mat4f::translation(5, 0, 0) *
		mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) *
		mat4f::scaling(2, 2, 2);
	
//	m4f MCube_T = mat4f::translation(Mcube.m14, Mcube.m24, Mcube.m34);
	Mcube1 = Mcube.translation(5, 0, 0) * Mcube.scaling(2, 2, 2) * mat4f::translation(std::cos(angle) * 1.5, std::sin(angle) * 1.5, 0) *
		mat4f::rotation(0.0f, 0.0f, 0.0f) *
		mat4f::scaling(0.5, 0.5, 0.5);

	Mcube2 = Mcube1 * mat4f::translation(std::cos(angle) * 1.5, std::sin(angle) * 1.5, 1) *
		mat4f::rotation(angle, 1.0f, 0.0f, 0.0f) *
		mat4f::scaling(0.5, 0.5, 0.5);


	// Sponza model-to-world transformation
	Msponza = mat4f::translation(0, -5, 0) *		 // Move down 5 units
		mat4f::rotation(fPI / 2, 0.0f, 1.0f, 0.0f) * // Rotate pi/2 radians (90 degrees) around y
		mat4f::scaling(0.05f);						 // The scene is quite large so scale it down to 5%

	Mspaceship = mat4f::translation(-7, 0, 0) *
		mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) *
		mat4f::scaling(0.2f, 0.2f, 0.2f);

	// Increment the rotation angle.
	angle += angle_vel * dt;

	// Print fps
	fps_cooldown -= dt;
	if (fps_cooldown < 0.0)
	{
		std::cout << "fps " << (int)(1.0f / dt) << std::endl;
//		printf("fps %i\n", (int)(1.0f / dt));
		fps_cooldown = 2.0;
	}
	
	light.x += std::cos(angle) * 2;
}

//
// Called every frame, after update
//
void OurTestScene::Render()
{
	// Bind transformation_buffer to slot b0 of the VS
	dxdevice_context->VSSetConstantBuffers(0, 1, &transformation_buffer);
	dxdevice_context->PSSetConstantBuffers(0, 1, &lightandcamera_buffer);
	dxdevice_context->PSSetConstantBuffers(1, 1, &mtl_buffer);

	// Obtain the matrices needed for rendering from the camera
	Mview = camera->get_WorldToViewMatrix();
	Mproj = camera->get_ProjectionMatrix();
	
	UpdateMaterialBuffer(vec4f(0,0,255,1));

	// Load matrices + the Quad's transformation to the device and render it
	UpdateTransformationBuffer(Mquad, Mview, Mproj);
	quad->Render();

	UpdateTransformationBuffer(Mcube, Mview, Mproj);
	cube->Render();

	UpdateTransformationBuffer(Mcube1, Mview, Mproj);
	cube1->Render();

	UpdateTransformationBuffer(Mcube2, Mview, Mproj);
	cube2->Render();
	
	UpdateTransformationBuffer(Mspaceship, Mview, Mproj);
	spaceship->Render();

	// Load matrices + Sponza's transformation to the device and render it
	UpdateTransformationBuffer(Msponza, Mview, Mproj);
	sponza->Render();
	
	UpdateLightAndCameraBuffer(light, camera->position.xyz0());
	
}

void OurTestScene::Release()
{
	SAFE_DELETE(quad);
	SAFE_DELETE(cube);
	SAFE_DELETE(cube1);
	SAFE_DELETE(cube2);
	SAFE_DELETE(spaceship);
	SAFE_DELETE(sponza);
	SAFE_DELETE(camera);

	SAFE_RELEASE(transformation_buffer);
	// + release other CBuffers
	SAFE_RELEASE(lightandcamera_buffer);
	SAFE_RELEASE(mtl_buffer);
}

void OurTestScene::WindowResize(
	int window_width,
	int window_height)
{
	if (camera)
		camera->aspect = float(window_width) / window_height;

	Scene::WindowResize(window_width, window_height);
}

void OurTestScene::InitTransformationBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(TransformationBuffer);
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &transformation_buffer));
}

void OurTestScene::UpdateTransformationBuffer(
	mat4f ModelToWorldMatrix,
	mat4f WorldToViewMatrix,
	mat4f ProjectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(transformation_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	TransformationBuffer* matrix_buffer_ = (TransformationBuffer*)resource.pData;
	matrix_buffer_->ModelToWorldMatrix = ModelToWorldMatrix;
	matrix_buffer_->WorldToViewMatrix = WorldToViewMatrix;
	matrix_buffer_->ProjectionMatrix = ProjectionMatrix;
	dxdevice_context->Unmap(transformation_buffer, 0);
}

void OurTestScene::InitLightAndCameraBuffer() 
{
	HRESULT hr;
	D3D11_BUFFER_DESC LightCameraBuffer_desc = { 0 };
	LightCameraBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	LightCameraBuffer_desc.ByteWidth = sizeof(TransformationBuffer);
	LightCameraBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	LightCameraBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	LightCameraBuffer_desc.MiscFlags = 0;
	LightCameraBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&LightCameraBuffer_desc, nullptr, &lightandcamera_buffer));
}

void OurTestScene::UpdateLightAndCameraBuffer(
	vec4f lightposition,
	vec4f cameraposition)
{
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(lightandcamera_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	LightandCameraBuffer* lightcamera_buffer = (LightandCameraBuffer*)resource.pData;
	lightcamera_buffer->lightposition = lightposition;
	lightcamera_buffer->cameraposition = cameraposition;	
	dxdevice_context->Unmap(lightandcamera_buffer, 0);

}

void OurTestScene::InitMaterialBuffer() 
{
	HRESULT hr;
	D3D11_BUFFER_DESC PhongColorAndShininessBuffer_desc = { 0 };
	PhongColorAndShininessBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	PhongColorAndShininessBuffer_desc.ByteWidth = sizeof(TransformationBuffer);
	PhongColorAndShininessBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	PhongColorAndShininessBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	PhongColorAndShininessBuffer_desc.MiscFlags = 0;
	PhongColorAndShininessBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&PhongColorAndShininessBuffer_desc, nullptr, &mtl_buffer));
}

void OurTestScene::UpdateMaterialBuffer(vec4f colorandshine) 
{
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(mtl_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	PhongColorAndShininessBuffer* phong_buffer = (PhongColorAndShininessBuffer*)resource.pData;
	phong_buffer->colorandshine = colorandshine;	
	dxdevice_context->Unmap(mtl_buffer, 0);
}