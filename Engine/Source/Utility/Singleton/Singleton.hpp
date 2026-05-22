#pragma once
#include <cassert>	// assert
#include <utility>

#define DECLARE_SINGLETON(ClassName)	\
	friend class Singleton<ClassName>;	\
	static constexpr const char* Name = #ClassName;

template <typename T>
// シングルトンクラス
class Singleton {
protected:
	// インスタンス
	static T* instance;

	Singleton() = default;
	virtual ~Singleton() = default;
public:
	Singleton(const Singleton) = delete;
	Singleton& operator=(const Singleton) = delete;
	// 生成
	template <typename... Args>
	static void Create(Args&&... args) {
		if (instance == nullptr) {
			instance = new T(std::forward<Args>(args)...);
		}
	}
	// 削除
	static void Delete() {
		if (instance != nullptr) {
			delete instance;
			instance = nullptr;
		}
	}
	// 取得
	static T& Get() {
		assert(instance != nullptr && "Instanceが存在しない");
		return *instance;
	}
	// 生ポインタ取得用
	static T* GetPtr() {
		return instance;
	}
	// 生成確認
	static bool IsCreated() {
		return instance != nullptr;
	}
};

template <typename T> T* Singleton<T>::instance = nullptr;