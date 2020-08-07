// TEvent.h
// By PaintDream (paintdream@paintdream.com)
// 2015-6-6
//

#ifndef __TEVENT_H__
#define __TEVENT_H__

#include "TProxy.h"
#include "TAlgorithm.h"

namespace PaintsNow {
#if defined(_MSC_VER) && _MSC_VER < 1800
	template <class A = Void, class B = Void, class C = Void, class D = Void, class E = Void, class F = Void, class G = Void, class H = Void, class I = Void, class J = Void, class K = Void, class L = Void, class M = Void, class N = Void, class O = Void, class P = Void>
	class TEvent {
	public:
		virtual ~TEvent() {}
		typedef TWrapper<void, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> W;
		virtual TEvent& operator += (const W& t) {
			std::binary_insert(callbacks, t);
			return *this;
		}

		virtual TEvent& operator -= (const W& t) {
			std::binary_erase(callbacks, t);
			return *this;
		}

		void operator () () {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m();
			}
		} 

		void operator () (A a) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a);
			}
		} 

		void operator () (A a, B b) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b);
			}
		} 

		void operator () (A a, B b, C c) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c);
			}
		} 

		void operator () (A a, B b, C c, D d) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d);
			}
		} 

		void operator () (A a, B b, C c, D d, E e) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e);
			}
		}

		void operator () (A a, B b, C c, D d, E e, F f) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e, f);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e, f, g);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e, f, g, h);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e, f, g, h, i);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e, f, g, h, i, j);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e, f, g, h, i, j, k);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& m = *it;
				++it;
				m(a, b, c, d, e, f, g, h, i, j, k, l);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& mm = *it;
				++it;
				mm(a, b, c, d, e, f, g, h, i, j, k, l, m);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& mm = *it;
				++it;
				mm(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != callbacks.end();) {
				const W& mm = *it;
				++it;
				mm(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
			}
		} 

		void operator () (A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) {
			for (typename std::vector<W>::iterator it = callbacks.begin(); it != cald();) {
				const W& mm = *it;
				++it;
				mm(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
			}
		} 

		size_t GetCount() const { return callbacks.size(); }

	private:
		std::vector<W> callbacks;
	};

#else
	template <typename... Args>
	class TEvent {
	public:
		typedef TWrapper<void, Args...> W;
		virtual ~TEvent() {}

		virtual TEvent& operator += (const W& t) {
			std::binary_insert(callbacks, t);
			return *this;
		}

		virtual TEvent& operator -= (const W& t) {
			// assert(callbacks.count(t) != 0);
			std::binary_erase(callbacks, t);
			return *this;
		}

		template <typename... Params>
		void operator () (Params&&... params) {
			typename std::vector<W>::iterator it = callbacks.begin();
			if (it != callbacks.end()) {
				while (true) {
					const TWrapper<void, Args...>& m = *it;
					if (++it != callbacks.end()) {
						m(params...);
					} else {
						m(std::forward<Params>(params)...);
						break;
					}
				}
			}
		}

		size_t GetCount() const { return callbacks.size(); }

	protected:
		std::vector<W> callbacks;
	};
#endif
}


#endif // __TEVENT_H__
