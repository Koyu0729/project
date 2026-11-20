#include "stageOuterFloorAndWalls.h"

#include "camera.h"
#include "gameConfigData.h"
#include "manager.h"
#include "scene.h"
#include "stageBlockManager.h"

#include <algorithm>
#include <cmath>

void StageOuterFloorAndWalls::Init()
{
	CreateQuad();
	SetTexture("asset\\texture\\outerFloor.png");
	m_HasPlacedTiles = false;

	if (m_TwoSidedRasterizer != nullptr)
	{
		return;
	}

	D3D11_RASTERIZER_DESC desc{};
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_NONE;
	desc.DepthClipEnable = TRUE;

	Renderer::GetDevice()->CreateRasterizerState(&desc, &m_TwoSidedRasterizer);
}

void StageOuterFloorAndWalls::Uninit()
{
	if (m_TwoSidedRasterizer != nullptr)
	{
		m_TwoSidedRasterizer->Release();
		m_TwoSidedRasterizer = nullptr;
	}

	m_Stage = nullptr;
	UIQuadBase::Uninit();
}

void StageOuterFloorAndWalls::SetLayout(const GroundLoadData& ground, StageBlockManager& stage)
{
	m_Stage = &stage;
	m_FloorCenterX = ground.center.x;
	m_FloorY = ground.center.y + ground.halfSize.y + ground.tileHalfSizeY * 2.0f + 0.01f;

	stage.GetWalkableXRange(0.0f, 0.0f, m_LeftWallX, m_RightWallX);
}

void StageOuterFloorAndWalls::RecycleTiles(float cameraZ)
{
	int cameraTile = (int)floorf(cameraZ / TileLength);

	if (m_HasPlacedTiles && cameraTile == m_CameraTile)
	{
		return;
	}

	int firstTile = cameraTile - TileCount / 2;

	for (int i = 0; i < TileCount; i++)
	{
		int tile = firstTile + i;
		// 区画番号が負でも、使い回す床の番号を0～TileCount-1に収める
		int slot = (tile % TileCount + TileCount) % TileCount;
		m_TileCenters[slot] = ((float)tile + 0.5f) * TileLength;
	}

	m_CameraTile = cameraTile;
	m_HasPlacedTiles = true;
}

void StageOuterFloorAndWalls::DrawFloor()
{
	const float repeatX = FloorWidth / TextureRepeatLength;
	const float repeatZ = TileLength / TextureRepeatLength;

	VERTEX_3D vertex[4]{};
	vertex[0].Position = XMFLOAT3(-FloorWidth * 0.5f, 0.0f, TileLength * 0.5f);
	vertex[1].Position = XMFLOAT3(FloorWidth * 0.5f, 0.0f, TileLength * 0.5f);
	vertex[2].Position = XMFLOAT3(-FloorWidth * 0.5f, 0.0f, -TileLength * 0.5f);
	vertex[3].Position = XMFLOAT3(FloorWidth * 0.5f, 0.0f, -TileLength * 0.5f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(repeatX, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, repeatZ);
	vertex[3].TexCoord = XMFLOAT2(repeatX, repeatZ);

	for (VERTEX_3D& point : vertex)
	{
		point.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		point.Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}

	SetTexture("asset\\texture\\outerFloor.png");
	UpdateQuad(vertex);
	SetDraw3D();

	for (float z : m_TileCenters)
	{
		Renderer::SetWorldMatrix(XMMatrixTranslation(m_FloorCenterX, m_FloorY - 0.06f, z));
		DrawQuad();
	}
}

void StageOuterFloorAndWalls::DrawWall(float x, float minZ, float maxZ)
{
	for (float centerZ : m_TileCenters)
	{
		float startZ = std::max(minZ, centerZ - TileLength * 0.5f);
		float endZ = std::min(maxZ, centerZ + TileLength * 0.5f);

		if (endZ <= startZ)
		{
			continue;
		}

		XMMATRIX scale = XMMatrixScaling(1.0f, WallHeight, endZ - startZ);
		XMMATRIX translation = XMMatrixTranslation(x, m_FloorY, (startZ + endZ) * 0.5f);
		Renderer::SetWorldMatrix(scale * translation);
		DrawQuad();
	}
}

void StageOuterFloorAndWalls::DrawSideWalls(float cameraX)
{
	VERTEX_3D vertex[4]{};
	vertex[0].Position = XMFLOAT3(0.0f, 1.0f, -0.5f);
	vertex[1].Position = XMFLOAT3(0.0f, 1.0f, 0.5f);
	vertex[2].Position = XMFLOAT3(0.0f, 0.0f, -0.5f);
	vertex[3].Position = XMFLOAT3(0.0f, 0.0f, 0.5f);

	// 側壁の不透明度は上端が0、床に接する下端が0.5
	vertex[0].Diffuse = XMFLOAT4(0.24f, 0.49f, 0.79f, 0.0f);
	vertex[1].Diffuse = vertex[0].Diffuse;
	vertex[2].Diffuse = XMFLOAT4(0.24f, 0.49f, 0.79f, 0.50f);
	vertex[3].Diffuse = vertex[2].Diffuse;

	for (VERTEX_3D& point : vertex)
	{
		point.Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
		point.TexCoord = XMFLOAT2(0.5f, 0.5f);
	}

	float minZ = 0.0f;
	float maxZ = 0.0f;
	m_Stage->GetStageZClampRange(0.0f, minZ, maxZ);

	bool leftIsCloser = fabsf(cameraX - m_LeftWallX) < fabsf(cameraX - m_RightWallX);

	SetTexture("asset\\texture\\siro.png");
	UpdateQuad(vertex);
	SetDraw3D();

	Renderer::SetDepthEnable(false);

	if (leftIsCloser)
	{
		DrawWall(m_RightWallX, minZ, maxZ);
		DrawWall(m_LeftWallX, minZ, maxZ);
	}
	else
	{
		DrawWall(m_LeftWallX, minZ, maxZ);
		DrawWall(m_RightWallX, minZ, maxZ);
	}

	Renderer::SetDepthEnable(true);
}

static void RestoreRasterizer(ID3D11DeviceContext* context, ID3D11RasterizerState* previousRasterizer)
{
	context->RSSetState(previousRasterizer);

	if (previousRasterizer != nullptr)
	{
		previousRasterizer->Release();
	}
}

void StageOuterFloorAndWalls::Draw()
{
	Scene* scene = Manager::GetScene();
	if (scene == nullptr)
	{
		return;
	}

	Camera* camera = scene->GetGameObject<Camera>();
	if (camera == nullptr || m_Stage == nullptr || m_TwoSidedRasterizer == nullptr)
	{
		return;
	}

	Vector3 cameraPosition = camera->GetPosition();
	RecycleTiles(cameraPosition.z);

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	ID3D11RasterizerState* previousRasterizer = nullptr;
	context->RSGetState(&previousRasterizer);
	context->RSSetState(m_TwoSidedRasterizer);

	try
	{
		DrawFloor();
		DrawSideWalls(cameraPosition.x);
	}
	catch (...)
	{
		RestoreRasterizer(context, previousRasterizer);
		throw;
	}

	RestoreRasterizer(context, previousRasterizer);
}
