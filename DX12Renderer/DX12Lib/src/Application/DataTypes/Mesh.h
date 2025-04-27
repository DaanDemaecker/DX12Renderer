// Mesh.h

// This class will serve as a wrapper for a single mesh

#ifndef _MESH_
#define _MESH_

// File includes
#include "Includes/GlmIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Application/DataTypes/Structs.h"
#include "Application/Buffers/VertexBuffer.h"
#include "Application/Buffers/IndexBuffer.h"

// Standard library includes
#include <wrl.h>
#include <vector>


namespace DDM
{
	// Class forward declarations
	class CommandList;

	class Mesh
	{
	public:
		Mesh();
		virtual ~Mesh();

		Mesh(Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;

		Mesh& operator=(Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;


		void Draw(CommandList& commandList, DirectX::XMMATRIX& viewMatrix, DirectX::XMMATRIX& projectionMatrix);

		// Copyright(c) 2018 Jeremiah van Oosten
		static std::unique_ptr<Mesh> CreateCube(CommandList& commandList);

		void SetPosition(float x, float y, float z);

	private:

		void Initialize(CommandList& commandList);

		//Is model initialized
		bool m_Initialized{ false };

		// Rotation vector
		DirectX::XMVECTOR m_Rotation{0,0,0,0};
		// Position vector
		glm::vec3 m_Position{0,0,0};
		// Scale vector
		DirectX::XMVECTOR m_Scale{ 1, 1, 1 };


		DirectX::XMMATRIX m_ModelMatrix;

		std::vector<VertexPosColor> m_Vertices{};
		std::vector<uint16_t> m_Indices{};
		
		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
		
		void SetMVPMatrix(CommandList& commandList, DirectX::XMMATRIX& viewMatrix, DirectX::XMMATRIX& projectionMatrix);

	};
}

#endif // !_MESH_
