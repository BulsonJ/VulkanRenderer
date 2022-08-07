#include "Engine.h"
#include <SDL.h>
#include <public/tracy/Tracy.hpp>
#include <backends/imgui_impl_sdl.h>

#include "Log.h"
#include "Image.h"

void Engine::init() {
	ZoneScoped;

	Log::Init();
	rend.init();
	setupScene();
}

void Engine::setupScene() {
	EngineTypes::MeshDesc fileMesh;
	Handle<RenderMesh> fileMeshHandle {};
	if (fileMesh.loadFromObj("../../assets/meshes/cube.obj"))
	{
		fileMeshHandle = rend.uploadMesh(fileMesh);
	}

	EngineTypes::MeshDesc cubeMeshDesc = EngineTypes::MeshDesc::GenerateCube();
	Handle<RenderMesh> cubeMeshHandle = rend.uploadMesh(cubeMeshDesc);

	static const std::string texturePaths[] = {
		"../../assets/textures/default.png",
		"../../assets/textures/texture.jpg",
		"../../assets/textures/metal/metal_albedo.png",
		"../../assets/textures/metal/metal_normal.png"
	};

	std::vector<Handle<Handle<Image>>> textures;
	for (int i = 0; i < std::size(texturePaths); ++i)
	{
		EngineTypes::Texture img;
		EngineTypes::TextureUtil::LoadTextureFromFile(texturePaths[i].c_str(), img);
		Handle<Handle<Image>> texHandle = rend.uploadTexture(img);
		textures.push_back(texHandle);
	}

	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			const EngineTypes::RenderObject materialTestObject{
				.meshHandle = fileMeshHandle,
				.textureHandle = textures[2],
				.normalHandle = textures[3],
				.translation = { 1.0f * j,-0.5f,1.0f * i},
			};
			renderObjects.push_back(materialTestObject);
		}
	}


	LOG_CORE_INFO("Scene setup.");
}

void Engine::run()
{
	bool bQuit = { false };
	SDL_Event e;

	while (!bQuit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			ImGui_ImplSDL2_ProcessEvent(&e);
			if (e.type == SDL_QUIT)
			{
				bQuit = true;
			}
			if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_MINIMIZED)
			{
				rend.window.resized = true;
				break;
			}
		}
		rend.draw(renderObjects);
	}
}

void Engine::deinit()
{
	ZoneScoped;
	rend.deinit();
}