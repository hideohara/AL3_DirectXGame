#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <time.h>

using namespace DirectX;

// コンストラクタ
GameScene::GameScene() {}

// デストラクタ
GameScene::~GameScene() {
	delete spriteBG_;
	delete modelStage_;
	delete modelPlayer_;
	delete modelBeam_;
	delete modelEnemy_;
}

// 初期化
void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	// 乱数初期化
	srand((unsigned int)time(NULL));

	// BG
	textureHandleBG_ = TextureManager::Load("bg.jpg");
	spriteBG_ = Sprite::Create(textureHandleBG_, {0, 0});

	// ビュープロジェクションの初期化
	viewProjection_.eye = {0, 1, -6};
	viewProjection_.target = {0, 1, 0};
	viewProjection_.Initialize();

	// ステージ
	textureHandleStage_ = TextureManager::Load("stage.jpg");
	modelStage_ = Model::Create();
	worldTransformStage_.translation_ = {0, -1.5f, 0};
	worldTransformStage_.scale_ = {4.5f, 1, 40};
	worldTransformStage_.Initialize();

	// プレイヤー
	textureHandlePlayer_ = TextureManager::Load("player.png");
	modelPlayer_ = Model::Create();
	worldTransformPlayer_.scale_ = {0.5f, 0.5f, 0.5f};
	worldTransformPlayer_.Initialize();

	// ビーム
	textureHandleBeam_ = TextureManager::Load("beam.png");
	modelBeam_ = Model::Create();
	worldTransformBeam_.scale_ = {0.3f, 0.3f, 0.3f};
	worldTransformBeam_.Initialize();

	// 敵
	textureHandleEnemy_ = TextureManager::Load("enemy.png");
	modelEnemy_ = Model::Create();
	worldTransformEnemy_.scale_ = {0.5f, 0.5f, 0.5f};
	worldTransformEnemy_.Initialize();

	// サウンドデータの読み込み
	// soundDataHandle_ = audio_->LoadWave("Alarm01.wav");

	// 音声再生
	// audio_->PlayWave(soundDataHandle_);
	// 音声再生
	// voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);
}

// 更新
void GameScene::Update() {
	// 各シーンの更新処理を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayUpdate(); // ゲームプレイ更新
		break;
	}
}

// 表示
void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// 各シーンの表示処理を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayDraw2DBack(); // ゲームプレイ２Ｄ背景表示
		break;
	}

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	// 各シーンの表示処理を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayDraw3D(); // ゲームプレイ３Ｄ表示
		break;
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// 各シーンの表示処理を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayDraw2DNear(); // ゲームプレイ２Ｄ近景表示
		break;
	}

	// デバッグテキストの描画
	debugText_->DrawAll(commandList);
	//
	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

// *****************************************************
// ゲームプレイ
// *****************************************************

// ゲームプレイ更新
void GameScene::GamePlayUpdate() {
	PlayerUpdate(); // プレイヤー更新
	BeamUpdate();   // ビーム更新
	EnemyUpdate();  // 敵更新
	Collision();    // 衝突判定
}

// ゲームプレイ3D表示
void GameScene::GamePlayDraw3D() {
	// ステージ
	modelStage_->Draw(worldTransformStage_, viewProjection_, textureHandleStage_);

	// プレイヤー
	modelPlayer_->Draw(worldTransformPlayer_, viewProjection_, textureHandlePlayer_);

	// ビーム
	if (beamFlag_ == 1) {
		modelBeam_->Draw(worldTransformBeam_, viewProjection_, textureHandleBeam_);
	}

	// 敵
	if (enemyFlag_ == 1) {
		modelEnemy_->Draw(worldTransformEnemy_, viewProjection_, textureHandleEnemy_);
	}
}

// ゲームプレイ背景2D表示
void GameScene::GamePlayDraw2DBack() { spriteBG_->Draw(); }

// ゲームプレイ近景2D表示
void GameScene::GamePlayDraw2DNear() {
	// ゲームスコア
	char str[100];
	sprintf_s(str, "SCORE %d", gameScore_);
	debugText_->Print(str, 200, 10, 2);

	sprintf_s(str, "LIFE %d", playerLife_);
	debugText_->Print(str, 900, 10, 2);
}

// *****************************************************

// —------------------------------------------
// プレイヤー
// —------------------------------------------

// プレイヤー更新
void GameScene::PlayerUpdate() {
	// 移動

	// 右へ移動
	if (input_->PushKey(DIK_RIGHT)) {
		worldTransformPlayer_.translation_.x += 0.1f;
	}

	// 左へ移動
	if (input_->PushKey(DIK_LEFT)) {
		worldTransformPlayer_.translation_.x -= 0.1f;
	}

	// 移動制限
	if (worldTransformPlayer_.translation_.x > 4) {
		worldTransformPlayer_.translation_.x = 4;
	}
	if (worldTransformPlayer_.translation_.x < -4) {
		worldTransformPlayer_.translation_.x = -4;
	}

	//　行列更新
	worldTransformPlayer_.UpdateMatrix();
}

// —------------------------------------------
// ビーム
// —------------------------------------------

// ビーム更新
void GameScene::BeamUpdate() {
	// 移動
	BeamMove();

	// ビーム発生
	BeamBorn();

	//行列更新
	worldTransformBeam_.UpdateMatrix();
}

// ビーム移動
void GameScene::BeamMove() {
	// 存在すれば
	if (beamFlag_ == 1) {
		// 奥へ移動
		worldTransformBeam_.translation_.z += 0.3f;

		// 画面端ならば存在しない
		if (worldTransformBeam_.translation_.z > 40) {
			// 存在しない
			beamFlag_ = 0;
		}

		// 回転
		worldTransformBeam_.rotation_.x += 0.1f;
	}
}

// ビーム発生（発射）
void GameScene::BeamBorn() {
	// 存在しなければ
	if (beamFlag_ == 0) {

		// スペースキーを押したらビームを発射する
		if (input_->PushKey(DIK_SPACE)) {
			//ビーム座標にプレイヤー座標を代入する）
			worldTransformBeam_.translation_.x = worldTransformPlayer_.translation_.x;
			worldTransformBeam_.translation_.z = worldTransformPlayer_.translation_.z;

			// 存在する
			beamFlag_ = 1;
		}
	}
}

// —------------------------------------------
// 敵
// —------------------------------------------

// 敵更新
void GameScene::EnemyUpdate() {
	// 移動
	EnemyMove();

	// 敵発生
	EnemyBorn();

	//行列更新
	worldTransformEnemy_.UpdateMatrix();
}

// 敵移動
void GameScene::EnemyMove() {
	// 存在すれば
	if (enemyFlag_ == 1) {
		// 手前へ移動
		worldTransformEnemy_.translation_.z -= 0.2f;

		// 画面端ならば存在しない
		if (worldTransformEnemy_.translation_.z < -5) {
			// 存在しない
			enemyFlag_ = 0;
		}

		// 回転
		worldTransformEnemy_.rotation_.x -= 0.1f;
	}
}

// 敵発生
void GameScene::EnemyBorn() {
	// 存在しなければ
	if (enemyFlag_ == 0) {

		// 存在する
		enemyFlag_ = 1;

		// z座標を40にする
		worldTransformEnemy_.translation_.z = 40;

		// 乱数でＸ座標の指定
		int x = rand() % 80;
		float x2 = (float)x / 10 - 4;
		worldTransformEnemy_.translation_.x = x2;
	}
}

// ------------------------------------------------
// 衝突判定
// ------------------------------------------------

// 衝突判定
void GameScene::Collision() {
	// 衝突判定（プレイヤーと敵）
	CollisionPlayerEnemy();

	// 衝突判定（ビームと敵）
	CollisionBeamEnemy();
}

// 衝突判定（プレイヤーと敵）
void GameScene::CollisionPlayerEnemy() {
	// 敵が存在すれば
	if (enemyFlag_ == 1) {
		// 差を求める
		float dx = abs(worldTransformPlayer_.translation_.x - worldTransformEnemy_.translation_.x);
		float dz = abs(worldTransformPlayer_.translation_.z - worldTransformEnemy_.translation_.z);

		// 衝突したら
		if (dx < 1 && dz < 1) {
			// 存在しない
			enemyFlag_ = 0;

			// ライフを引く
			playerLife_ -= 1;
		}
	}
}

// 衝突判定（ビームと敵）
void GameScene::CollisionBeamEnemy() {
	// 敵が存在すれば
	if (enemyFlag_ == 1) {
		// ビームが存在すれば
		if (beamFlag_ == 1) {
			// 差を求める
			float dx =
			  abs(worldTransformBeam_.translation_.x - worldTransformEnemy_.translation_.x);
			float dz =
			  abs(worldTransformBeam_.translation_.z - worldTransformEnemy_.translation_.z);

			// 衝突したら
			if (dx < 1 && dz < 1) {
				// 存在しない
				enemyFlag_ = 0;
				beamFlag_ = 0;

				// スコア加算
				gameScore_ += 1;
			}
		}
	}
}
