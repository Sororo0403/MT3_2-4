#include <Novice.h>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>
#include <imgui.h>
#include <algorithm> // std::absを使うために必要
// #include <array> // std::array を使うために必要でしたが、Triangle構造体の変更により不要になりました

// ウィンドウの横幅
static const float kWindowWidth = 1280.0f;
// ウィンドウの縦幅
static const float kWindowHeight = 720.0f;

// タイトル
const char kWindowTitle[] = "LE2B_19_ハタナカ_タロウ";

/// <summary>
/// 4x4行列
/// </summary>
struct Matrix4x4 {
	float m[4][4];
};

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 {
	float x;
	float y;
	float z;
};

/// <summary>
/// 線分
/// </summary>
struct LineSegment {
	Vector3 start;
	Vector3 end;
};

/// <summary>
/// 三角形
/// </summary>
struct Triangle {
	Vector3 p1; // 頂点1
	Vector3 p2; // 頂点2
	Vector3 p3; // 頂点3
};


struct Plane {
	Vector3 normal;
	float distance;
};

/// <summary>
/// 単位行列を作成
/// </summary>
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}

/// <summary>
/// 4x4行列同士を乗算
/// </summary>
/// <param name="m1">左側の行列</param>
/// <param name="m2">右側の行列</param>
/// <returns>乗算結果の行列</returns>
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

/// <summary>
/// 3次元ベクトルの外積を計算する関数
/// </summary>
/// <param name="v1">左側のベクトル</param>
/// <param name="v2">右側のベクトル</param>
/// <returns>外積の結果ベクトル</returns>
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

/// <summary>
/// 3次元ベクトルの内積を計算する関数
/// </summary>
/// <param name="v1">左側のベクトル</param>
/// <param name="v2">右側のベクトル</param>
/// <returns>内積の結果</returns>
float Dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}


/// <summary>
/// x軸回転行列を作成
/// </summary>
/// <param name="radian">x軸の回転量(ラジアン)</param>
/// <returns>x軸回転行列</returns>
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 matrix = MakeIdentity4x4();
	matrix.m[1][1] = std::cos(radian);
	matrix.m[1][2] = std::sin(radian);
	matrix.m[2][1] = -std::sin(radian);
	matrix.m[2][2] = std::cos(radian);
	return matrix;
}

/// <summary>
/// y軸回転行列を作成
/// </summary>
/// <param name="radian">y軸の回転量(ラジアン)</param>
/// <returns>y軸回転行列</returns>
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 matrix = MakeIdentity4x4();
	matrix.m[0][0] = std::cos(radian);
	matrix.m[0][2] = -std::sin(radian);
	matrix.m[2][0] = std::sin(radian);
	matrix.m[2][2] = std::cos(radian);
	return matrix;
}

/// <summary>
/// z軸回転行列を作成
/// </summary>
/// <param name="radian">z軸の回転量(ラジアン)</param>
/// <returns>z軸回転行列</returns>
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 matrix = MakeIdentity4x4();
	matrix.m[0][0] = std::cos(radian);
	matrix.m[0][1] = std::sin(radian);
	matrix.m[1][0] = -std::sin(radian);
	matrix.m[1][1] = std::cos(radian);
	return matrix;
}

/// <summary>
/// 平行移動行列の作成
/// </summary>
/// <param name="translate">平行移動量</param>
/// <returns>平行移動行列</returns>
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 matrix = MakeIdentity4x4();
	matrix.m[3][0] = translate.x;
	matrix.m[3][1] = translate.y;
	matrix.m[3][2] = translate.z;
	return matrix;
}

/// <summary>
/// スケーリング行列の作成
/// </summary>
/// <param name="scale">スケーリング量</param>
/// <returns>スケーリング行列</returns>
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 matrix = MakeIdentity4x4();
	matrix.m[0][0] = scale.x;
	matrix.m[1][1] = scale.y;
	matrix.m[2][2] = scale.z;
	return matrix;
}

/// <summary>
/// アフィン変換行列を作成
/// </summary>
/// <param name="scale">スケーリング量</param>
/// <param name="rotate">回転量 (各軸の回転角、ラジアン)</param>
/// <param name="translate">平行移動量</param>
/// <returns>アフィン変換行列</returns>
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	// 回転行列を合成
	Matrix4x4 rotateXYZMatrix = Multiply(rotateZMatrix, Multiply(rotateYMatrix, rotateXMatrix));

	return Multiply(scaleMatrix, Multiply(rotateXYZMatrix, translateMatrix));
}


/// <summary>
/// 逆行列を計算
/// </summary>
/// <param name="matrix">逆行列を計算する行列</param>
/// <returns>逆行列</returns>
Matrix4x4 Inverse(const Matrix4x4& matrix) {
	Matrix4x4 result = MakeIdentity4x4();

	// 回転成分の転置
	result.m[0][0] = matrix.m[0][0]; result.m[1][0] = matrix.m[0][1]; result.m[2][0] = matrix.m[0][2];
	result.m[0][1] = matrix.m[1][0]; result.m[1][1] = matrix.m[1][1]; result.m[2][1] = matrix.m[1][2];
	result.m[0][2] = matrix.m[2][0]; result.m[1][2] = matrix.m[2][1]; result.m[2][2] = matrix.m[2][2];

	// 平行移動成分の逆変換
	result.m[3][0] = -matrix.m[3][0] * result.m[0][0] - matrix.m[3][1] * result.m[1][0] - matrix.m[3][2] * result.m[2][0];
	result.m[3][1] = -matrix.m[3][0] * result.m[0][1] - matrix.m[3][1] * result.m[1][1] - matrix.m[3][2] * result.m[2][1];
	result.m[3][2] = -matrix.m[3][0] * result.m[0][2] - matrix.m[3][1] * result.m[1][2] - matrix.m[3][2] * result.m[2][2];

	return result;
}

/// <summary>
/// 透視投影行列を作成する関数
/// </summary>
/// <param name="fovY">視野角 (ラジアン)</param>
/// <param name="aspectRatio">アスペクト比</param>
/// <param name="nearClip">ニアクリップ距離</param>
/// <param name="farClip">ファークリップ距離</param>
/// <returns>透視投影行列</returns>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result;
	float h = 1.0f / tanf(fovY / 2.0f);

	result.m[0][0] = h / aspectRatio;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = h;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	result.m[3][3] = 0.0f;

	return result;
}

/// <summary>
/// ビューポート変換行列を作成する関数
/// </summary>
/// <param name="left">ビューポートの左端の座標</param>
/// <param name="top">ビューポートの上端の座標</param>
/// <param name="width">ビューポートの幅</param>
/// <param name="height">ビューポートの高さ</param>
/// <param name="minDepth">深度の最小値</param>
/// <param name="maxDepth">深度の最大値</param>
/// <returns>ビューポート変換行列</returns>
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result;

	result.m[0][0] = width / 2.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[2][3] = 0.0f;

	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;

	return result;
}

/// <summary>
/// ベクトルを4x4行列で変換
/// </summary>
/// <param name="vector">変換するベクトル</param>
/// <param name="matrix">変換行列</param>
/// <returns>変換後のベクトル</returns>
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	float w =
		matrix.m[0][3] * vector.x +
		matrix.m[1][3] * vector.y +
		matrix.m[2][3] * vector.z +
		matrix.m[3][3];
	result.x =
		(matrix.m[0][0] * vector.x +
			matrix.m[1][0] * vector.y +
			matrix.m[2][0] * vector.z +
			matrix.m[3][0]) / w;
	result.y =
		(matrix.m[0][1] * vector.x +
			matrix.m[1][1] * vector.y +
			matrix.m[2][1] * vector.z +
			matrix.m[3][1]) / w;
	result.z =
		(matrix.m[0][2] * vector.x +
			matrix.m[1][2] * vector.y +
			matrix.m[2][2] * vector.z +
			matrix.m[3][2]) / w;
	return result;
}

/// <summary>
/// 線分をワイヤーフレームで描画します。
/// </summary>
/// <param name="lineSegment">描画する線分</param>
/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
/// <param name="viewportMatrix">ビューポート変換行列</param>
/// <param name="color">描画色</param>
void DrawLineSegment(const LineSegment& lineSegment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// ワールド変換は線分自体には不要 (点はすでにワールド座標)
	// 直接ビュープロジェクション行列で変換

	// スクリーン座標変換
	Vector3 screenStart = Transform(Transform(lineSegment.start, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = Transform(Transform(lineSegment.end, viewProjectionMatrix), viewportMatrix);

	// スクリーン座標で線分を描画
	Novice::DrawLine(
		static_cast<int>(screenStart.x),
		static_cast<int>(screenStart.y),
		static_cast<int>(screenEnd.x),
		static_cast<int>(screenEnd.y),
		color
	);
}


/// <summary>
/// グリッドをワイヤーフレームで描画
/// </summary>
/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
/// <param name="viewportMatrix">ビューポート変換行列</param>
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	// グリッドの属性
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / static_cast<float>(kSubdivision);

	// グリッドの色 (灰色)
	uint32_t gridColor = 0xAAAAAAFF;

	// グリッドのワールド変換行列 (今回は単位行列)
	Matrix4x4 worldMatrix = MakeIdentity4x4();
	// ワールド * ビュープロジェクション行列
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);

	// 奥から手前への線を順々に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + kGridEvery * xIndex;

		// 線分の開始点と終了点
		Vector3 startPoint = { x, 0.0f, -kGridHalfWidth };
		Vector3 endPoint = { x, 0.0f, kGridHalfWidth };

		// スクリーン座標変換
		Vector3 screenStart = Transform(Transform(startPoint, worldViewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(endPoint, worldViewProjectionMatrix), viewportMatrix);

		// スクリーン座標で線を描画
		Novice::DrawLine(
			static_cast<int>(screenStart.x),
			static_cast<int>(screenStart.y),
			static_cast<int>(screenEnd.x),
			static_cast<int>(screenEnd.y),
			gridColor
		);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + kGridEvery * zIndex;

		// 線分の開始点と終了点
		Vector3 startPoint = { -kGridHalfWidth, 0.0f, z };
		Vector3 endPoint = { kGridHalfWidth, 0.0f, z };

		// スクリーン座標変換
		Vector3 screenStart = Transform(Transform(startPoint, worldViewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(endPoint, worldViewProjectionMatrix), viewportMatrix);

		// スクリーン座標で線を描画
		Novice::DrawLine(
			static_cast<int>(screenStart.x),
			static_cast<int>(screenStart.y),
			static_cast<int>(screenEnd.x),
			static_cast<int>(screenEnd.y),
			gridColor
		);
	}
}

/// <summary>
/// ベクトルを正規化します。
/// </summary>
/// <param name="vector">正規化するベクトル</param>
/// <returns>正規化されたベクトル</returns>
Vector3 Normalize(const Vector3& vector) {
	float length = std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	if (length == 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}
	return { vector.x / length, vector.y / length, vector.z / length };
}

/// <summary>
/// ベクトルに垂直なベクトルを計算します。
/// </summary>
/// <param name="vector">基準となるベクトル</param>
/// <returns>垂直なベクトル</returns>
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y, vector.x, 0.0f };
	}
	return { 0.0f, vector.z, vector.y };
}

/// <summary>
/// 三角形をワイヤーフレームで描画します。
/// </summary>
/// <param name="triangle">描画する三角形</param>
/// <param name="viewProjectMatrix">ビュープロジェクション行列</param>
/// <param name="viewportMatrix">ビューポート変換行列</param>
/// <param name="color">描画色</param>
void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 screenP1 = Transform(Transform(triangle.p1, viewProjectMatrix), viewportMatrix);
	Vector3 screenP2 = Transform(Transform(triangle.p2, viewProjectMatrix), viewportMatrix);
	Vector3 screenP3 = Transform(Transform(triangle.p3, viewProjectMatrix), viewportMatrix);

	Novice::DrawLine(
		static_cast<int>(screenP1.x), static_cast<int>(screenP1.y),
		static_cast<int>(screenP2.x), static_cast<int>(screenP2.y),
		color);
	Novice::DrawLine(
		static_cast<int>(screenP2.x), static_cast<int>(screenP2.y),
		static_cast<int>(screenP3.x), static_cast<int>(screenP3.y),
		color);
	Novice::DrawLine(
		static_cast<int>(screenP3.x), static_cast<int>(screenP3.y),
		static_cast<int>(screenP1.x), static_cast<int>(screenP1.y),
		color);
}


/// <summary>
/// 線分と三角形の衝突判定を行います。
/// </summary>
/// <param name="lineSegment">衝突判定を行う線分</param>
/// <param name="triangle">衝突判定を行う三角形</param>
/// <returns>衝突している場合は true、それ以外は false</returns>
bool IsCollision(const LineSegment& lineSegment, const Triangle& triangle) {
	// 三角形の法線を計算
	Vector3 v0 = triangle.p1;
	Vector3 v1 = triangle.p2;
	Vector3 v2 = triangle.p3;
	Vector3 edge1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
	Vector3 edge2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
	Vector3 normal = Normalize(Cross(edge1, edge2));
	float distance = Dot(normal, v0);

	// 線分の方向ベクトル
	Vector3 lineDirection = {
		lineSegment.end.x - lineSegment.start.x,
		lineSegment.end.y - lineSegment.start.y,
		lineSegment.end.z - lineSegment.start.z
	};

	float dot_n_ld = Dot(normal, lineDirection);

	// 線分と平面が平行の場合
	const float kEpsilon = 1e-6f;
	if (std::abs(dot_n_ld) < kEpsilon) {
		// 線分の始点が平面上にあるかチェック (近似的に)
		float dist_start_plane = Dot(normal, lineSegment.start) - distance;
		if (std::abs(dist_start_plane) < kEpsilon) {
			// 平行かつ平面上に始点がある場合、ここでは衝突とみなさない（より厳密な判定が必要な場合あり）
			return false;
		}
		return false; // 平行で平面上にない
	}

	// 線分と平面の交点を計算
	float t = (distance - Dot(normal, lineSegment.start)) / dot_n_ld;

	// 交点が線分の範囲外の場合
	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	// 交点を計算
	Vector3 intersectionPoint = {
		lineSegment.start.x + t * lineDirection.x,
		lineSegment.start.y + t * lineDirection.y,
		lineSegment.start.z + t * lineDirection.z
	};

	// 交点が三角形の内部にあるか判定 (重心座標系)
	Vector3 p = intersectionPoint;
	Vector3 a = triangle.p1;
	Vector3 b = triangle.p2;
	Vector3 c = triangle.p3;

	Vector3 ab = { b.x - a.x, b.y - a.y, b.z - a.z };
	Vector3 ac = { c.x - a.x, c.y - a.y, c.z - a.z };
	Vector3 ap = { p.x - a.x, p.y - a.y, p.z - a.z };

	float dot_ab_ab = Dot(ab, ab);
	float dot_ab_ac = Dot(ab, ac);
	float dot_ac_ac = Dot(ac, ac);
	float dot_ap_ab = Dot(ap, ab);
	float dot_ap_ac = Dot(ap, ac);

	float denominator = dot_ab_ab * dot_ac_ac - dot_ab_ac * dot_ab_ac;
	if (std::abs(denominator) < kEpsilon) { // 三角形が縮退している場合など
		return false;
	}

	float u = (dot_ac_ac * dot_ap_ab - dot_ab_ac * dot_ap_ac) / denominator;
	float v = (dot_ab_ab * dot_ap_ac - dot_ab_ac * dot_ap_ab) / denominator;

	return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(
		kWindowTitle,
		static_cast<int>(kWindowWidth),
		static_cast<int>(kWindowHeight)
	);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// 線分の初期位置
	LineSegment lineSegment = { {-2.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 1.0f} }; // 適当な初期値を設定

	// カメラの属性
	Vector3 cameraTranslate{ 0.0f, 1.9f, -6.49f };
	Vector3 cameraRotate{ 0.26f, 0.0f, 0.0f }; // ラジアンで設定すること

	// 透視投影行列の設定
	float fovY = 0.45f;
	float aspectRatio = kWindowWidth / kWindowHeight;
	float nearClip = 0.1f;
	float farClip = 100.0f;

	// 三角形の初期頂点
	Triangle triangle = {
		{ -1.0f, 0.0f,  1.0f }, // p1 (左下)
		{  0.0f, 1.0f,  0.0f }, // p2 (上)
		{  1.0f, 0.0f,  1.0f }  // p3 (右下)
	};


	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// カメラのワールド変換行列 (回転 -> 平行移動)
		Matrix4x4 cameraRotateMatrix = Multiply(MakeRotateXMatrix(cameraRotate.x), Multiply(MakeRotateYMatrix(cameraRotate.y), MakeRotateZMatrix(cameraRotate.z)));
		Matrix4x4 cameraTranslateMatrix = MakeTranslateMatrix(cameraTranslate);
		Matrix4x4 cameraWorldMatrix = Multiply(cameraRotateMatrix, cameraTranslateMatrix);

		Matrix4x4 viewMatrix = Inverse(cameraWorldMatrix);

		// 透視投影行列
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);

		// ビュープロジェクション行列
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

		// ビューポート変換行列
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, kWindowWidth, kWindowHeight, 0.0f, 1.0f);

		// 線分と三角形の衝突判定
		bool isColliding = IsCollision(lineSegment, triangle);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f); // ラジアンなので注意
		ImGui::DragFloat3("LineSegment.Start", &lineSegment.start.x, 0.01f); // 線分の開始座標を操作
		ImGui::DragFloat3("LineSegment.End", &lineSegment.end.x, 0.01f);	// 線分の終了座標を操作
		ImGui::Text("Triangle Vertices");
		ImGui::DragFloat3("Vertex p1", &triangle.p1.x, 0.01f);
		ImGui::DragFloat3("Vertex p2", &triangle.p2.x, 0.01f);
		ImGui::DragFloat3("Vertex p3", &triangle.p3.x, 0.01f);
		ImGui::End();

		// グリッド描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 線分描画 (衝突していれば赤、そうでなければ白)
		if (isColliding) {
			DrawLineSegment(lineSegment, viewProjectionMatrix, viewportMatrix, 0xFF0000FF);
		}
		else {
			DrawLineSegment(lineSegment, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);
		}

		// 三角形描画
		DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, 0x00FF00FF);


		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}