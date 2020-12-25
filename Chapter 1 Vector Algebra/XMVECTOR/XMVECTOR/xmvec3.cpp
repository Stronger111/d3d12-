/*
#include <windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;
//XM_CALLCONV
ostream& XM_CALLCONV operator <<(ostream& os, FXMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);
	os << "(" << dest.x << "," << dest.y << "," << dest.z << ")";
	return os;
}

int main()
{
	//设置bool 输出为TRUE 或者 false
	cout.setf(ios_base::boolalpha);
	//检查是否支持 SSE2指令集
	if (!XMVerifyCPUSupport())
	{
		cout << "directx math not supported" << endl;
		return 0;
	}

	XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
	XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

	XMVECTOR a = u + v;
	XMVECTOR b = u - v;
	//标量乘法
	XMVECTOR c = 10.0f * u;
	XMVECTOR L = XMVector3Length(u);
	//d=u/||u||
	XMVECTOR d = XMVector3Normalize(u);
	//s=u dot v
	XMVECTOR s = XMVector3Dot(u,v);
	XMVECTOR e = XMVector3Cross(u,v);
	//w 对于n的两个投影分量
	XMVECTOR projW;
	XMVECTOR perpW;
	XMVector3ComponentsFromNormal(&projW,&perpW,w,n);
	bool equal = XMVector3Equal(projW + perpW, w) != 0;
	bool notEqual = XMVector3NotEqual(projW + perpW, w) != 0;
	XMVECTOR angleVec = XMVector3AngleBetweenVectors(projW, perpW);
	float angleRadians = XMVectorGetX(angleVec); //Radians 弧度
	float angleDegess = XMConvertToDegrees(angleRadians);//Degess 度数
	cout << "u            =" << u << endl;
	cout << "v            =" << v << endl;
	cout << "w            =" << w << endl;
	cout << "n            =" << n << endl;
	cout << "a=u+v        =" << a << endl;
	cout << "b=u-v        =" << b << endl;
	cout << "c=10*u        ="<< c << endl;
	cout << "d=u/||u||        =" << d << endl;
	cout << "L=||u||        =" << L << endl;
	cout << "s=u dot v        =" << s << endl;
	cout << "e=u cross v        =" << e << endl;
	cout << "projW        =" << projW << endl;
	cout << "perpW       =" << perpW << endl;
	cout << "equal       =" << equal << endl;
	cout << "notEqual       =" << notEqual << endl;
	cout << "angleDegess       =" << angleDegess << endl;
	return 0;
}
*/