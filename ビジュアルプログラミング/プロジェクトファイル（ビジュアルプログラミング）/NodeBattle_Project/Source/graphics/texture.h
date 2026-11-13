#pragma once
#include <d3d11.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "DirectXTex.h"

class Texture
{
  private:
	// GPU画像はここだけが所有し、利用側へは借りた参照を返す
	static std::unordered_map<std::string, ID3D11ShaderResourceView *> m_TexturePool;

	// ワーカーで読み込んだ画像を、メインスレッドのGPU作成まで保持する
	static std::unordered_map<std::string, std::unique_ptr<DirectX::ScratchImage>> m_CPUImages;
	static std::mutex m_Mutex;

	// パスの区切りと大小文字を統一した共有キーを作る
	static std::string CreateKey(const char *fileName);

  public:
	// 画像をCPU側で展開して一時保存する
	static bool PrepareCPU(const char *fileName);
	// 一時保存した画像をGPUへ渡す。メイン側から呼び出す
	static void UploadPrepared();
	// 共有済みのGPU画像を返す。未作成なら読み込みとGPU作成を行う
	static ID3D11ShaderResourceView *Load(const char *fileName);
	static void UnloadAll();
};
