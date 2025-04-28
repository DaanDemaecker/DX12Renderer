// Mesh.cpp

// Header include
#include "Mesh.h"
#include "Application/CommandList.h"

// Standard library includes
#include <iostream>

using namespace DirectX;
using namespace Microsoft::WRL;

DDM::Mesh::Mesh()
{

}

DDM::Mesh::~Mesh()
{
}

void DDM::Mesh::Initialize(CommandList& commandList)
{
    commandList.CopyVertexBuffer(m_VertexBuffer, m_Vertices.size(), sizeof(VertexPosColor), m_Vertices.data());
    commandList.CopyIndexBuffer<uint16_t>(m_IndexBuffer, m_Indices);

    m_Initialized = true;
}

void DDM::Mesh::SetMVPMatrix(CommandList& commandList, DirectX::XMMATRIX& viewMatrix, DirectX::XMMATRIX& projectionMatrix)
{
    XMMATRIX scaleMatrix = XMMatrixScalingFromVector(m_Scale);
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(m_Rotation);
    XMMATRIX translationMatrix = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

    m_ModelMatrix = scaleMatrix * rotationMatrix * translationMatrix;

    XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, viewMatrix);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, projectionMatrix);
    commandList.GetGraphicsCommandList()->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);
}



void DDM::Mesh::Draw(CommandList& commandList, DirectX::XMMATRIX& viewMatrix, DirectX::XMMATRIX& projectionMatrix)
{
    SetMVPMatrix(commandList, viewMatrix, projectionMatrix);
    commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList.SetVertexBuffer(0, m_VertexBuffer);
    commandList.SetIndexBuffer(m_IndexBuffer);
    commandList.DrawIndexed(static_cast<uint16_t>(m_Indices.size()));

}

std::unique_ptr<DDM::Mesh> DDM::Mesh::CreateCube(CommandList& commandList)
{
    auto mesh = std::make_unique<Mesh>();

    // A cube has six faces, each one pointing in a different direction.
    const int FaceCount = 6;

    static const XMVECTORF32 faceNormals[FaceCount] =
    {
        { 0,  0,  1 },
        { 0,  0, -1 },
        { 1,  0,  0 },
        { -1,  0,  0 },
        { 0,  1,  0 },
        { 0, -1,  0 },
    };

    static const XMVECTORF32 textureCoordinates[4] =
    {
        { 1, 0 },
        { 1, 1 },
        { 0, 1 },
        { 0, 0 },
    };

    // Create each face in turn.
    for (int i = 0; i < FaceCount; i++)
    {
        XMVECTOR normal = faceNormals[i];

        // Get two vectors perpendicular both to the face normal and to each other.
        XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

        XMVECTOR side1 = XMVector3Cross(normal, basis);
        XMVECTOR side2 = XMVector3Cross(normal, side1);

        // Six indices (two triangles) per face.
        size_t vbase = mesh->m_Vertices.size();
        mesh->m_Indices.push_back(static_cast<uint16_t>(vbase + 2));
        mesh->m_Indices.push_back(static_cast<uint16_t>(vbase + 1));
        mesh->m_Indices.push_back(static_cast<uint16_t>(vbase + 0));

        mesh->m_Indices.push_back(static_cast<uint16_t>(vbase + 3));
        mesh->m_Indices.push_back(static_cast<uint16_t>(vbase + 2));
        mesh->m_Indices.push_back(static_cast<uint16_t>(vbase + 0));

        // Four vertices per face.
        mesh->m_Vertices.push_back(VertexPosColor{ (normal - side1 - side2), XMFLOAT3(1,1,1) });
        mesh->m_Vertices.push_back(VertexPosColor{ (normal - side1 + side2), XMFLOAT3(1,0,0) });
        mesh->m_Vertices.push_back(VertexPosColor{ (normal + side1 + side2), XMFLOAT3(0,1,0) });
        mesh->m_Vertices.push_back(VertexPosColor{ (normal + side1 - side2), XMFLOAT3(0,0,1) });
    }

    mesh->Initialize(commandList);

    return mesh;
}

void DDM::Mesh::SetPosition(float x, float y, float z)
{
    m_Position.x = x;
    m_Position.y = y;
    m_Position.z = z;
}
