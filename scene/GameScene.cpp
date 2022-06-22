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
	delete spriteTitle_;
	delete spriteEnter_;
	delete spriteGameOver_;
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
	for (int i = 0; i < 10; i++) {
		worldTransformBeam_[i].scale_ = {0.3f, 0.3f, 0.3f};
		worldTransformBeam_[i].Initialize();
	}

	// 敵
	textureHandleEnemy_ = TextureManager::Load("enemy.png");
	modelEnemy_ = Model::Create();
	for (int i = 0; i < 10; i++) {
		worldTransformEnemy_[i].scale_ = {0.5f, 0.5f, 0.5f};
		worldTransformEnemy_[i].Initialize();
	}
	// タイトル(2Dスプライト)
	textureHandleTitle_ = TextureManager::Load("title.png");
	spriteTitle_ = Sprite::Create(textureHandleTitle_, {0, 0});

	// エンター(2Dスプライト)
	textureHandleEnter_ = TextureManager::Load("enter.png");
	spriteEnter_ = Sprite::Create(textureHandleEnter_, {400, 500});

	// ゲームオーバー(2Dスプライト)
	textureHandleGameOver_ = TextureManager::Load("gameover.png");
	spriteGameOver_ = Sprite::Create(textureHandleGameOver_, {0, 200});

	// サウンドデータの読み込み
	soundDataHandleTitleBGM_ = audio_->LoadWave("Audio/Ring05.wav");
	soundDataHandleGamePlayBGM_ = audio_->LoadWave("Audio/Ring08.wav");
	soundDataHandleGameOverBGM_ = audio_->LoadWave("Audio/Ring09.wav");
	soundDataHandleEnemyHitSE_ = audio_->LoadWave("Audio/chord.wav");
	soundDataHandlePlayerHitSE_ = audio_->LoadWave("Audio/tada.wav");

	// タイトルＢＧＭを再生
	voiceHandleBGM_ = audio_->PlayWave(soundDataHandleTitleBGM_, true);
}

// 更新
void GameScene::Update() {
	// 各シーンの更新処理を呼び出す
	switch (sceneMode_) {
	case 0:
		GamePlayUpdate(); // ゲームプレイ更新
		break;
	case 1:
		TitleUpdate(); // タイトル更新
		break;
	case 2:
		GameOverUpdate(); // ゲームオーバー更新
		break;
	}

	gameTimer_ += 1; //タイマー変数加算
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
	case 2:
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
	case 2:
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
		GamePlayDraw2DNear(); // ゲームプレイ２Ｄ前景表示
		break;
	case 1:
		TitleDraw2DNear(); // タイトル表示
		break;
	case 2:
		GamePlayDraw2DNear(); // ゲームプレイ２Ｄ前景表示
		GameOverDraw2DNear(); // ゲームオーバー更新
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

	// ライフ０でゲームオーバー
	if (playerLife_ <= 0) {
		sceneMode_ = 2;

		// BGM切り替え
		audio_->StopWave(voiceHandleBGM_); // BGMを停止
		voiceHandleBGM_ =
		  audio_->PlayWave(soundDataHandleGameOverBGM_, true); // ゲームオーバーBGMを再生
	}
}

// ゲームプレイ3D表示
void GameScene::GamePlayDraw3D() {
	// ステージ
	modelStage_->Draw(worldTransformStage_, viewProjection_, textureHandleStage_);

	// プレイヤー
	modelPlayer_->Draw(worldTransformPlayer_, viewProjection_, textureHandlePlayer_);

	// ビーム
	for (int i = 0; i < 10; i++) {
		if (beamFlag_[i] == 1) {
			modelBeam_->Draw(worldTransformBeam_[i], viewProjection_, textureHandleBeam_);
		}
	}

	// 敵
	for (int i = 0; i < 10; i++) {
		if (enemyFlag_[i] == 1) {
			modelEnemy_->Draw(worldTransformEnemy_[i], viewProjection_, textureHandleEnemy_);
		}
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
// ゲームプレイ初期化
void GameScene::GamePlayStart() {
	gameScore_ = 0;  // ゲームスコア
	playerLife_ = 3; // プレイヤーライフ
	gameTimer_ = 0;  //タイマー変数
	for (int i = 0; i < 10; i++) {
		beamFlag_[i] = 0; // ビーム存在フラグ（0:存在しない、1:存在する）
	}
	for (int i = 0; i < 10; i++) {
		enemyFlag_[i] = 0; // 敵存在フラグ（0:存在しない、1:存在する）
	}
}

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
	for (int i = 0; i < 10; i++) {
		worldTransformBeam_[i].UpdateMatrix();
	}
}

// ビーム移動
void GameScene::BeamMove() {
	for (int i = 0; i < 10; i++) {
		// 存在すれば
		if (beamFlag_[i] == 1) {
			// 奥へ移動
			worldTransformBeam_[i].translation_.z += 0.3f;

			// 画面端ならば存在しない
			if (worldTransformBeam_[i].translation_.z > 40) {
				// 存在しない
				beamFlag_[i] = 0;
			}

			// 回転
			worldTransformBeam_[i].rotation_.x += 0.1f;
		}
	}
}

// ビーム発生（発射）
void GameScene::BeamBorn() {
	// 発射タイマーが0ならば
	if (beamTimer_ == 0) {
		for (int i = 0; i < 10; i++) {
			// 存在しなければ
			if (beamFlag_[i] == 0) {

				// スペースキーを押したらビームを発射する
				if (input_->PushKey(DIK_SPACE)) {
					//ビーム座標にプレイヤー座標を代入する）
					worldTransformBeam_[i].translation_.x = worldTransformPlayer_.translation_.x;
					worldTransformBeam_[i].translation_.z = worldTransformPlayer_.translation_.z;

					// 存在する
					beamFlag_[i] = 1;

					// 発射タイマーを１にする
					beamTimer_ = 1;

					// ループ終了
					break;
				}
			}
		}
	}

	else {
		// 発射タイマーが1以上
		// 10を超えると再び発射が可能
		beamTimer_++;
		if (beamTimer_ > 10) {
			beamTimer_ = 0;
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
	for (int i = 0; i < 10; i++) {
		worldTransformEnemy_[i].UpdateMatrix();
	}
}

// 敵移動
void GameScene::EnemyMove() {
	for (int i = 0; i < 10; i++) {
		// 存在すれば
		if (enemyFlag_[i] == 1) {
			// 手前へ移動
			worldTransformEnemy_[i].translation_.z -= 0.2f;

			// 画面端ならば存在しない
			if (worldTransformEnemy_[i].translation_.z < -5) {
				// 存在しない
				enemyFlag_[i] = 0;
			}

			// 横移動
			worldTransformEnemy_[i].translation_.x += enemySpeed_[i];
			if (worldTransformEnemy_[i].translation_.x > 5) {
				enemySpeed_[i] = -0.2f;
			}
			if (worldTransformEnemy_[i].translation_.x < -5) {
				enemySpeed_[i] = 0.2f;
			}

			// 回転
			worldTransformEnemy_[i].rotation_.x -= 0.1f;
		}
	}
}

// 敵発生
void GameScene::EnemyBorn() {

	// 乱数で発生
	if (rand() % 10 == 0) {

		for (int i = 0; i < 10; i++) {
			// 存在しなければ
			if (enemyFlag_[i] == 0) {

				// 存在する
				enemyFlag_[i] = 1;

				// z座標を40にする
				worldTransformEnemy_[i].translation_.z = 40;

				// 乱数でＸ座標の指定
				int x = rand() % 80;
				float x2 = (float)x / 10 - 4;
				worldTransformEnemy_[i].translation_.x = x2;

				// 敵スピード
				if (rand() % 2 == 0) {
					enemySpeed_[i] = 0.2f;
				} else {
					enemySpeed_[i] = -0.2f;
				}

				// ループ終了
				break;
			}
		}
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
	for (int i = 0; i < 10; i++) {
		// 敵が存在すれば
		if (enemyFlag_[i] == 1) {
			// 差を求める
			float dx =
			  abs(worldTransformPlayer_.translation_.x - worldTransformEnemy_[i].translation_.x);
			float dz =
			  abs(worldTransformPlayer_.translation_.z - worldTransformEnemy_[i].translation_.z);

			// 衝突したら
			if (dx < 1 && dz < 1) {
				// 存在しない
				enemyFlag_[i] = 0;

				// ライフを引く
				playerLife_ -= 1;

				// プレイヤーヒットSE
				audio_->PlayWave(soundDataHandlePlayerHitSE_);
			}
		}
	}
}

// 衝突判定（ビームと敵）
void GameScene::CollisionBeamEnemy() {
	// 敵でループ
	for (int e = 0; e < 10; e++) {
		// 敵が存在すれば
		if (enemyFlag_[e] == 1) {
			// ビームでループ
			for (int b = 0; b < 10; b++) {
				// ビームが存在すれば
				if (beamFlag_[b] == 1) {

					// 差を求める
					float dx = abs(
					  worldTransformBeam_[b].translation_.x -
					  worldTransformEnemy_[e].translation_.x);
					float dz = abs(
					  worldTransformBeam_[b].translation_.z -
					  worldTransformEnemy_[e].translation_.z);

					// 衝突したら
					if (dx < 1 && dz < 1) {
						// 存在しない
						enemyFlag_[e] = 0;
						beamFlag_[b] = 0;

						// スコア加算
						gameScore_ += 1;

						// 敵ヒットSE
						audio_->PlayWave(soundDataHandleEnemyHitSE_);
					}
				}
			}
		}
	}
}

// ******************************************
// タイトル
// ******************************************

// タイトル更新
void GameScene::TitleUpdate() {
	// エンターキーを押した瞬間
	if (input_->TriggerKey(DIK_RETURN)) {
		// モードをゲームプレイへ変更
		sceneMode_ = 0;

		// ゲームプレイ初期化
		GamePlayStart();

		// BGM切り替え
		audio_->StopWave(voiceHandleBGM_); // BGMを停止
		voiceHandleBGM_ =
		  audio_->PlayWave(soundDataHandleGamePlayBGM_, true); // ゲームプレイBGMを再生
	}
}

// タイトル表示
void GameScene::TitleDraw2DNear() {
	// タイトル表示
	spriteTitle_->Draw();
	// エンター表示
	if (gameTimer_ % 40 >= 20) {
		spriteEnter_->Draw();
	}
}

// ******************************************
// ゲームオーバー
// ******************************************

// ゲームオーバー更新
void GameScene::GameOverUpdate() {
	// エンターキーを押した瞬間
	if (input_->TriggerKey(DIK_RETURN)) {
		// モードをタイトルへ変更
		sceneMode_ = 1;

		// BGM切り替え
		audio_->StopWave(voiceHandleBGM_);                                  // BGMを停止
		voiceHandleBGM_ = audio_->PlayWave(soundDataHandleTitleBGM_, true); // タイトルBGMを再生
	}
}

// ゲームオーバー表示
void GameScene::GameOverDraw2DNear() {
	// ゲームオーバー表示
	spriteGameOver_->Draw();
	// エンター表示
	if (gameTimer_ % 40 >= 20) {
		spriteEnter_->Draw();
	}
}
