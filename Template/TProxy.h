// TProxy.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//
#ifndef __TPROXY_H__
#define __TPROXY_H__

#include "../PaintsNow.h"

// Remarks: If you use C++ 11, all features provided in this file can be well replaced by std::function<(...)>.
// But in order to provide the same interface for both vc6 or other vs (which do not support std::function()), I wrote TProxy/TWrapper for them.

#include <cassert>
#include <cstring>
#include <set>
#include <utility>

namespace PaintsNow {
	class IHost; // Just a type monitor, no defination.
	// Any pointer to member function of a multi-Interfaced class will takes 2 * sizeof(void*) size.
	// However, when it comes to single-Interfaced class or native class, MSVC compiler need only half storage of it.
	// So we must create Void in forced multi-Interfaced way to prevent possible memory corruption in TWrapper<> layout.

	template <class T>
	struct ReturnType {
		typedef T type;
	};

	template <>
	struct ReturnType<void> {
		typedef Void type;
	};

#if defined(_MSC_VER) && _MSC_VER < 1800 || defined(COMPATIBLE_PROXY)

	struct void_type {};
	struct not_void_type {};
	template <class T>
	struct count_void_type { enum { value = 1 }; };
	template <>
	struct count_void_type<Void> { enum { value = 0 }; };

	template <class R = Void, class A = Void, class B = Void, class C = Void, class D = Void, class E = Void, class F = Void, class G = Void, class H = Void, class I = Void, class J = Void, class K = Void, class L = Void, class M = Void, class N = Void, class O = Void, class P = Void>
	class TProxy {
	public:
		static inline size_t GetCount() {
			return count_void_type<A>::value + count_void_type<B>::value + count_void_type<C>::value + count_void_type<D>::value
				+ count_void_type<E>::value + count_void_type<F>::value + count_void_type<G>::value + count_void_type<H>::value
				+ count_void_type<I>::value + count_void_type<J>::value + count_void_type<K>::value + count_void_type<L>::value
				+ count_void_type<M>::value + count_void_type<N>::value + count_void_type<O>::value + count_void_type<P>::value;
		}
	};

	template <class ManageType, class T, class R = Void, class A = Void, class B = Void, class C = Void, class D = Void, class E = Void, class F = Void, class G = Void, class H = Void, class I = Void, class J = Void, class K = Void, class L = Void, class M = Void, class N = Void, class O = Void, class P = Void>
	class TMethod : public TProxy<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> {
	public:
		typedef R(T::*ZZ)();
		typedef R(T::*ZA)(A a);
		typedef R(T::*ZB)(A a, B b);
		typedef R(T::*ZC)(A a, B b, C c);
		typedef R(T::*ZD)(A a, B b, C c, D d);
		typedef R(T::*ZE)(A a, B b, C c, D d, E e);
		typedef R(T::*ZF)(A a, B b, C c, D d, E e, F f);
		typedef R(T::*ZG)(A a, B b, C c, D d, E e, F f, G g);
		typedef R(T::*ZH)(A a, B b, C c, D d, E e, F f, G g, H h);
		typedef R(T::*ZI)(A a, B b, C c, D d, E e, F f, G g, H h, I i);
		typedef R(T::*ZJ)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j);
		typedef R(T::*ZK)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k);
		typedef R(T::*ZL)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l);
		typedef R(T::*ZM)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m);
		typedef R(T::*ZN)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n);
		typedef R(T::*ZO)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o);
		typedef R(T::*ZP)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p);

		struct Table {
			union {
				void* invoker;
				typename ReturnType<R>::type(*invokerZ)(const TMethod* t);
				typename ReturnType<R>::type(*invokerA)(const TMethod* t, A a);
				typename ReturnType<R>::type(*invokerB)(const TMethod* t, A a, B b);
				typename ReturnType<R>::type(*invokerC)(const TMethod* t, A a, B b, C c);
				typename ReturnType<R>::type(*invokerD)(const TMethod* t, A a, B b, C c, D d);
				typename ReturnType<R>::type(*invokerE)(const TMethod* t, A a, B b, C c, D d, E e);
				typename ReturnType<R>::type(*invokerF)(const TMethod* t, A a, B b, C c, D d, E e, F f);
				typename ReturnType<R>::type(*invokerG)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g);
				typename ReturnType<R>::type(*invokerH)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h);
				typename ReturnType<R>::type(*invokerI)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i);
				typename ReturnType<R>::type(*invokerJ)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j);
				typename ReturnType<R>::type(*invokerK)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k);
				typename ReturnType<R>::type(*invokerL)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l);
				typename ReturnType<R>::type(*invokerM)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m);
				typename ReturnType<R>::type(*invokerN)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n);
				typename ReturnType<R>::type(*invokerO)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o);
				typename ReturnType<R>::type(*invokerP)(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p);
			};
			void(*duplicator)(TMethod& output, const TMethod& input);
			void(*destructor)(TMethod& input);
		};

		static void CopyImpl(TMethod& output, const TMethod& input) {
			output.~TMethod();
			output.tablePointer = input.tablePointer;
			output.host = input.host;
			output.p = input.p;
		}

		static void CopyImplManaged(TMethod& output, const TMethod& input) {
			output.~TMethod();
			output.tablePointer = input.tablePointer;
			output.host = reinterpret_cast<IHost*>(new T(*reinterpret_cast<T*>(input.host)));
			output.p = input.p;
		}

		static void DestroyImplManaged(TMethod& input) {
			if (input.host != nullptr) {
				delete reinterpret_cast<T*>(input.host);
			}
		}

		static void DestroyImpl(TMethod& input) {}

		template <class T>
		static Table CreateTable(void* invoker, T) {
			Table tab;
			tab.invoker = invoker;
			tab.duplicator = &TMethod::CopyImpl;
			tab.destructor = &TMethod::DestroyImpl;
			return tab;
		}

		template <>
		static Table CreateTable(void* invoker, std::true_type) {
			Table tab;
			tab.invoker = invoker;
			tab.duplicator = &TMethod::CopyImplManaged;
			tab.destructor = &TMethod::DestroyImplManaged;
			return tab;
		}

		~TMethod() {
			tablePointer->destructor(*this);
		}

		TMethod(const TMethod& rhs) {
			static Table tab = CreateTable(&TMethod::InvokerZ, ManageType());
			tablePointer = &tab;
			*this = rhs;
		}

		TMethod& operator = (const TMethod& rhs) {
			rhs.tablePointer->duplicator(*this, rhs);
			return *this;
		}

		TMethod() : host(nullptr), p(nullptr) {
			static Table tab = CreateTable(&TMethod::InvokerZ, ManageType());
			tablePointer = &tab;
		}

		TMethod(T* ptr, ZZ func) : host((IHost*)ptr), p(func) {
			static Table tab = CreateTable(&TMethod::InvokerZ, ManageType());
			tablePointer = &tab;
		}

		TMethod(T* ptr, ZA func) : host((IHost*)ptr), pa(func) {
			static Table tab = CreateTable(&TMethod::InvokerA, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZB func) : host((IHost*)ptr), pb(func) {
			static Table tab = CreateTable(&TMethod::InvokerB, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZC func) : host((IHost*)ptr), pc(func) {
			static Table tab = CreateTable(&TMethod::InvokerC, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZD func) : host((IHost*)ptr), pd(func) {
			static Table tab = CreateTable(&TMethod::InvokerD, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZE func) : host((IHost*)ptr), pe(func) {
			static Table tab = CreateTable(&TMethod::InvokerE, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZF func) : host((IHost*)ptr), pf(func) {
			static Table tab = CreateTable(&TMethod::InvokerF, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZG func) : host((IHost*)ptr), pg(func) {
			static Table tab = CreateTable(&TMethod::InvokerG, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZH func) : host((IHost*)ptr), ph(func) {
			static Table tab = CreateTable(&TMethod::InvokerH, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZI func) : host((IHost*)ptr), pi(func) {
			static Table tab = CreateTable(&TMethod::InvokerI, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZJ func) : host((IHost*)ptr), pj(func) {
			static Table tab = CreateTable(&TMethod::InvokerJ, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZK func) : host((IHost*)ptr), pk(func) {
			static Table tab = CreateTable(&TMethod::InvokerK, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZL func) : host((IHost*)ptr), pl(func) {
			static Table tab = CreateTable(&TMethod::InvokerL, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZM func) : host((IHost*)ptr), pm(func) {
			static Table tab = CreateTable(&TMethod::InvokerM, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZN func) : host((IHost*)ptr), pn(func) {
			static Table tab = CreateTable(&TMethod::InvokerN, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZO func) : host((IHost*)ptr), po(func) {
			static Table tab = CreateTable(&TMethod::InvokerO, ManageType());
			tablePointer = &tab;
		}
		TMethod(T* ptr, ZP func) : host((IHost*)ptr), pp(func) {
			static Table tab = CreateTable(&TMethod::InvokerP, ManageType());
			tablePointer = &tab;
		}

		template <class Z>
		struct Dispatch {
			IHost* host;
			union {
				ZZ p;
				ZA pa;
				ZB pb;
				ZC pc;
				ZD pd;
				ZE pe;
				ZF pf;
				ZG pg;
				ZH ph;
				ZI pi;
				ZJ pj;
				ZK pk;
				ZL pl;
				ZM pm;
				ZN pn;
				ZO po;
				ZP pp;
			};

			Dispatch(IHost* h, ZZ f) : host(h), p(f) {}
			inline Z Invoke() const {
				return ((reinterpret_cast<T*>(host))->*p)();
			}

			inline Z Invoke(A a) const {
				return ((reinterpret_cast<T*>(host))->*pa)(a);
			}

			inline Z Invoke(A a, B b) const {
				return ((reinterpret_cast<T*>(host))->*pb)(a, b);
			}

			inline Z Invoke(A a, B b, C c) const {
				return ((reinterpret_cast<T*>(host))->*pc)(a, b, c);
			}

			inline Z Invoke(A a, B b, C c, D d) const {
				return ((reinterpret_cast<T*>(host))->*pd)(a, b, c, d);
			}

			inline Z Invoke(A a, B b, C c, D d, E e) const {
				return ((reinterpret_cast<T*>(host))->*pe)(a, b, c, d, e);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f) const {
				return ((reinterpret_cast<T*>(host))->*pf)(a, b, c, d, e, f);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g) const {
				return ((reinterpret_cast<T*>(host))->*pg)(a, b, c, d, e, f, g);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h) const {
				return ((reinterpret_cast<T*>(host))->*ph)(a, b, c, d, e, f, g, h);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i) const {
				return ((reinterpret_cast<T*>(host))->*pi)(a, b, c, d, e, f, g, h, i);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const {
				return ((reinterpret_cast<T*>(host))->*pj)(a, b, c, d, e, f, g, h, i, j);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const {
				return ((reinterpret_cast<T*>(host))->*pk)(a, b, c, d, e, f, g, h, i, j, k);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const {
				return ((reinterpret_cast<T*>(host))->*pl)(a, b, c, d, e, f, g, h, i, j, k, l);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const {
				return ((reinterpret_cast<T*>(host))->*pm)(a, b, c, d, e, f, g, h, i, j, k, l, m);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const {
				return ((reinterpret_cast<T*>(host))->*pn)(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const {
				return ((reinterpret_cast<T*>(host))->*po)(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const {
				return ((reinterpret_cast<T*>(host))->*pp)(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
			}
		};

		template <>
		struct Dispatch<void> {
			IHost* host;
			union {
				ZZ p;
				ZA pa;
				ZB pb;
				ZC pc;
				ZD pd;
				ZE pe;
				ZF pf;
				ZG pg;
				ZH ph;
				ZI pi;
				ZJ pj;
				ZK pk;
				ZL pl;
				ZM pm;
				ZN pn;
				ZO po;
				ZP pp;
			};

			Dispatch<void>(IHost* h, ZZ f) : host(h), p(f) {}

			inline Void Invoke() const {
				((reinterpret_cast<T*>(host))->*p)();
				return Void();
			}

			inline Void Invoke(A a) const {
				((reinterpret_cast<T*>(host))->*pa)(a);
				return Void();
			}

			inline Void Invoke(A a, B b) const {
				((reinterpret_cast<T*>(host))->*pb)(a, b);
				return Void();
			}

			inline Void Invoke(A a, B b, C c) const {
				((reinterpret_cast<T*>(host))->*pc)(a, b, c);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d) const {
				((reinterpret_cast<T*>(host))->*pd)(a, b, c, d);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e) const {
				((reinterpret_cast<T*>(host))->*pe)(a, b, c, d, e);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f) const {
				((reinterpret_cast<T*>(host))->*pf)(a, b, c, d, e, f);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g) const {
				((reinterpret_cast<T*>(host))->*pg)(a, b, c, d, e, f, g);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h) const {
				((reinterpret_cast<T*>(host))->*ph)(a, b, c, d, e, f, g, h);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i) const {
				((reinterpret_cast<T*>(host))->*pi)(a, b, c, d, e, f, g, h, i);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const {
				((reinterpret_cast<T*>(host))->*pj)(a, b, c, d, e, f, g, h, i, j);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const {
				((reinterpret_cast<T*>(host))->*pk)(a, b, c, d, e, f, g, h, i, j, k);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const {
				((reinterpret_cast<T*>(host))->*pl)(a, b, c, d, e, f, g, h, i, j, k, l);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const {
				((reinterpret_cast<T*>(host))->*pm)(a, b, c, d, e, f, g, h, i, j, k, l, m);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const {
				((reinterpret_cast<T*>(host))->*pn)(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const {
				((reinterpret_cast<T*>(host))->*po)(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const {
				((reinterpret_cast<T*>(host))->*pp)(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
				return Void();
			}
		};

		static typename ReturnType<R>::type InvokerZ(const TMethod* t) {
			assert(GetCount() == 0);
			return Dispatch<R>(t->host, t->p).Invoke();
		}

		static typename ReturnType<R>::type InvokerA(const TMethod* t, A a) {
			assert(GetCount() == 1);
			return Dispatch<R>(t->host, t->p).Invoke(a);
		}

		static typename ReturnType<R>::type InvokerB(const TMethod* t, A a, B b) {
			assert(GetCount() == 2);
			return Dispatch<R>(t->host, t->p).Invoke(a, b);
		}

		static typename ReturnType<R>::type InvokerC(const TMethod* t, A a, B b, C c) {
			assert(GetCount() == 3);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c);
		}

		static typename ReturnType<R>::type InvokerD(const TMethod* t, A a, B b, C c, D d) {
			assert(GetCount() == 4);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d);
		}

		static typename ReturnType<R>::type InvokerE(const TMethod* t, A a, B b, C c, D d, E e) {
			assert(GetCount() == 5);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e);
		}

		static typename ReturnType<R>::type InvokerF(const TMethod* t, A a, B b, C c, D d, E e, F f) {
			assert(GetCount() == 6);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f);
		}

		static typename ReturnType<R>::type InvokerG(const TMethod* t, A a, B b, C c, D d, E e, F f, G g) {
			assert(GetCount() == 7);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g);
		}

		static typename ReturnType<R>::type InvokerH(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h) {
			assert(GetCount() == 8);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h);
		}

		static typename ReturnType<R>::type InvokerI(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i) {
			assert(GetCount() == 9);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i);
		}

		static typename ReturnType<R>::type InvokerJ(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) {
			assert(GetCount() == 10);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j);
		}

		static typename ReturnType<R>::type InvokerK(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) {
			assert(GetCount() == 11);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k);
		}

		static typename ReturnType<R>::type InvokerL(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) {
			assert(GetCount() == 12);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l);
		}

		static typename ReturnType<R>::type InvokerM(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) {
			assert(GetCount() == 13);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		static typename ReturnType<R>::type InvokerN(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) {
			assert(GetCount() == 14);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}

		static typename ReturnType<R>::type InvokerO(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) {
			assert(GetCount() == 15);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
		}

		static typename ReturnType<R>::type InvokerP(const TMethod* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) {
			assert(GetCount() == 16);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
		}

		typename ReturnType<R>::type operator () () const {
			return tablePointer->invokerZ(this);
		}

		typename ReturnType<R>::type operator () (A a) const {
			return tablePointer->invokerA(this, a);
		}

		typename ReturnType<R>::type operator () (A a, B b) const {
			return tablePointer->invokerB(this, a, b);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c) const {
			return tablePointer->invokerC(this, a, b, c);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d) const {
			return tablePointer->invokerD(this, a, b, c, d);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e) const {
			return tablePointer->invokerE(this, a, b, c, d, e);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f) const {
			return tablePointer->invokerF(this, a, b, c, d, e, f);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g) const {
			return tablePointer->invokerG(this, a, b, c, d, e, f, g);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h) const {
			return tablePointer->invokerH(this, a, b, c, d, e, f, g, h);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i) const {
			return tablePointer->invokerI(this, a, b, c, d, e, f, g, h, i);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const {
			return tablePointer->invokerJ(this, a, b, c, d, e, f, g, h, i, j);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const {
			return tablePointer->invokerK(this, a, b, c, d, e, f, g, h, i, j, k);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const {
			return tablePointer->invokerL(this, a, b, c, d, e, f, g, h, i, j, k, l);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const {
			return tablePointer->invokerM(this, a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const {
			return tablePointer->invokerN(this, a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const {
			return tablePointer->invokerO(this, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const {
			return tablePointer->invokerP(this, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
		}

		Table* tablePointer;
		IHost* host;
		union {
			ZZ p;
			ZA pa;
			ZB pb;
			ZC pc;
			ZD pd;
			ZE pe;
			ZF pf;
			ZG pg;
			ZH ph;
			ZI pi;
			ZJ pj;
			ZK pk;
			ZL pl;
			ZM pm;
			ZN pn;
			ZO po;
			ZP pp;
		};
	};

	template <class R = Void, class A = Void, class B = Void, class C = Void, class D = Void, class E = Void, class F = Void, class G = Void, class H = Void, class I = Void, class J = Void, class K = Void, class L = Void, class M = Void, class N = Void, class O = Void, class P = Void>
	class TFunction : public TProxy<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> {
	public:
		typedef R(*ZZ)();
		typedef R(*ZA)(A a);
		typedef R(*ZB)(A a, B b);
		typedef R(*ZC)(A a, B b, C c);
		typedef R(*ZD)(A a, B b, C c, D d);
		typedef R(*ZE)(A a, B b, C c, D d, E e);
		typedef R(*ZF)(A a, B b, C c, D d, E e, F f);
		typedef R(*ZG)(A a, B b, C c, D d, E e, F f, G g);
		typedef R(*ZH)(A a, B b, C c, D d, E e, F f, G g, H h);
		typedef R(*ZI)(A a, B b, C c, D d, E e, F f, G g, H h, I i);
		typedef R(*ZJ)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j);
		typedef R(*ZK)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k);
		typedef R(*ZL)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l);
		typedef R(*ZM)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m);
		typedef R(*ZN)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n);
		typedef R(*ZO)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o);
		typedef R(*ZP)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p);

		struct Table {
			union {
				void* invoker;

				typename ReturnType<R>::type(*invokerZ)(const TFunction* t);
				typename ReturnType<R>::type(*invokerA)(const TFunction* t, A a);
				typename ReturnType<R>::type(*invokerB)(const TFunction* t, A a, B b);
				typename ReturnType<R>::type(*invokerC)(const TFunction* t, A a, B b, C c);
				typename ReturnType<R>::type(*invokerD)(const TFunction* t, A a, B b, C c, D d);
				typename ReturnType<R>::type(*invokerE)(const TFunction* t, A a, B b, C c, D d, E e);
				typename ReturnType<R>::type(*invokerF)(const TFunction* t, A a, B b, C c, D d, E e, F f);
				typename ReturnType<R>::type(*invokerG)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g);
				typename ReturnType<R>::type(*invokerH)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h);
				typename ReturnType<R>::type(*invokerI)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i);
				typename ReturnType<R>::type(*invokerJ)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j);
				typename ReturnType<R>::type(*invokerK)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k);
				typename ReturnType<R>::type(*invokerL)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l);
				typename ReturnType<R>::type(*invokerM)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m);
				typename ReturnType<R>::type(*invokerN)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n);
				typename ReturnType<R>::type(*invokerO)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o);
				typename ReturnType<R>::type(*invokerP)(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p);
			};

			void(*duplicator)(TFunction& output, const TFunction& input);
			void(*destructor)(TFunction& input);
		};

		static void CopyImpl(TFunction& output, const TFunction& input) {
			output.~TFunction();
			output.tablePointer = input.tablePointer;
			output.host = input.host;
			output.p = input.p;
		}

		static void DestroyImpl(TFunction& input) {}
		static Table CreateTable(void* invoker) {
			Table tab;
			tab.invoker = invoker;
			tab.duplicator = &TFunction::CopyImpl;
			tab.destructor = &TFunction::DestroyImpl;
			return tab;
		}

		~TFunction() {
			tablePointer->destructor(*this);
		}

		TFunction(const TFunction& rhs) {
			static Table tab = CreateTable(&TFunction::InvokerZ);
			tablePointer = &tab;

			*this = rhs;
		}

		TFunction& operator = (const TFunction& rhs) {
			rhs.tablePointer->duplicator(*this, rhs);
			return *this;
		}

		TFunction() : host(nullptr), p(nullptr) {
			static Table tab = CreateTable(&TFunction::InvokerZ);
			tablePointer = &tab;
		}
		TFunction(ZZ func) : host(nullptr), p(func) {
			static Table tab = CreateTable(&TFunction::InvokerZ);
			tablePointer = &tab;
		}
		TFunction(ZA func) : host(nullptr), pa(func) {
			static Table tab = CreateTable(&TFunction::InvokerA);
			tablePointer = &tab;
		}
		TFunction(ZB func) : host(nullptr), pb(func) {
			static Table tab = CreateTable(&TFunction::InvokerB);
			tablePointer = &tab;
		}
		TFunction(ZC func) : host(nullptr), pc(func) {
			static Table tab = CreateTable(&TFunction::InvokerC);
			tablePointer = &tab;
		}
		TFunction(ZD func) : host(nullptr), pd(func) {
			static Table tab = CreateTable(&TFunction::InvokerD);
			tablePointer = &tab;
		}
		TFunction(ZE func) : host(nullptr), pe(func) {
			static Table tab = CreateTable(&TFunction::InvokerE);
			tablePointer = &tab;
		}
		TFunction(ZF func) : host(nullptr), pf(func) {
			static Table tab = CreateTable(&TFunction::InvokerF);
			tablePointer = &tab;
		}
		TFunction(ZG func) : host(nullptr), pg(func) {
			static Table tab = CreateTable(&TFunction::InvokerG);
			tablePointer = &tab;
		}
		TFunction(ZH func) : host(nullptr), ph(func) {
			static Table tab = CreateTable(&TFunction::InvokerH);
			tablePointer = &tab;
		}
		TFunction(ZI func) : host(nullptr), pi(func) {
			static Table tab = CreateTable(&TFunction::InvokerI);
			tablePointer = &tab;
		}
		TFunction(ZJ func) : host(nullptr), pj(func) {
			static Table tab = CreateTable(&TFunction::InvokerJ);
			tablePointer = &tab;
		}
		TFunction(ZK func) : host(nullptr), pk(func) {
			static Table tab = CreateTable(&TFunction::InvokerK);
			tablePointer = &tab;
		}
		TFunction(ZL func) : host(nullptr), pl(func) {
			static Table tab = CreateTable(&TFunction::InvokerL);
			tablePointer = &tab;
		}
		TFunction(ZM func) : host(nullptr), pm(func) {
			static Table tab = CreateTable(&TFunction::InvokerM);
			tablePointer = &tab;
		}
		TFunction(ZN func) : host(nullptr), pn(func) {
			static Table tab = CreateTable(&TFunction::InvokerN);
			tablePointer = &tab;
		}
		TFunction(ZO func) : host(nullptr), po(func) {
			static Table tab = CreateTable(&TFunction::InvokerO);
			tablePointer = &tab;
		}
		TFunction(ZP func) : host(nullptr), pp(func) {
			static Table tab = CreateTable(&TFunction::InvokerP);
			tablePointer = &tab;
		}

		template <class Z>
		struct Dispatch {
			IHost* host;
			union {
				ZZ p;
				ZA pa;
				ZB pb;
				ZC pc;
				ZD pd;
				ZE pe;
				ZF pf;
				ZG pg;
				ZH ph;
				ZI pi;
				ZJ pj;
				ZK pk;
				ZL pl;
				ZM pm;
				ZN pn;
				ZO po;
				ZP pp;
			};

			Dispatch(IHost* h, ZZ f) : host(h), p(f) {}
			inline Z Invoke() const {
				return p();
			}

			inline Z Invoke(A a) const {
				return pa(a);
			}

			inline Z Invoke(A a, B b) const {
				return pb(a, b);
			}

			inline Z Invoke(A a, B b, C c) const {
				return pc(a, b, c);
			}

			inline Z Invoke(A a, B b, C c, D d) const {
				return pd(a, b, c, d);
			}

			inline Z Invoke(A a, B b, C c, D d, E e) const {
				return pe(a, b, c, d, e);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f) const {
				return pf(a, b, c, d, e, f);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g) const {
				return pg(a, b, c, d, e, f, g);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h) const {
				return ph(a, b, c, d, e, f, g, h);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i) const {
				return pi(a, b, c, d, e, f, g, h, i);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const {
				return pj(a, b, c, d, e, f, g, h, i, j);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const {
				return pk(a, b, c, d, e, f, g, h, i, j, k);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const {
				return pl(a, b, c, d, e, f, g, h, i, j, k, l);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const {
				return pm(a, b, c, d, e, f, g, h, i, j, k, l, m);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const {
				return pn(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const {
				return po(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
			}

			inline Z Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const {
				return pp(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
			}
		};

		template <>
		struct Dispatch<void> {
			IHost* host;
			union {
				ZZ p;
				ZA pa;
				ZB pb;
				ZC pc;
				ZD pd;
				ZE pe;
				ZF pf;
				ZG pg;
				ZH ph;
				ZI pi;
				ZJ pj;
				ZK pk;
				ZL pl;
				ZM pm;
				ZN pn;
				ZO po;
				ZP pp;
			};

			Dispatch<void>(IHost* h, ZZ f) : host(h), p(f) {}

			inline Void Invoke() const {
				p();
				return Void();
			}

			inline Void Invoke(A a) const {
				pa(a);
				return Void();
			}

			inline Void Invoke(A a, B b) const {
				pb(a, b);
				return Void();
			}

			inline Void Invoke(A a, B b, C c) const {
				pc(a, b, c);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d) const {
				pd(a, b, c, d);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e) const {
				pe(a, b, c, d, e);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f) const {
				pf(a, b, c, d, e, f);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g) const {
				pg(a, b, c, d, e, f, g);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h) const {
				ph(a, b, c, d, e, f, g, h);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i) const {
				pi(a, b, c, d, e, f, g, h, i);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const {
				pj(a, b, c, d, e, f, g, h, i, j);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const {
				pk(a, b, c, d, e, f, g, h, i, j, k);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const {
				pl(a, b, c, d, e, f, g, h, i, j, k, l);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const {
				pm(a, b, c, d, e, f, g, h, i, j, k, l, m);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const {
				pn(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const {
				po(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
				return Void();
			}

			inline Void Invoke(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const {
				pp(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
				return Void();
			}
		};

		static typename ReturnType<R>::type InvokerZ(const TFunction* t) {
			assert(GetCount() == 0);
			return Dispatch<R>(t->host, t->p).Invoke();
		}

		static typename ReturnType<R>::type InvokerA(const TFunction* t, A a) {
			assert(GetCount() == 1);
			return Dispatch<R>(t->host, t->p).Invoke(a);
		}

		static typename ReturnType<R>::type InvokerB(const TFunction* t, A a, B b) {
			assert(GetCount() == 2);
			return Dispatch<R>(t->host, t->p).Invoke(a, b);
		}

		static typename ReturnType<R>::type InvokerC(const TFunction* t, A a, B b, C c) {
			assert(GetCount() == 3);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c);
		}

		static typename ReturnType<R>::type InvokerD(const TFunction* t, A a, B b, C c, D d) {
			assert(GetCount() == 4);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d);
		}

		static typename ReturnType<R>::type InvokerE(const TFunction* t, A a, B b, C c, D d, E e) {
			assert(GetCount() == 5);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e);
		}

		static typename ReturnType<R>::type InvokerF(const TFunction* t, A a, B b, C c, D d, E e, F f) {
			assert(GetCount() == 6);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f);
		}

		static typename ReturnType<R>::type InvokerG(const TFunction* t, A a, B b, C c, D d, E e, F f, G g) {
			assert(GetCount() == 7);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g);
		}

		static typename ReturnType<R>::type InvokerH(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h) {
			assert(GetCount() == 8);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h);
		}

		static typename ReturnType<R>::type InvokerI(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i) {
			assert(GetCount() == 9);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i);
		}

		static typename ReturnType<R>::type InvokerJ(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) {
			assert(GetCount() == 10);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j);
		}

		static typename ReturnType<R>::type InvokerK(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) {
			assert(GetCount() == 11);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k);
		}

		static typename ReturnType<R>::type InvokerL(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) {
			assert(GetCount() == 12);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l);
		}

		static typename ReturnType<R>::type InvokerM(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) {
			assert(GetCount() == 13);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		static typename ReturnType<R>::type InvokerN(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) {
			assert(GetCount() == 14);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}

		static typename ReturnType<R>::type InvokerO(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) {
			assert(GetCount() == 15);
			return Dispatch<R>(t->host, t->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
		}

		static typename ReturnType<R>::type InvokerP(const TFunction* t, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) {
			assert(GetCount() == 16);
			return Dispatch<R>(host, this->p).Invoke(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
		}

		typename ReturnType<R>::type operator () () const {
			return tablePointer->invokerZ(this);
		}

		typename ReturnType<R>::type operator () (A a) const {
			return tablePointer->invokerA(this, a);
		}

		typename ReturnType<R>::type operator () (A a, B b) const {
			return tablePointer->invokerB(this, a, b);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c) const {
			return tablePointer->invokerC(this, a, b, c);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d) const {
			return tablePointer->invokerD(this, a, b, c, d);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e) const {
			return tablePointer->invokerE(this, a, b, c, d, e);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f) const {
			return tablePointer->invokerF(this, a, b, c, d, e, f);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g) const {
			return tablePointer->invokerG(this, a, b, c, d, e, f, g);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h) const {
			return tablePointer->invokerH(this, a, b, c, d, e, f, g, h);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i) const {
			return tablePointer->invokerI(this, a, b, c, d, e, f, g, h, i);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const {
			return tablePointer->invokerJ(this, a, b, c, d, e, f, g, h, i, j);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const {
			return tablePointer->invokerK(this, a, b, c, d, e, f, g, h, i, j, k);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const {
			return tablePointer->invokerL(this, a, b, c, d, e, f, g, h, i, j, k, l);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const {
			return tablePointer->invokerM(this, a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const {
			return tablePointer->invokerN(this, a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const {
			return tablePointer->invokerO(this, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
		}

		typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const {
			return tablePointer->invokerP(this, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
		}

		Table* tablePointer;
		IHost* host;
		union {
			ZZ p;
			ZA pa;
			ZB pb;
			ZC pc;
			ZD pd;
			ZE pe;
			ZF pf;
			ZG pg;
			ZH ph;
			ZI pi;
			ZJ pj;
			ZK pk;
			ZL pl;
			ZM pm;
			ZN pn;
			ZO po;
			ZP pp;
		};
	};

	// TWrapper is the same as TMethod, without template class type T.
	template <class R = Void, class A = Void, class B = Void, class C = Void, class D = Void, class E = Void, class F = Void, class G = Void, class H = Void, class I = Void, class J = Void, class K = Void, class L = Void, class M = Void, class N = Void, class O = Void, class P = Void>
	class TWrapper {
	private:
		typedef R _R; typedef A _A; typedef B _B; typedef C _C;
		typedef D _D; typedef E _E; typedef F _F; typedef G _G;
		typedef H _H; typedef I _I; typedef J _J; typedef K _K;
		typedef L _L; typedef M _M; typedef N _N; typedef O _O;
		typedef P _P;
		template <class T>
		void Init(T t) {
			TFunction<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> func(t);
			TMethod<std::false_type, Void, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>& m = reinterpret_cast<TMethod<std::false_type, Void, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>&>(func);
			proxy = m;
		}
	public:
		TWrapper() {}

		TWrapper(R(*d)()) {
			Init(d);
		}

		TWrapper(R(*d)(A)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I, J)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I, J, K)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I, J, K, L)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I, J, K, L, M)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I, J, K, L, M, N)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O)) {
			Init(d);
		}

		TWrapper(R(*d)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P)) {
			Init(d);
		}

		TWrapper(const TProxy<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>& t) {
			const TMethod<std::false_type, Void, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>& m = reinterpret_cast<const TMethod<std::false_type, Void, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>&>(t);
			proxy = m;
		}

		operator bool() const {
			return proxy.p != nullptr;
		}

		void Clear() {
			proxy.p = nullptr;
		}

		inline IHost* GetHost() const { return proxy.host; }
		inline size_t GetCount() const { return proxy.GetCount(); }

		bool operator < (const TWrapper& rhs) const {
			if (proxy.host < rhs.proxy.host) {
				return true;
			} else if (proxy.host > rhs.proxy.host) {
				return false;
			} else {
				return *(void**)&proxy.p < *(void**)&rhs.proxy.p;
			}
		}

		inline void Hook(TWrapper& rhs) {
			std::swap(proxy, rhs.proxy);
		}

		const TProxy<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>& GetProxy() const {
			return proxy;
		}

		inline typename ReturnType<R>::type operator () () const {
			return proxy();
		}

		inline typename ReturnType<R>::type operator () (A a) const {
			return proxy(a);
		}

		inline typename ReturnType<R>::type operator () (A a, B b) const {
			return proxy(a, b);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c) const {
			return proxy(a, b, c);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d) const {
			return proxy(a, b, c, d);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e) const {
			return proxy(a, b, c, d, e);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f) const {
			return proxy(a, b, c, d, e, f);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g) const {
			return proxy(a, b, c, d, e, f, g);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h) const {
			return proxy(a, b, c, d, e, f, g, h);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i) const {
			return proxy(a, b, c, d, e, f, g, h, i);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const {
			return proxy(a, b, c, d, e, f, g, h, i, j);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const {
			return proxy(a, b, c, d, e, f, g, h, i, j, k);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const {
			return proxy(a, b, c, d, e, f, g, h, i, j, k, l);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const {
			return proxy(a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const {
			return proxy(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const {
			return proxy(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
		}

		inline typename ReturnType<R>::type operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const {
			return proxy(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
		}

		TMethod<std::false_type, Void, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> proxy;
	};

	template <class T, class R>
	const TWrapper<R> Wrap(T* t, R(T::*d)()) {
		return TWrapper<R>(TMethod<std::false_type, T, R>(t, d));
	}

	template <class T, class R, class A>
	const TWrapper<R, A> Wrap(T* t, R(T::*d)(A a)) {
		return TWrapper<R, A>(TMethod<std::false_type, T, R, A>(t, d));
	}

	template <class T, class R, class A, class B>
	const TWrapper<R, A, B> Wrap(T* t, R(T::*d)(A a, B b)) {
		return TWrapper<R, A, B>(TMethod<std::false_type, T, R, A, B>(t, d));
	}

	template <class T, class R, class A, class B, class C>
	const TWrapper<R, A, B, C> Wrap(T* t, R(T::*d)(A a, B b, C c)) {
		return TWrapper<R, A, B, C>(TMethod<std::false_type, T, R, A, B, C>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D>
	const TWrapper<R, A, B, C, D> Wrap(T* t, R(T::*d)(A a, B b, C c, D d)) {
		return TWrapper<R, A, B, C, D>(TMethod<std::false_type, T, R, A, B, C, D>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E>
	const TWrapper<R, A, B, C, D, E> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e)) {
		return TWrapper<R, A, B, C, D, E>(TMethod<std::false_type, T, R, A, B, C, D, E>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F>
	const TWrapper<R, A, B, C, D, E, F> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f)) {
		return TWrapper<R, A, B, C, D, E, F>(TMethod<std::false_type, T, R, A, B, C, D, E, F>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G>
	const TWrapper<R, A, B, C, D, E, F, G> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g)) {
		return TWrapper<R, A, B, C, D, E, F, G>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H>
	const TWrapper<R, A, B, C, D, E, F, G, H> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h)) {
		return TWrapper<R, A, B, C, D, E, F, G, H>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I>
	const TWrapper<R, A, B, C, D, E, F, G, H, I> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(t, d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(t, d));
	}

	template <class T, class R>
	const TWrapper<R> Wrap(T* t, R(T::*d)() const) {
		typedef R(T::*org)();
		return TWrapper<R>(TMethod<std::false_type, T, R>(t, (org)d));
	}

	template <class T, class R, class A>
	const TWrapper<R, A> Wrap(T* t, R(T::*d)(A a) const) {
		typedef R(T::*org)(A);
		return TWrapper<R, A>(TMethod<std::false_type, T, R, A>(t, (org)d));
	}

	template <class T, class R, class A, class B>
	const TWrapper<R, A, B> Wrap(T* t, R(T::*d)(A a, B b) const) {
		typedef R(T::*org)(A, B);
		return TWrapper<R, A, B>(TMethod<std::false_type, T, R, A, B>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C>
	const TWrapper<R, A, B, C> Wrap(T* t, R(T::*d)(A a, B b, C c) const) {
		typedef R(T::*org)(A, B, C);
		return TWrapper<R, A, B, C>(TMethod<std::false_type, T, R, A, B, C>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D>
	const TWrapper<R, A, B, C, D> Wrap(T* t, R(T::*d)(A a, B b, C c, D d) const) {
		typedef R(T::*org)(A, B, C, D);
		return TWrapper<R, A, B, C, D>(TMethod<std::false_type, T, R, A, B, C, D>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E>
	const TWrapper<R, A, B, C, D, E> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e) const) {
		typedef R(T::*org)(A, B, C, D, E);
		return TWrapper<R, A, B, C, D, E>(TMethod<std::false_type, T, R, A, B, C, D, E>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F>
	const TWrapper<R, A, B, C, D, E, F> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f) const) {
		typedef R(T::*org)(A, B, C, D, E, F);
		return TWrapper<R, A, B, C, D, E, F>(TMethod<std::false_type, T, R, A, B, C, D, E, F>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G>
	const TWrapper<R, A, B, C, D, E, F, G> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G);
		return TWrapper<R, A, B, C, D, E, F, G>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H>
	const TWrapper<R, A, B, C, D, E, F, G, H> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H);
		return TWrapper<R, A, B, C, D, E, F, G, H>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I>
	const TWrapper<R, A, B, C, D, E, F, G, H, I> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I);
		return TWrapper<R, A, B, C, D, E, F, G, H, I>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M, N);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(t, (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> Wrap(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(TMethod<std::false_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(t, (org)d));
	}

	// closure
	template <class T, class R>
	const TWrapper<R> WrapClosure(T* t, R(T::*d)()) {
		return TWrapper<R>(TMethod<std::true_type, T, R>(new T(*t), d));
	}

	template <class T, class R, class A>
	const TWrapper<R, A> WrapClosure(T* t, R(T::*d)(A a)) {
		return TWrapper<R, A>(TMethod<std::true_type, T, R, A>(new T(*t), d));
	}

	template <class T, class R, class A, class B>
	const TWrapper<R, A, B> WrapClosure(T* t, R(T::*d)(A a, B b)) {
		return TWrapper<R, A, B>(TMethod<std::true_type, T, R, A, B>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C>
	const TWrapper<R, A, B, C> WrapClosure(T* t, R(T::*d)(A a, B b, C c)) {
		return TWrapper<R, A, B, C>(TMethod<std::true_type, T, R, A, B, C>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D>
	const TWrapper<R, A, B, C, D> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d)) {
		return TWrapper<R, A, B, C, D>(TMethod<std::true_type, T, R, A, B, C, D>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E>
	const TWrapper<R, A, B, C, D, E> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e)) {
		return TWrapper<R, A, B, C, D, E>(TMethod<std::true_type, T, R, A, B, C, D, E>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F>
	const TWrapper<R, A, B, C, D, E, F> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f)) {
		return TWrapper<R, A, B, C, D, E, F>(TMethod<std::true_type, T, R, A, B, C, D, E, F>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G>
	const TWrapper<R, A, B, C, D, E, F, G> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g)) {
		return TWrapper<R, A, B, C, D, E, F, G>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H>
	const TWrapper<R, A, B, C, D, E, F, G, H> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h)) {
		return TWrapper<R, A, B, C, D, E, F, G, H>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I>
	const TWrapper<R, A, B, C, D, E, F, G, H, I> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(new T(*t), d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(new T(*t), d));
	}

	template <class T, class R>
	const TWrapper<R> WrapClosure(T* t, R(T::*d)() const) {
		typedef R(T::*org)();
		return TWrapper<R>(TMethod<std::true_type, T, R>(new T(*t), (org)d));
	}

	template <class T, class R, class A>
	const TWrapper<R, A> WrapClosure(T* t, R(T::*d)(A a) const) {
		typedef R(T::*org)(A);
		return TWrapper<R, A>(TMethod<std::true_type, T, R, A>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B>
	const TWrapper<R, A, B> WrapClosure(T* t, R(T::*d)(A a, B b) const) {
		typedef R(T::*org)(A, B);
		return TWrapper<R, A, B>(TMethod<std::true_type, T, R, A, B>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C>
	const TWrapper<R, A, B, C> WrapClosure(T* t, R(T::*d)(A a, B b, C c) const) {
		typedef R(T::*org)(A, B, C);
		return TWrapper<R, A, B, C>(TMethod<std::true_type, T, R, A, B, C>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D>
	const TWrapper<R, A, B, C, D> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d) const) {
		typedef R(T::*org)(A, B, C, D);
		return TWrapper<R, A, B, C, D>(TMethod<std::true_type, T, R, A, B, C, D>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E>
	const TWrapper<R, A, B, C, D, E> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e) const) {
		typedef R(T::*org)(A, B, C, D, E);
		return TWrapper<R, A, B, C, D, E>(TMethod<std::true_type, T, R, A, B, C, D, E>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F>
	const TWrapper<R, A, B, C, D, E, F> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f) const) {
		typedef R(T::*org)(A, B, C, D, E, F);
		return TWrapper<R, A, B, C, D, E, F>(TMethod<std::true_type, T, R, A, B, C, D, E, F>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G>
	const TWrapper<R, A, B, C, D, E, F, G> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G);
		return TWrapper<R, A, B, C, D, E, F, G>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H>
	const TWrapper<R, A, B, C, D, E, F, G, H> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H);
		return TWrapper<R, A, B, C, D, E, F, G, H>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I>
	const TWrapper<R, A, B, C, D, E, F, G, H, I> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I);
		return TWrapper<R, A, B, C, D, E, F, G, H, I>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M, N);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(new T(*t), (org)d));
	}

	template <class T, class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> WrapClosure(T* t, R(T::*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) const) {
		typedef R(T::*org)(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P);
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(TMethod<std::true_type, T, R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(new T(*t), (org)d));
	}

	// function

	template <class R>
	const TWrapper<R> Wrap(R(*d)()) {
		return TWrapper<R>(d);
	}

	template <class R, class A>
	const TWrapper<R, A> Wrap(R(*d)(A a)) {
		return TWrapper<R, A>(d);
	}

	template <class R, class A, class B>
	const TWrapper<R, A, B> Wrap(R(*d)(A a, B b)) {
		return TWrapper<R, A, B>(d);
	}

	template <class R, class A, class B, class C>
	const TWrapper<R, A, B, C> Wrap(R(*d)(A a, B b, C c)) {
		return TWrapper<R, A, B, C>(d);
	}

	template <class R, class A, class B, class C, class D>
	const TWrapper<R, A, B, C, D> Wrap(R(*d)(A a, B b, C c, D d)) {
		return TWrapper<R, A, B, C, D>(d);
	}

	template <class R, class A, class B, class C, class D, class E>
	const TWrapper<R, A, B, C, D, E> Wrap(R(*d)(A a, B b, C c, D d, E e)) {
		return TWrapper<R, A, B, C, D, E>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F>
	const TWrapper<R, A, B, C, D, E, F> Wrap(R(*d)(A a, B b, C c, D d, E e, F f)) {
		return TWrapper<R, A, B, C, D, E, F>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G>
	const TWrapper<R, A, B, C, D, E, F, G> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g)) {
		return TWrapper<R, A, B, C, D, E, F, G>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H>
	const TWrapper<R, A, B, C, D, E, F, G, H> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h)) {
		return TWrapper<R, A, B, C, D, E, F, G, H>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I>
	const TWrapper<R, A, B, C, D, E, F, G, H, I> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>(d);
	}

	template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
	const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> Wrap(R(*d)(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)) {
		return TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>(d);
	}

#else

	template <typename R = Void, typename... Args>
	class TProxy {
	public:
		static inline size_t GetCount() { return sizeof...(Args); }
	};


	template <class T, class M, class Z, class... Args>
	struct Dispatch {
		Dispatch(const M& t) : p(t) {}
		inline Z Invoke(Args&&... args) const {
			return ((reinterpret_cast<T*>(p.host))->*(p.p))(std::forward<Args>(args)...);
		}
		const M& p;
	};

	template <class T, class M, class... Args>
	struct Dispatch<T, M, void, Args...> {
		Dispatch(const M& t) : p(t) {}
		inline Void Invoke(Args&&... args) {
			((reinterpret_cast<T*>(p.host))->*(p.p))(std::forward<Args>(args)...);
			return Void();
		}
		const M& p;
	};

	template <bool manageHost, typename T, typename R = Void, typename... Args>
	class TMethod : public TProxy<R, Args...> {
	public:
		typedef R(T::*FUNC)(Args...);
		struct Table {
			typename ReturnType<R>::type(*invoker)(const TMethod*, Args&&... args);
			void(*duplicator)(TMethod& output, const TMethod& input);
			void(*destructor)(TMethod& input);
		};

		void InitTablePointer() {
			static Table tabManage = { &TMethod::Invoke, &TMethod::Copy, &TMethod::Destroy };
			tablePointer = &tabManage;
		}

		TMethod(T* ptr = nullptr, FUNC func = nullptr) : tablePointer(nullptr), host((IHost*)ptr), p(func) {
			InitTablePointer();
		}

		TMethod(const TMethod& rhs) {
			InitTablePointer();
			*this = rhs;
		}

		TMethod(TMethod&& rhs) {
			InitTablePointer();
			*this = std::move(rhs);
		}

		~TMethod() {
			tablePointer->destructor(*this);
		}

		template <bool host>
		static typename std::enable_if<!host>::type CopyImpl(TMethod& output, const TMethod& input) {
			output.~TMethod();
			output.tablePointer = input.tablePointer;
			output.host = input.host;
			output.p = input.p;
		}

		template <bool host>
		static typename std::enable_if<host>::type CopyImpl(TMethod& output, const TMethod& input) {
			output.~TMethod();
			output.tablePointer = input.tablePointer;
			output.host = reinterpret_cast<IHost*>(new T(*reinterpret_cast<T*>(input.host)));
			output.p = input.p;
		}

		static void Copy(TMethod& output, const TMethod& input) {
			CopyImpl<manageHost>(output, input);
		}

		template <bool host>
		static typename std::enable_if<!host>::type DestroyImpl(TMethod& input) {}
		template <bool host>
		static typename std::enable_if<host>::type DestroyImpl(TMethod& input) {
			if (input.host != nullptr) {
				delete reinterpret_cast<T*>(input.host);
			}
		}

		static void Destroy(TMethod& input) {
			DestroyImpl<manageHost>(input);
		}

		static typename ReturnType<R>::type Invoke(const TMethod* m, Args&&... args) {
			return Dispatch<T, TMethod<manageHost, T, R, Args...>, R, Args...>(*m).Invoke(std::forward<Args>(args)...);
		}

		// virtual size_t GetLength() const { return sizeof(*this); }
		typename ReturnType<R>::type operator () (Args&&... args) const {
			return tablePointer->invoker(this, std::forward<Args>(args)...);
		}

		TMethod& operator = (const TMethod& rhs) {
			rhs.tablePointer->duplicator(*this, rhs);
			return *this;
		}

		TMethod& operator = (TMethod&& rhs) {
			tablePointer = rhs.tablePointer;
			host = rhs.host;
			p = rhs.p;

			rhs.host = nullptr; // clear destructor info.
			return *this;
		}

		Table* tablePointer;
		IHost* host;
		FUNC p;
	};

	template <typename R = Void, typename... Args>
	class TFunction : public TProxy<R, Args...> {
	public:
		typedef R(*FUNC)(Args...);
		struct Table {
			typename ReturnType<R>::type(*invoker)(const TFunction*, Args&&... args);
			void(*duplicator)(TFunction& output, const TFunction& input);
			void(*destructor)(TFunction& input);
		};

		static void Copy(TFunction& output, const TFunction& input) {
			output.tablePointer = input.tablePointer;
			output.host = input.host;
			output.p = input.p;
		}
		static void Destroy(TFunction& input) {}

		TFunction(FUNC func = nullptr) : tablePointer(nullptr), host(nullptr), p(func) {
	    Assign(ReturnType<R>());
		}

		template <class T>
		void Assign(ReturnType<T>) {
			static Table tab = { Invoke, Copy, Destroy };
			tablePointer = &tab;
		}
#ifdef _MSC_VER
		template <>
#endif
		void Assign(ReturnType<void>) {
			static Table tab = { InvokeNoReturn, Copy, Destroy };
			tablePointer = &tab;
		}

		static typename ReturnType<R>::type Invoke(const TFunction* m, Args&&... args) {
			return m->p(std::forward<Args>(args)...);
		}

		static typename ReturnType<void>::type InvokeNoReturn(const TFunction* m, Args&&... args) {
			m->p(std::forward<Args>(args)...);
			return Void();
		}

		Table* tablePointer;
		IHost* host;
		FUNC p;
	};

	template <typename... Args>
	class TWrapper {};

	template <typename R, typename... Args>
	class TWrapper<R, Args...>  {
	public:
		TWrapper() { }
		TWrapper(R(*d)(Args...)) : TWrapper(TFunction<R, Args...>(d)) {}
		TWrapper(const TProxy<R, Args...>& t) {
			proxy = static_cast<const TMethod<false, Void, R, Args...>&>(t);
		}
		TWrapper(TProxy<R, Args...>&& t) {
			proxy = static_cast<TMethod<false, Void, R, Args...>&&>(t);
		}

		TWrapper(const TWrapper& w) {
			*this = w;
		}

		TWrapper(TWrapper&& rhs) {
			*this = std::move(rhs);
		}

		TWrapper& operator = (TWrapper&& rhs) {
			proxy = std::move(rhs.proxy);
			return *this;
		}

		TWrapper& operator = (const TWrapper& rhs) {
			proxy = rhs.proxy;
			return *this;
		}

		void Clear() {
			proxy.~TMethod<false, Void, R, Args...>();
			proxy.host = nullptr;
			proxy.p = nullptr;
		}

		operator bool() const {
			return proxy.p != nullptr;
		}

		bool operator < (const TWrapper& rhs) const {
			if (proxy.host < rhs.proxy.host) {
				return true;
			} else if (proxy.host > rhs.proxy.host) {
				return false;
			} else {
				return *(void**)&proxy.p < *(void**)&rhs.proxy.p;
			}
		}

		inline typename ReturnType<R>::type operator () (Args... args) const {
			return proxy(std::forward<Args>(args)...);
		}

		inline IHost* GetHost() const { return proxy.host; }
		const TProxy<R, Args...>& GetProxy() const { return proxy; }
		inline size_t GetCount() const { return proxy.GetCount(); }
		inline void Hook(TWrapper& rhs) {
			std::swap(proxy, rhs.proxy);
		}

		TMethod<false, Void, R, Args...> proxy;
	};

	template <typename T, typename R, typename... Args>
	const TWrapper<R, Args...> Wrap(T* t, R(T::*d)(Args...)) {
		return TWrapper<R, Args...>(TMethod<false, T, R, Args...>(t, d));
	}

	template <typename T, typename R, typename... Args>
	const TWrapper<R, Args...> Wrap(const T* t, R(T::*d)(Args...) const) {
		typedef R(T::*org)(Args...);
		return TWrapper<R, Args...>(TMethod<false, T, R, Args...>(const_cast<T*>(t), (org)d));
	}

	template <typename R, typename... Args>
	const TWrapper<R, Args...> Wrap(R(*d)(Args...)) {
		return TWrapper<R, Args...>(d);
	}

	template <typename T, typename R, typename... Args>
	const TWrapper<R, Args...> WrapClosure(T* t, R(T::*d)(Args...)) {
		typedef R(T::*org)(Args...);
		return TWrapper<R, Args...>(TMethod<true, T, R, Args...>(new T(*t), (org)d));
	}

	template <typename T, typename R, typename... Args>
	const TWrapper<R, Args...> WrapClosure(const T* t, R(T::*d)(Args...) const) {
		typedef R(T::*org)(Args...);
		return TWrapper<R, Args...>(TMethod<true, T, R, Args...>(new T(*t), (org)d));
	}

	// be aware of object's life-scope !!!
	template <typename T>
	auto WrapClosure(const T& object) -> decltype(WrapClosure(&object, &T::operator())) {
		return WrapClosure(&object, &T::operator());
	}

#endif


#if defined(_MSC_VER) && _MSC_VER < 1800 || defined(COMPATIBLE_PROXY)
	class Invoker {
	public:
		Invoker(const TWrapper<void>& func) {
			func();
		}

		template <typename A>
		Invoker(const TWrapper<void, A>& func, A a) {
			func(a);
		}

		template <typename A, typename B>
		Invoker(const TWrapper<void, A, B>& func, A a, B b) {
			func(a, b);
		}

		template <typename A, typename B, typename C>
		Invoker(const TWrapper<void, A, B, C>& func, A a, B b, C c) {
			func(a, b, c);
		}

		template <typename A, typename B, typename C, typename D>
		Invoker(const TWrapper<void, A, B, C, D>& func, A a, B b, C c, D d) {
			func(a, b, c, d);
		}

		template <typename A, typename B, typename C, typename D, typename E>
		Invoker(const TWrapper<void, A, B, C, D, E>& func, A a, B b, C c, D d, E e) {
			func(a, b, c, d, e);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F>
		Invoker(const TWrapper<void, A, B, C, D, E, F>& func, A a, B b, C c, D d, E e, F f) {
			func(a, b, c, d, e, f);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G>& func, A a, B b, C c, D d, E e, F f, G g) {
			func(a, b, c, d, e, f, g);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H>& func, A a, B b, C c, D d, E e, F f, G g, H h) {
			func(a, b, c, d, e, f, g, h);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i) {
			func(a, b, c, d, e, f, g, h, i);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I, J>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) {
			func(a, b, c, d, e, f, g, h, i, j);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I, J, K>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) {
			func(a, b, c, d, e, f, g, h, i, j, k);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I, J, K, L>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) {
			func(a, b, c, d, e, f, g, h, i, j, k, l);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I, J, K, L, M>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) {
			func(a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I, J, K, L, M, N>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) {
			func(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) {
			func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
		}

		template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
		Invoker(const TWrapper<void, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>& func, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) {
			func(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
		}
	};
#else
	class Invoker {
	public:
		Invoker(const TWrapper<void>& func) {
			func();
		}
		template <typename... Args>
		Invoker(const TWrapper<void, Args...>& func, Args&&... args) {
			func(std::forward<Args>(args)...);
		}
	};
#endif
}


#endif // __TPROXY_H__
