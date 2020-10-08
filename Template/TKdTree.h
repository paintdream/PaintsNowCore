#pragma once

#include "../PaintsNow.h"
#include "../Interface/IType.h"
#include "TTagged.h"
#include <algorithm>
#include <cassert>
#include <utility>

namespace PaintsNow {
#ifndef _MSC_VER
	template <class T, class prim_type = typename T::first_type, size_t N = prim_type::size * 2>
#else
	template <class T, class prim_type = T::first_type, size_t N = prim_type::size * 2>
#endif
	struct TOverlap {
		typedef typename prim_type::type type;
		enum { size = N };

		static inline type Get(const T& v, uint16_t index) {
			const type* buffer = reinterpret_cast<const type*>(&v);
			return buffer[index];
		}

		static inline bool Compare(const T& lhs, const T& rhs, uint16_t index) {
			return Get(rhs, index) < Get(lhs, index);
		}

		static inline bool OverlapLeft(const T& lhs, const T& rhs, uint16_t index) {
			return index < size / 2 || !(Get(lhs, index) < Get(rhs, index - size / 2));
		}

		static inline bool OverlapRight(const T& lhs, const T& rhs, uint16_t index) {
			return index >= size / 2 || !(Get(rhs, index + size / 2) < Get(lhs, index));
		}
	};

	// KdTree with custom spatial structure
	template <class K, class Base = Void, class P = TOverlap<K>, size_t KEY_BITS = 3>
	class TKdTree : public Base {
	public:
		typedef uint8_t KeyType;
		typedef TKdTree Type;
		TKdTree(const K& k = K(), const KeyType i = 0) : key(k), leftNode(nullptr), rightNode(nullptr), _parentNode(nullptr) {
			const size_t KEY_MASK = (1 << KEY_BITS) - 1;
			assert(P::size <= KEY_MASK + 1);
			assert(i <= KEY_MASK);
			assert(((size_t)this & KEY_MASK) == 0); // must be aligned
			SetIndex(i);
		}

		inline void Attach(TKdTree* tree) {
			assert(tree != nullptr && tree != this);
			assert(tree->leftNode == nullptr && tree->rightNode == nullptr && tree->GetParent() == nullptr);
			CheckCycle();
			Merge(tree);
			CheckCycle();
		}

		template <class S>
		inline TKdTree* Detach(S& selector) {
			CheckCycle();
			TKdTree* newRoot = nullptr;
			if (LightDetach(newRoot)) {
				CheckCycle();
				return newRoot;
			}

			CheckCycle();
			assert(leftNode != nullptr && rightNode != nullptr);

			KeyType keyIndex = GetIndex();
			TKdTree* p = selector(leftNode, rightNode) ? rightNode->FindMinimal(keyIndex) : leftNode->FindMaximal(keyIndex);
			CheckCycle();
			assert(p != nullptr);
			newRoot = p;

			TKdTree* temp = nullptr;
			p->Detach(selector);
			assert(p->GetParent() == nullptr);
			assert(p->leftNode == nullptr && p->rightNode == nullptr);

			CheckCycle();
			if (GetParent() != nullptr) {
				TKdTree** pp = GetParent()->leftNode == this ? &GetParent()->leftNode : &GetParent()->rightNode;
				*pp = newRoot;
				newRoot = nullptr;
			}

			if (leftNode != nullptr) {
				leftNode->SetParent(p);
			}

			if (rightNode != nullptr) {
				rightNode->SetParent(p);
			}

			std::swap(links, p->links);
			p->CheckCycle();
			CheckCycle();

			return newRoot;
		}

		template <class Q>
		inline void Query(const K& targetKey, Q& queryer) {
			// ranged queryer
			KeyType keyIndex = GetIndex();
			if (queryer(targetKey, *this)) {
				CheckCycle();
				if (leftNode != nullptr && P::OverlapLeft(key, targetKey, keyIndex)) {
					leftNode->Query(targetKey, queryer);
				}
				CheckCycle();

				if (rightNode != nullptr && P::OverlapRight(key, targetKey, keyIndex)) {
					rightNode->Query(targetKey, queryer);
				}

				CheckCycle();
			}
		}

		inline const K& GetKey() const {
			return key;
		}

		inline void SetKey(const K& k) {
			key = k;
		}

		inline KeyType GetIndex() const {
			return (KeyType)_parentNode.Tag();
		}

		inline void SetIndex(KeyType t) {
			_parentNode.Tag(t);
		}

		inline TKdTree* GetParent() const {
			return _parentNode();
		}

		inline void SetParent(TKdTree* tree) {
			_parentNode(tree);
		}

		inline void CheckCycle() {
			KeyType keyIndex = GetIndex();
			assert(leftNode != this && rightNode != this);
			if (leftNode != nullptr) {
				assert(leftNode->GetParent() == this);
				assert(!P::Compare(leftNode->key, key, keyIndex));
			}

			if (rightNode != nullptr) {
				assert(rightNode->GetParent() == this);
				assert(!P::Compare(key, rightNode->key, keyIndex));
			}

			if (GetParent() != nullptr) {
				assert(GetParent()->leftNode == this || GetParent()->rightNode == this);
			}
		}

		void Validate() {
			CheckCycle();
			if (leftNode != nullptr) {
				leftNode->CheckCycle();
			}

			if (rightNode != nullptr) {
				rightNode->CheckCycle();
			}
		}

	protected:
		TKdTree* PlaceDown() {
			TKdTree* savedParent = GetParent();
			if (savedParent != nullptr) {
				assert(leftNode != nullptr || rightNode != nullptr);
				TKdTree** pp = this == savedParent->leftNode ? &savedParent->leftNode : &savedParent->rightNode;
				TKdTree* newRoot = Detach();
				assert(savedParent != GetParent());
				assert(*pp != nullptr);
				// savedParent->Validate();
				(*pp)->Attach(this);
				// savedParent->Validate();
			} else {
				TKdTree* newRoot = Detach();
				assert(newRoot != nullptr);
				newRoot->Attach(this);
				savedParent = newRoot;
			}

			return savedParent;
		}

		bool LightDetach(TKdTree*& newRoot) {
			newRoot = nullptr;
			CheckCycle();
			if (leftNode != nullptr) {
				if (rightNode != nullptr) {
					return false;
				} else {
					leftNode->SetParent(GetParent());
					newRoot = leftNode;
					leftNode = nullptr;
				}
			} else if (rightNode != nullptr) {
				rightNode->SetParent(GetParent());
				newRoot = rightNode;
				rightNode = nullptr;
			}

			if (GetParent() != nullptr) {
				CheckCycle();
				TKdTree** pp = GetParent()->leftNode == this ? &GetParent()->leftNode : &GetParent()->rightNode;
				*pp = newRoot;
				if (newRoot != nullptr) {
					newRoot->CheckCycle();
					newRoot = nullptr;
				}
				SetParent(nullptr);
			}
			CheckCycle();

			return true;
		}

		TKdTree* FindMinimal(KeyType index) {
			TKdTree* p = this;
			if (leftNode != nullptr) {
				TKdTree* compare = leftNode->FindMinimal(index);
				if (P::Compare(p->key, compare->key, index)) {
					p = compare;
				}
			}

			KeyType keyIndex = GetIndex();
			if (index != keyIndex && rightNode != nullptr) {
				TKdTree* compare = rightNode->FindMinimal(index);
				if (P::Compare(p->key, compare->key, index)) {
					p = compare;
				}
			}

			return p;
		}

		TKdTree* FindMaximal(KeyType index) {
			TKdTree* p = this;
			KeyType keyIndex = GetIndex();
			if (index != keyIndex && leftNode != nullptr) {
				TKdTree* compare = leftNode->FindMaximal(index);
				if (!P::Compare(p->key, compare->key, index)) {
					p = compare;
				}
			}

			if (rightNode != nullptr) {
				TKdTree* compare = rightNode->FindMaximal(index);
				if (!P::Compare(p->key, compare->key, index)) {
					p = compare;
				}
			}

			return p;
		}

		inline void Merge(TKdTree* tree) {
			CheckCycle();
			assert(tree->GetParent() == nullptr);
			KeyType keyIndex = GetIndex();
			bool left = P::Compare(key, tree->key, keyIndex);

			TKdTree** ptr = left ? &leftNode : &rightNode;
			if (*ptr == nullptr) {
				*ptr = tree;
				tree->SetParent(this);
			} else {
				(*ptr)->Merge(tree);
			}
		}

		K key;													// 0	: Float3Pair => 24 Bytes
		union {
			struct {
				TTagged<TKdTree*, KEY_BITS> _parentNode;		// 24	: Pointer => 4 Bytes (32bit mode), 8 Bytes (64bit mode)
				TKdTree* leftNode;								// 28(32)
				TKdTree* rightNode;								// 32(40)
			};
			struct {
				TKdTree* links[3];
			} links;
		};
	};
}
