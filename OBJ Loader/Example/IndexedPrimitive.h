//
// BGTD 9201
//	Class that represents a simple indexed primitive
//

#include <d3d11_1.h>
#include <SimpleMath.h>
#include <Effects.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;


class IndexedPrimitive
{
public:
	IndexedPrimitive();
	~IndexedPrimitive();

	// initialze the geometry
	void InitializeGeometry(ID3D11Device* pDevice);

	// set up the effect
	void InitializeShaders(ID3D11Device* pDevice);

	// draw the primitive
	void Draw(ID3D11DeviceContext* pDeviceContext, const Matrix& world, const Matrix& view, const Matrix& projection);


private:
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	ID3D11InputLayout* pInputLayout;

	BasicEffect*  pEffect;


};