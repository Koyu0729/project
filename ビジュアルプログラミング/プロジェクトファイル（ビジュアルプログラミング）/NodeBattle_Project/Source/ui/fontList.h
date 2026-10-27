#pragma once
#include <dwrite.h>
#include <wrl.h>
#include <vector>
#include <string>

class CustomFontFileEnumerator : public IDWriteFontFileEnumerator
{
public:
	CustomFontFileEnumerator(IDWriteFactory* factory, const std::vector<std::wstring>& fontFilePaths)
		: refCount_(0), factory_(factory), fontFilePaths_(fontFilePaths), currentFileIndex_(-1)
	{
		factory_->AddRef();
	}

	~CustomFontFileEnumerator()
	{
		factory_->Release();
	}

	IFACEMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override
	{
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteFontCollectionLoader))
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		else
		{
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
	}

	IFACEMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&refCount_);
	}

	IFACEMETHODIMP_(ULONG) Release() override
	{
		ULONG newCount = InterlockedDecrement(&refCount_);
		if (newCount == 0)
			delete this;

		return newCount;
	}

	IFACEMETHODIMP MoveNext(OUT BOOL* hasCurrentFile) override {
		if (++currentFileIndex_ < static_cast<int>(fontFilePaths_.size())) {
			*hasCurrentFile = TRUE;
			return S_OK;
		}
		else {
			*hasCurrentFile = FALSE;
			return S_OK;
		}
	}

	IFACEMETHODIMP GetCurrentFontFile(OUT IDWriteFontFile** fontFile) override
	{
		// フォントファイルを読み込む
		return factory_->CreateFontFileReference(fontFilePaths_[currentFileIndex_].c_str(), nullptr, fontFile);
	}

private:
	ULONG refCount_;

	// DirectWriteファクトリ
	IDWriteFactory* factory_;

	// フォントファイルのパス
	std::vector<std::wstring> fontFilePaths_;

	// 現在のファイルインデックス
	int currentFileIndex_;
};

