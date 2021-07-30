#include "MyFramework.h"
#include "Terrain.h"

Terrain::Terrain(Shader* shader, wstring heightFile) : shader(shader) {
	heightMap = new Texture(heightFile);

	CreateVertexDate();
	CreateIndexData();
	CreateNormalData();
	CreateBuffer();
}

Terrain::~Terrain() {
	SafeDelete(heightMap);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeRelease(vertexBuffer);
	SafeRelease(indexBuffer);
}

void Terrain::Update() {
	static Vector3 direction = Vector3(-1, -1, 1);
	ImGui::SliderFloat3("Direction", direction, -1, 1);
	shader->AsVector("Direction")->SetFloatVector(direction);
	Matrix world;
	D3DXMatrixIdentity(&world);

	shader->AsMatrix("World")->SetMatrix(world);
	shader->AsMatrix("View")->SetMatrix(Context::Get()->View());
	shader->AsMatrix("Projection")->SetMatrix(Context::Get()->Projection());
}

void Terrain::Render() {
	//for (int i = 0; i < vertexCount; i++) {
	//	Vector3 start = vertices[i].Position;
	//	Vector3 end = vertices[i].Position + vertices[i].Normal * 2;

	//	DebugLine::Get()->RenderLine(start, end, Color(0, 1, 0, 1));
	//}

	UINT stride = sizeof(TerrainVertex);
	UINT offset = 0;

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	shader->DrawIndexed(0, pass, indexCount);
}

float Terrain::GetHeight(Vector3& position)
{
	/*
	Grid 한칸의 길이가 length 라고 할 때 높이 = length * (x - v0.x) / length +
	*/
	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	if (x < 0 || x > width) return FLT_MIN;
	if (z < 0 || z > height) return FLT_MIN;

	UINT index[4];
	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + x + 1;
	index[3] = width * (z + 1) + x + 1;

	Vector3 v[4];
	for (int i = 0; i < 4; i++) {
		v[i] = vertices[index[i]].Position;
	}

	float ddx = (position.x - v[0].x) / 1.0f;
	float ddz = (position.z - v[0].z) / 1.0f;

	Vector3 result;

	if (ddx + ddz <= 1.0f) {
		result = v[0] + (v[2] - v[0]) * ddx + (v[1] - v[0]) * ddz;
	}
	else {
		ddx = 1.0f - ddx;
		ddz = 1.0f - ddz;

		result = v[3] + (v[1] - v[3]) * ddx + (v[2] - v[3]) * ddz;
	}

	return result.y;
}

float Terrain::GetVerticalRaycast(Vector3& position) {

	UINT x = (UINT)position.x;
	UINT z = (UINT)position.z;

	if (x < 0 || x > width) return FLT_MIN;
	if (z < 0 || z > height) return FLT_MIN;

	UINT index[4];
	index[0] = width * z + x;
	index[1] = width * (z + 1) + x;
	index[2] = width * z + x + 1;
	index[3] = width * (z + 1) + x + 1;

	Vector3 p[4];
	for (int i = 0; i < 4; i++) {
		p[i] = vertices[index[i]].Position;
	}

	Vector3 start(position.x, 50, position.z);
	Vector3 direction(0, -1, 0);

	// u = ddx, v = ddz
	float u, v, distance;

	Vector3 result(-1, FLT_MIN, -1);

	if (D3DXIntersectTri(&p[0], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE) {
		result = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;
	}

	if (D3DXIntersectTri(&p[3], &p[1], &p[2], &start, &direction, &u, &v, &distance) == TRUE) {
		result = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;
	}

	return result.y;
}

void Terrain::CreateVertexDate() {
	vector<Color> heights;
	heightMap->ReadPixel(DXGI_FORMAT_R8G8B8A8_UNORM, &heights);

	width = heightMap->GetWidth();
	height = heightMap->GetHeight();

	vertexCount = width * height;
	vertices = new TerrainVertex[vertexCount];
	for (UINT z = 0; z < height; z++) {
		for (UINT x = 0; x < width; x++) {
			UINT index = width * z + x;
			UINT pixel = width * (height - 1 - z) + x;
			vertices[index].Position.x = (float)x;
			vertices[index].Position.y = heights[pixel].r * 255.0f / 10.0f;
			vertices[index].Position.z = (float)z;
		}
	}


}

void Terrain::CreateIndexData(){
	indexCount = (width - 1) * (height - 1) * 6;
	indices = new UINT[indexCount];

	UINT index = 0;

	for (UINT y = 0; y < height - 1; y++) {
		for (UINT x = 0; x < width - 1; x++) {
			UINT i = (width + 1) * y + x;
			indices[index + 0] = width * y + x;
			indices[index + 1] = width * (y + 1) + x;
			indices[index + 2] = width * y + x + 1;
			indices[index + 3] = width * y + x + 1;
			indices[index + 4] = width * (y + 1) + x;
			indices[index + 5] = width * (y + 1) + x + 1;

			index += 6;
		}
	}
}

void Terrain::CreateNormalData() {
	for (UINT i = 0; i < indexCount / 3; i++) {
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		TerrainVertex v0 = vertices[index0];
		TerrainVertex v1 = vertices[index1];
		TerrainVertex v2 = vertices[index2];

		Vector3 a = v1.Position - v0.Position;
		Vector3 b = v2.Position - v0.Position;

		Vector3 normal;
		D3DXVec3Cross(&normal, &a, &b);

		vertices[index0].Normal += normal;
		vertices[index1].Normal += normal;
		vertices[index2].Normal += normal;
	}

	for (UINT i = 0; i < vertexCount; i++) {
		D3DXVec3Normalize(&vertices[i].Normal, &vertices[i].Normal);
	}
}

void Terrain::CreateBuffer() {
	// Create Vertex Buffer
	{
		/*
		GPU로 데이터를 넘겨서 그릴수 있도록 버퍼를 생성
		*/
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC)); // 구조체 멤버 안에 포인터 변수가 있을 때 사용하는 초기화 함수
		desc.ByteWidth = sizeof(TerrainVertex) * vertexCount; // 그려질 갯수
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 정점을 기르는 VERTEX BUFFER 사용

		/*
		GetDevice() : CPU 관련된 처리
		GetDC() : GPU 관련된 처리
		밑에 3줄이 RAM에서 VRAM으로 GPU를 사용하도록 복사하는 과정
		*/
		D3D11_SUBRESOURCE_DATA subResource = { 0 }; // 포인터 변수가 없는 구조체 초기화
		subResource.pSysMem = vertices; // 실제로 그릴 데이터의 시작 주소
		Check(D3D::GetDevice()->CreateBuffer(&desc, &subResource, &vertexBuffer));
	}

	// Create Index Buffer
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC)); // 구조체 멤버 안에 포인터 변수가 있을 때 사용하는 초기화 함수
		desc.ByteWidth = sizeof(UINT) * indexCount; // 그려질 갯수
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER; // 정점을 기르는 VERTEX BUFFER 사용

		D3D11_SUBRESOURCE_DATA subResource = { 0 }; // 포인터 변수가 없는 구조체 초기화
		subResource.pSysMem = indices;

		Check(D3D::GetDevice()->CreateBuffer(&desc, &subResource, &indexBuffer));
	}
}
