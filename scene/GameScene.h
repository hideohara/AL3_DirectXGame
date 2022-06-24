﻿#pragma once

#include "Audio.h"
#include "DebugText.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Model.h"
#include "SafeDelete.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <DirectXMath.h>

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

  public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

  private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;
	DebugText* debugText_ = nullptr;

	/// <summary>
	/// ゲームシーン用
	/// </summary>

	// BG（スプライト）
	uint32_t textureHandleBG_ = 0;
	Sprite* spriteBG_ = nullptr;

	// ビュープロジェクション
	ViewProjection viewProjection_;

	// ステージ
	uint32_t textureHandleStage_ = 0;
	Model* modelStage_ = nullptr;
	WorldTransform worldTransformStage_[20];

	// プレイヤー
	uint32_t textureHandlePlayer_ = 0;
	Model* modelPlayer_ = nullptr;
	WorldTransform worldTransformPlayer_;

	// ビ－ム
	uint32_t textureHandleBeam_ = 0;
	Model* modelBeam_ = nullptr;
	WorldTransform worldTransformBeam_[10];
	int beamFlag_[10] = {}; // ビーム存在フラグ（0:存在しない、1:存在する）

	// 敵
	uint32_t textureHandleEnemy_ = 0;
	Model* modelEnemy_ = nullptr;
	WorldTransform worldTransformEnemy_[10];
	int enemyFlag_[10] = {};    // 敵存在フラグ（0:存在しない、1:存在する）
	float enemySpeed_[10] = {}; // 敵のスピード

	// サウンドデータハンドル
	// uint32_t soundDataHandle_ = 0;

	// 音声再生ハンドル
	// uint32_t voiceHandle_ = 0;

	//  値を表示したい変数
	// int32_t value_ = 0;

	// タイトル（スプライト）
	uint32_t textureHandleTitle_ = 0;
	Sprite* spriteTitle_ = nullptr;

	// エンター（スプライト）
	uint32_t textureHandleEnter_ = 0;
	Sprite* spriteEnter_ = nullptr;

	// ゲームオーバー（スプライト）
	uint32_t textureHandleGameOver_ = 0;
	Sprite* spriteGameOver_ = nullptr;

	int gameScore_ = 0;             // ゲームスコア
	int playerLife_ = 3;            // プレイヤーライフ
	int sceneMode_ = 1;             // シーンモード（0:ゲームプレイ　1:タイトル）
	int gameTimer_ = 0;             //タイマー変数
	int beamTimer_ = 0;             // ビーム発射タイマー
	float enemyJumpSpeed_[10] = {}; // 敵ジャンプの移動速度

	// サウンド
	uint32_t soundDataHandleTitleBGM_ = 0;    // タイトルBGM
	uint32_t soundDataHandleGamePlayBGM_ = 0; // ゲームプレイBGM
	uint32_t soundDataHandleGameOverBGM_ = 0; // ゲームオーバーBGM
	uint32_t soundDataHandleEnemyHitSE_ = 0;  // 敵ヒットSE
	uint32_t soundDataHandlePlayerHitSE_ = 0; // プレイヤーヒットSE
	uint32_t voiceHandleBGM_ = 0;             // 音声再生ハンドル

	void GamePlayUpdate();     // ゲームプレイ更新
	void GamePlayDraw3D();     // ゲームプレイ3D表示
	void GamePlayDraw2DBack(); // ゲームプレイ背景2D表示
	void GamePlayDraw2DNear(); // ゲームプレイ近景2D表示
	void GamePlayStart();      // ゲームプレイ初期化

	void PlayerUpdate();         // プレイヤー更新
	void BeamUpdate();           // ビーム更新
	void BeamMove();             // ビーム移動
	void BeamBorn();             // ビーム発生
	void EnemyUpdate();          // 敵更新
	void EnemyMove();            // 敵移動
	void EnemyBorn();            // 敵発生
	void Collision();            // 衝突判定
	void CollisionPlayerEnemy(); // 衝突判定（プレイヤーと敵）
	void CollisionBeamEnemy();   // 衝突判定（ビームと敵）
	void StageUpdate();          // ステージ更新
	void EnemyJump();            // 敵の消滅の演出

	void TitleUpdate();     // タイトル更新
	void TitleDraw2DNear(); // タイトル2D

	void GameOverUpdate();     // ゲームオーバー更新
	void GameOverDraw2DNear(); // ゲームオーバー2D
};
