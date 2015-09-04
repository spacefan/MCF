// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_STATIC_VECTOR_HPP_
#define MCF_CONTAINERS_STATIC_VECTOR_HPP_

#include "_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <memory>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT, std::size_t kCapacity>
class StaticVector {
	static_assert(kCapacity > 0, "A StaticVector shall have a non-zero capacity.");

private:
	alignas(ElementT) char x_aStorage[kCapacity][sizeof(ElementT)];
	std::size_t x_uSize;

public:
	StaticVector() noexcept
		: x_uSize(0)
	{
	}
	template<typename ...ParamsT>
	explicit StaticVector(std::size_t uSize, const ParamsT &...vParams)
		: StaticVector()
	{
		Append(uSize, vParams...);
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	StaticVector(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
		: StaticVector()
	{
		Append(itBegin, itEnd);
	}
	StaticVector(std::initializer_list<ElementT> rhs)
		: StaticVector()
	{
		Append(rhs.begin(), rhs.end());
	}
	StaticVector(const StaticVector &rhs)
		: StaticVector()
	{
		Append(rhs.GetBegin(), rhs.GetEnd());
	}
	StaticVector(StaticVector &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value)
		: StaticVector()
	{
		Append(std::make_move_iterator(rhs.GetBegin()), std::make_move_iterator(rhs.GetEnd()));
	}
	StaticVector &operator=(const StaticVector &rhs){
		Clear();
		Append(rhs.GetBegin(), rhs.GetEnd());
		return *this;
	}
	StaticVector &operator=(StaticVector &&rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
		Clear();
		Append(std::make_move_iterator(rhs.GetBegin()), std::make_move_iterator(rhs.GetEnd()));
		return *this;
	}
	~StaticVector(){
		Clear();
	}

public:
	// 容器需求。
	using ElementType = ElementT;

	struct AdvanceOnce {
		void operator()(const StaticVector &v, const ElementType *&p) noexcept {
			if(p + 1 == v.GetEnd()){
				p = nullptr;
			} else {
				++p;
			}
		}
		void operator()(StaticVector &v, ElementType *&p) noexcept {
			if(p + 1 == v.GetEnd()){
				p = nullptr;
			} else {
				++p;
			}
		}
	};
	struct RetreatOnce {
		void operator()(const StaticVector &v, const ElementType *&p) noexcept {
			if(p == v.GetBegin()){
				p = nullptr;
			} else {
				--p;
			}
		}
		void operator()(StaticVector &v, ElementType *&p) noexcept {
			if(p == v.GetBegin()){
				p = nullptr;
			} else {
				--p;
			}
		}
	};

	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <StaticVector, AdvanceOnce, RetreatOnce>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <StaticVector, AdvanceOnce, RetreatOnce>;

	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	void Clear() noexcept {
		Pop(x_uSize);
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		const auto pBegin = GetBegin();
		if(pBegin == GetEnd()){
			return ConstEnumerator(*this, nullptr);
		} else {
			return ConstEnumerator(*this, pBegin);
		}
	}
	Enumerator EnumerateFirst() noexcept {
		auto pBegin = GetBegin();
		if(pBegin == GetEnd()){
			return Enumerator(*this, nullptr);
		} else {
			return Enumerator(*this, pBegin);
		}
	}

	ConstEnumerator EnumerateLast() const noexcept {
		const auto pEnd = GetEnd();
		if(GetBegin() == pEnd){
			return ConstEnumerator(*this, nullptr);
		} else {
			return ConstEnumerator(*this, pEnd - 1);
		}
	}
	Enumerator EnumerateLast() noexcept {
		auto pEnd = GetEnd();
		if(GetBegin() == pEnd){
			return Enumerator(*this, nullptr);
		} else {
			return Enumerator(*this, pEnd - 1);
		}
	}

	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}

	void Swap(StaticVector &rhs) noexcept(std::is_nothrow_move_constructible<ElementT>::value) {
		auto vTemp = std::move(rhs);
		*this = std::move(rhs);
		rhs = std::move(vTemp);
	}

	// StaticVector 需求。
	const ElementT *GetData() const noexcept {
		return static_cast<const ElementT *>(static_cast<const void *>(x_aStorage));
	}
	ElementT *GetData() noexcept {
		return static_cast<ElementT *>(static_cast<void *>(x_aStorage));
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	static constexpr std::size_t GetCapacity() noexcept {
		return kCapacity;
	}

	const ElementT *GetBegin() const noexcept {
		return GetData();
	}
	ElementT *GetBegin() noexcept {
		return GetData();
	}
	const ElementT *GetEnd() const noexcept {
		return GetData() + x_uSize;
	}
	ElementT *GetEnd() noexcept {
		return GetData() + x_uSize;
	}

	const ElementT &Get(std::size_t uIndex) const {
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, __PRETTY_FUNCTION__);
		}
		return GetData()[uIndex];
	}
	ElementT &Get(std::size_t uIndex){
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, __PRETTY_FUNCTION__);
		}
		return GetData()[uIndex];
	}
	const ElementT &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}
	ElementT &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < x_uSize);

		return GetData()[uIndex];
	}

	template<typename ...ParamsT>
	void Resize(std::size_t uSize, ParamsT &&...vParams){
		if(uSize > x_uSize){
			Append(uSize - x_uSize, std::forward<ParamsT>(vParams)...);
		} else {
			Pop(x_uSize - uSize);
		}
	}
	template<typename ...ParamsT>
	ElementT *ResizeMore(std::size_t uDeltaSize, ParamsT &&...vParams){
		const auto uOldSize = x_uSize;
		Append(uDeltaSize - x_uSize, std::forward<ParamsT>(vParams)...);
		return GetData() + uOldSize;
	}

	void Reserve(std::size_t uNewCapacity){
		if(uNewCapacity > GetCapacity()){
			DEBUG_THROW(Exception, ERROR_OUTOFMEMORY, __PRETTY_FUNCTION__);
		}
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = x_uSize;
		const auto uNewCapacity = uOldSize + uDeltaCapacity;
		if(uNewCapacity < uOldSize){
			throw std::bad_alloc();
		}
		Reserve(uNewCapacity);
	}

	template<typename ...ParamsT>
	void Push(ParamsT &&...vParams){
		ReserveMore(1);
		UncheckedPush(std::forward<ParamsT>(vParams)...);
	}
	template<typename ...ParamsT>
	void UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ElementT, ParamsT &&...>::value) {
		ASSERT(GetCapacity() - x_uSize > 0);

		DefaultConstruct(GetData() + x_uSize, std::forward<ParamsT>(vParams)...);
		++x_uSize;
	}
	void Pop(std::size_t uCount = 1) noexcept {
		ASSERT(uCount <= x_uSize);

		for(std::size_t i = 0; i < uCount; ++i){
			Destruct(GetData() + x_uSize - i - 1);
		}
		x_uSize -= uCount;
	}

	template<typename ...ParamsT>
	void Append(std::size_t uSize, const ParamsT &...vParams){
		const auto uOldSize = x_uSize;
		ReserveMore(uSize);
		try {
			for(std::size_t i = 0; i < uSize; ++i){
				UncheckedPush(vParams...);
			}
		} catch(...){
			Pop(x_uSize - uOldSize);
			throw;
		}
	}
	template<typename IteratorT, std::enable_if_t<
		sizeof(typename std::iterator_traits<IteratorT>::value_type *),
		int> = 0>
	void Append(IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
		constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;
		const auto uOldSize = x_uSize;
		if(kHasDeltaSizeHint){
			ReserveMore(static_cast<std::size_t>(std::distance(itBegin, itEnd)));
		}
		try {
			if(kHasDeltaSizeHint){
				for(auto it = itBegin; it != itEnd; ++it){
					UncheckedPush(*it);
				}
			} else {
				for(auto it = itBegin; it != itEnd; ++it){
					Push(*it);
				}
			}
		} catch(...){
			Pop(x_uSize - uOldSize);
			throw;
		}
	}

	const ElementT &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	ElementT &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}
};

template<typename ElementT, std::size_t kCapacity>
void swap(StaticVector<ElementT, kCapacity> &lhs, StaticVector<ElementT, kCapacity> &rhs) noexcept(noexcept(lhs.Swap(rhs))) {
	lhs.Swap(rhs);
}

template<typename ElementT, std::size_t kCapacity>
decltype(auto) begin(const StaticVector<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT, std::size_t kCapacity>
decltype(auto) begin(StaticVector<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateFirst();
}

template<typename ElementT, std::size_t kCapacity>
decltype(auto) end(const StaticVector<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT, std::size_t kCapacity>
decltype(auto) end(StaticVector<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateSingular();
}

}

#endif