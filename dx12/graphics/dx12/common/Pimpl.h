#ifndef _PIMPL_
#define _PIMPL_
//
//	コピー・ムーブ対応Pimplイディオム
//	参考HP : http://zakio.net/blog/2009/04/15-222117.html
//

#include <cassert>

// 不完全型を受け付けるためのインタフェース
// ポインタを void* で扱っている部分がミソ
class ImplHolderBase {
public:
	virtual ~ImplHolderBase() {}
	virtual void CopyTo(ImplHolderBase* pHolder) const = 0;
	virtual void MoveTo(ImplHolderBase* pHolder) = 0;
	virtual void* GetPtr() = 0;
	virtual const void* GetPtr() const = 0;
};

// 深いコピーができるPimplクラスの実態へのポインタを保持するクラス
template <class T, class U>
class CopyableImplHolder : public ImplHolderBase {
	T* ptr_; // これが Impl の実体
public:
	explicit CopyableImplHolder(T* ptr): ptr_(ptr) {}
	explicit CopyableImplHolder(U* pInterface, CopyableImplHolder& right):
		ptr_(new T(pInterface, *right.ptr_))
	{}
	explicit CopyableImplHolder(U* pInterface, CopyableImplHolder&& right):
		ptr_(new T(pInterface, std::move(*right.ptr_)))
	{}

	virtual ~CopyableImplHolder()
	{
		delete ptr_;
	}
	virtual void CopyTo(ImplHolderBase* pHolder) const
	{
		// ImplHolder 以外は考えられないので、static_cast で変換可能
		*(static_cast<CopyableImplHolder*>(pHolder)->ptr_) = *ptr_; // ここで T の代入演算子が呼ばれる
	}
	virtual void MoveTo(ImplHolderBase* pHolder)
	{
		CopyableImplHolder* p = static_cast< CopyableImplHolder* >(pHolder);
		*p->ptr_ = std::move(*ptr_);
	}

	virtual void* GetPtr() { return ptr_; }
	virtual const void* GetPtr() const { return ptr_; }
private: // 念のためコピーと代入を禁止しておく
	CopyableImplHolder(const CopyableImplHolder&) = delete;
	CopyableImplHolder& operator = (const CopyableImplHolder&) = delete;
	CopyableImplHolder(const CopyableImplHolder&&) = delete;
	CopyableImplHolder& operator = (const CopyableImplHolder&&) = delete;
};

// 深いコピーができるPimplクラス
template <class T, class U>
class CopyablePimpl {
	ImplHolderBase* pHolder_;
public:
	explicit CopyablePimpl(T* ptr): pHolder_(new CopyableImplHolder<T, U>(ptr)) {}
	~CopyablePimpl()
	{
		delete pHolder_;
	}
	CopyablePimpl(U* pInterface, const CopyablePimpl& init)
		: pHolder_(new CopyableImplHolder<T, U>(pInterface, *static_cast< CopyableImplHolder<T, U>* >(init.pHolder_)))
	{}
	CopyablePimpl& operator = (const CopyablePimpl& rhs)
	{
		rhs.pHolder_->CopyTo(pHolder_);
		return *this;
	}
	CopyablePimpl(U* pInterface, CopyablePimpl&& right)
		: pHolder_(new CopyableImplHolder<T, U>(pInterface, std::move(*static_cast< CopyableImplHolder<T, U>* >(right.pHolder_))))
	{}
	CopyablePimpl& operator=(CopyablePimpl&& right)
	{
		right.pHolder_->MoveTo(this->pHolder_);
		return *this;
	}
	T& operator * () { return *(static_cast<T*>(pHolder_->GetPtr())); }
	const T& operator * () const { return *(static_cast<const T*>(pHolder_->GetPtr())); }
	T* operator -> () { return &(**this); }
	const T* operator -> () const { return &(**this); }
};

//#undef interface //MSVC用

//
//	implクラスからinterfaceクラスのメンバにアクセスするときに使う
//
template<class Interface>
class ImplInterfacePointer
{
	Interface* const mpInterface;
public:
	ImplInterfacePointer(Interface*const pInterface): mpInterface(pInterface) {}
    Interface* interface_() { return this->mpInterface; }
    const Interface* interface_()const { return this->mpInterface; }

	//implクラスにデフォルトのコピー、ムーブ演算子の生成を許可するためのコード
	void operator=(const ImplInterfacePointer&) {}
	void operator=(const ImplInterfacePointer&&) {}
};

#ifdef _MSC_VER
#	undef _noexcept
#	define _noexcept throw()
#endif

//
//	PIMPLイディオムのテンプレート
//	このマクロを使用したときはPIMPL_REQUIRE_CONSTRUCTOR_DECLARATIONマクロを使用するのを推奨します
//	上記のマクロはクラスにとって必要最低限の関数を宣言してくれます
//
#define IMPL_DECLARATION(ClassName, ImplClassName, implVarName, accessorFuncName, accessorModifier)\
	class ImplClassName;														\
	CopyablePimpl<ImplClassName, ClassName> implVarName;						\
accessorModifier:																\
	ImplClassName& accessorFuncName()_noexcept{ return *implVarName; }			\
	const ImplClassName& accessorFuncName()const _noexcept{ return *implVarName; }

//
//	IMPL_DECLARATIONマクロを使用したときに必要となる関数を宣言するマクロ
//
#define PIMPL_REQUIRE_FUNCTION_DECLARATION(ClassName)\
	ClassName()_noexcept;								\
	ClassName(const ClassName& from)_noexcept;			\
	ClassName(ClassName&& from)_noexcept;				\
	ClassName& operator=(ClassName&& from)_noexcept;

//
//	implクラスのコピーコンストラクタを定義するマクロ
//	メンバのコピーはコピー演算子で対応してます
//	なので、コピー時の処理を定義したいときはコピー演算子で行ってください
//	implクラスの宣言内で使用してください
//
#define IMPL_COPY_CONSTRUCTOR_WITH_IMPL_CLASS_NAME(ClassName, ImplClassName)	\
ImplClassName(ClassName* pInterface, const ImplClassName& right)				\
	: ImplInterfacePointer(pInterface)											\
{																				\
	*this = right;																\
}

//
//	implクラスのムーブコンストラクタを定義するマクロ
//	メンバのムーブはムーブ演算子内で行ってます
//	ムーブ時の処理を定義したいときはムーブ演算子で行ってください
//	implクラスの宣言内で使用してください
//
#define IMPL_MOVE_CONSTRUCTOR_WITH_IMPL_CLASS_NAME(ClassName, ImplClassName)	\
ImplClassName(ClassName* pInterface, ImplClassName&& right)						\
	: ImplInterfacePointer(pInterface)											\
{																				\
	*this = std::move(right);													\
}

//
//	IMPLイディオムテンプレート 
//
#define PIMPL_IDIOM_TEMPLATE(ClassName)							\
private:														\
	IMPL_DECLARATION(ClassName, _impl, mpImpl, impl, private)	\
public:															\
	PIMPL_REQUIRE_FUNCTION_DECLARATION(ClassName)

//
//	PIMPL_IDIOM_TEMPLATEをつかったときのcpp側のテンプレート
//
#define PIMPL_IDIOM_CPP_TEMPLATE(ClassName)						\
ClassName::ClassName() _noexcept									\
	: mpImpl(new _impl(this))									\
{}																\
ClassName::ClassName(const ClassName& from) _noexcept :			\
	mpImpl(this, from.mpImpl)									\
{}																\
ClassName::ClassName(ClassName&& from) _noexcept :				\
	mpImpl(this, std::move(from.mpImpl))						\
{}																\
ClassName& ClassName::operator=(ClassName&& from) _noexcept {	\
	this->mpImpl = std::move(from.mpImpl);						\
	return *this;												\
}

//
//	IMPL_COPY_CONSTRUCTOR_WITH_IMPL_CLASS_NAMEマクロのImplClassNameに_implを指定したもの
//
#define IMPL_COPY_CONSTRUCTOR(ClassName) IMPL_COPY_CONSTRUCTOR_WITH_IMPL_CLASS_NAME(ClassName, _impl)

//
//	IMPL_MOVE_CONSTRUCTOR_WITH_IMPL_CLASS_NAMEマクロのImplClassNameに_implを指定したもの
//
#define IMPL_MOVE_CONSTRUCTOR(ClassName) IMPL_MOVE_CONSTRUCTOR_WITH_IMPL_CLASS_NAME(ClassName, _impl)

/*
	IMPL_DECLARATIONマクロを使用したときの使用例
	
[.h]
class Interface
{
	IMPL_DECLARATION(Interface, hoge, mImpl, impl, private);
public:
	PIMPL_REQUIRE_CONSTRUCTOR_DECLARATION(Interface);
};

[.cpp]
Interface::Interface() _noexcept
	: mImpl(new hoge(this))
{}
Interface::Interface(const Interface& from) _noexcept
	: mImpl(this, from.mImpl)
{}
Interface::Interface(Interface&& from) _noexcept
	: mImpl(this, std::move(from.mImpl))
{}
Interface& Interface::operator=(Interface&& from) _noexcept{
	this->mImpl = std::move(from.mImpl);
	return *this;
}

*/

/*

	PIMPL_IDIOM_TEMPLATEマクロ関連の使用例

[.h]
	class Interface
	{
		PIMPL_IDIOM_TEMPLATE(Interface);
	public:
	};

[.cpp]
class Interface::_impl : private ImplInterfacePointer<Interface>
{
public:
	IMPL_COPY_CONSTRUCTOR(Interface);
	IMPL_MOVE_CONSTRUCTOR(Interface);

	_impl(Interface* pInterface)
		: ImplInterfacePointer(pInterface)
	{}
};
PIMPL_IDIOM_CPP_TEMPLATE(Interface)

*/

// ムーブ可能Pimplクラス用のポインタを保持するクラス
template <class T, class U>
class MoveableImplHolder : public ImplHolderBase {
    T* ptr_; // これが Impl の実体
public:
    explicit MoveableImplHolder(T* ptr): ptr_(ptr) {}
    explicit MoveableImplHolder(U* pInterface, MoveableImplHolder&& right):
        ptr_(new T(pInterface, std::move(*right.ptr_)))
    {}

    virtual ~MoveableImplHolder()
    {
        delete ptr_;
    }
    virtual void CopyTo(ImplHolderBase*) const {
        assert(false && "do not copy");
    }
    virtual void MoveTo(ImplHolderBase* pHolder)
    {
        auto* p = static_cast< MoveableImplHolder* >(pHolder);
        *p->ptr_ = std::move(*ptr_);
    }

    virtual void* GetPtr() { return ptr_; }
    virtual const void* GetPtr() const { return ptr_; }
private: // 念のためコピーと代入を禁止しておく
    MoveableImplHolder(const MoveableImplHolder&) = delete;
    MoveableImplHolder& operator = (const MoveableImplHolder&) = delete;
    MoveableImplHolder(const MoveableImplHolder&&) = delete;
    MoveableImplHolder& operator = (const MoveableImplHolder&&) = delete;
};

// ムーブ可能Pimplクラス
template <class T, class U>
class MoveablePimpl {
    ImplHolderBase* pHolder_;
public:
    explicit MoveablePimpl(T* ptr): pHolder_(new MoveableImplHolder<T, U>(ptr)) {}
    ~MoveablePimpl()
    {
        delete pHolder_;
    }
    MoveablePimpl& operator = (const MoveablePimpl& rhs)
    {
        rhs.pHolder_->CopyTo(pHolder_);
        return *this;
    }
    MoveablePimpl(U* pInterface, MoveablePimpl&& right)
        : pHolder_(new MoveableImplHolder<T, U>(pInterface, std::move(*static_cast< MoveableImplHolder<T, U>* >(right.pHolder_))))
    {}
    MoveablePimpl& operator=(MoveablePimpl&& right)
    {
        right.pHolder_->MoveTo(this->pHolder_);
        return *this;
    }
    T& operator * () { return *(static_cast<T*>(pHolder_->GetPtr())); }
    const T& operator * () const { return *(static_cast<const T*>(pHolder_->GetPtr())); }
    T* operator -> () { return &(**this); }
    const T* operator -> () const { return &(**this); }
};

//
//	コピー不可なPIMPLイディオムのテンプレート
//	このマクロを使用したときはUNCOPYABLE_PIMPL_REQUIRE_FUNCTION_DECLARATIONマクロを使用するのを推奨します
//	上記のマクロはクラスにとって必要最低限の関数を宣言してくれます
//
#define UNCOPYABLE_IMPL_DECLARATION(ClassName, ImplClassName, implVarName, accessorFuncName, accessorModifier)\
    class ImplClassName;														\
    MoveablePimpl<ImplClassName, ClassName> implVarName;						\
accessorModifier:																\
    ImplClassName& accessorFuncName()_noexcept{ return *implVarName; }			\
    const ImplClassName& accessorFuncName()const _noexcept{ return *implVarName; }

//
//	UNCOPYABLE_IMPL_DECLARATIONマクロを使用したときに必要となる関数を宣言するマクロ
//
#define UNCOPYABLE_PIMPL_REQUIRE_FUNCTION_DECLARATION(ClassName)\
    ClassName()_noexcept;								\
    ClassName(ClassName&& from)_noexcept;				\
    ClassName& operator=(ClassName&& from)_noexcept;     \
    ClassName(const ClassName&)= delete;                \
    ClassName& operator=(ClassName&)=delete;

//
//	implクラスのムーブコンストラクタを定義するマクロ
//	メンバのムーブはムーブ演算子内で行ってます
//	ムーブ時の処理を定義したいときはムーブ演算子で行ってください
//	implクラスの宣言内で使用してください
//
#define UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR_WITH_IMPL_CLASS_NAME(ClassName, ImplClassName)	\
ImplClassName(ClassName* pInterface, ImplClassName&& right)						\
    : ImplInterfacePointer(pInterface)											\
{																				\
    *this = std::move(right);													\
}

//
//	コピー不可なIMPLイディオムテンプレート
//
#undef UNCOPYABLE_PIMPL_IDIOM_TEMPLATE
#define UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(ClassName)							\
private:                                                                    \
    UNCOPYABLE_IMPL_DECLARATION(ClassName, _impl, mpImpl, impl, private)	\
public:                                                                     \
    UNCOPYABLE_PIMPL_REQUIRE_FUNCTION_DECLARATION(ClassName)

//
//	UNCOPYABLE_PIMPL_IDIOM_TEMPLATEをつかったときのcpp側のテンプレート
//
#define UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(ClassName)			\
ClassName::ClassName() _noexcept									\
    : mpImpl(new _impl(this))									\
{}																\
ClassName::ClassName(ClassName&& from) _noexcept :				\
    mpImpl(this, std::move(from.mpImpl))						\
{}																\
ClassName& ClassName::operator=(ClassName&& from) _noexcept {	\
    this->mpImpl = std::move(from.mpImpl);						\
    return *this;												\
}

//
//	IMPL_MOVE_CONSTRUCTOR_WITH_IMPL_CLASS_NAMEマクロのImplClassNameに_implを指定したもの
//	このマクロを使用し場合は必ず、ムーブ演算子を定義してください
//
#define UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(ClassName) UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR_WITH_IMPL_CLASS_NAME(ClassName, _impl)

/*
    UNCOPYABLE_IMPL_DECLARATIONマクロを使用したときの使用例

[.h]
class Interface
{
    UNCOPYABLE_IMPL_DECLARATION(Interface, hoge, mImpl, impl, private);
public:
    UNCOPYABLE_PIMPL_REQUIRE_FUNCTION_DECLARATION(Interface);
};

[.cpp]
Interface::Interface() _noexcept
    : mImpl(new hoge(this))
{}
Interface::Interface(Interface&& from) _noexcept
    : mImpl(this, std::move(from.mImpl))
{}
Interface& Interface::operator=(Interface&& from) _noexcept{
    this->mImpl = std::move(from.mImpl);
    return *this;
}

*/

/*

    UNCOPYABLE_PIMPL_IDIOM_TEMPLATEマクロ関連の使用例

[.h]
    class Interface
    {
        UNCOPYABLE_PIMPL_IDIOM_TEMPLATE(Interface);
    public:
    };

[.cpp]
    class Interface::_impl : private ImplInterfacePointer<Interface>
    {
    public:
        UNCOPYABLE_IMPL_MOVE_CONSTRUCTOR(Interface);

        _impl(Interface* pInterface)
            : ImplInterfacePointer(pInterface)
        {}
    };
    UNCOPYABLE_PIMPL_IDIOM_CPP_TEMPLATE(Interface)

*/

#endif
