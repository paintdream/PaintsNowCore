// ITask.h
// By PaintDream (paintdream@paintdream.com)
// 2016-4-6
//

#ifndef __ITASK_H__
#define __ITASK_H__

#include "../Template/TAlgorithm.h"
#include "IType.h"

namespace PaintsNow {
	class ITask {
	public:
		ITask();
		virtual ~ITask();
		enum {
			TASK_PRIORITY_BACKGROUND
		};

		virtual void Execute(void* context) = 0;
		virtual void Suspend(void* context) = 0;
		virtual void Resume(void* context) = 0;
		virtual void Abort(void* context) = 0;
		virtual bool Continue() const = 0;

		ITask* UpdateFlag(uint32_t flag);
		uint32_t flag;
	};

	class TaskOnce : public ITask {
	public:
		virtual void Execute(void* context) = 0;
		virtual void Suspend(void* context);
		virtual void Resume(void* context);
		virtual void Abort(void* context);
		virtual bool Continue() const;
	};

	class TaskRepeat : public ITask {
	public:
		virtual void Execute(void* context) = 0;
		virtual void Suspend(void* context);
		virtual void Resume(void* context);
		virtual void Abort(void* context) = 0;
		virtual bool Continue() const;
	};

#if defined(_MSC_VER) && _MSC_VER <= 1200
	template <class T>
	class TaskTemplate : public TaskOnce {
	public:
		TaskTemplate(T ref) : callback(ref) {}
		virtual void Execute(void* request) override {
			callback(request, true);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false);
			delete this;
		}

		T callback;
	};

	template <class T>
	ITask* CreateTask(T ref) {
		return new TaskTemplate<T>(ref);
	}

	template <class T, class A>
	class TaskTemplateA : public TaskOnce {
	public:
		TaskTemplateA(T ref, A a) : callback(ref) { pa = a; }
		virtual void Execute(void* request) override {
			callback(request, true, pa);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
	};

	template <class T, class A>
	ITask* CreateTask(T ref, A a) {
		return new TaskTemplateA<T, A>(ref, a);
	}

	template <class T, class A, class B>
	class TaskTemplateB : public TaskOnce {
	public:
		TaskTemplateB(T ref, A a, B b) : callback(ref) { pa = a; pb = b; }
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
	};

	template <class T, class A, class B>
	ITask* CreateTask(T ref, A a, B b) {
		return new TaskTemplateB<T, A, B>(ref, a, b);
	}

	template <class T, class A, class B, class C>
	class TaskTemplateC : public TaskOnce {
	public:
		TaskTemplateC(T ref, A a, B b, C c) : callback(ref) { pa = a; pb = b; pc = c; }
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
	};

	template <class T, class A, class B, class C>
	ITask* CreateTask(T ref, A a, B b, C c) {
		return new TaskTemplateC<T, A, B, C>(ref, a, b, c);
	}

	template <class T, class A, class B, class C, class D>
	class TaskTemplateD : public TaskOnce {
	public:
		TaskTemplateD(T ref, A a, B b, C c, D d) : callback(ref) { pa = a; pb = b; pc = c; pd = d; }
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
	};

	template <class T, class A, class B, class C, class D>
	ITask* CreateTask(T ref, A a, B b, C c, D d) {
		return new TaskTemplateD<T, A, B, C, D>(ref, a, b, c, d);
	}

	template <class T, class A, class B, class C, class D, class E>
	class TaskTemplateE : public TaskOnce {
	public:
		TaskTemplateE(T ref, A a, B b, C c, D d, E e) : callback(ref) { pa = a; pb = b; pc = c; pd = d; pe = e; }
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd, pe);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd, pe);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
	};

	template <class T, class A, class B, class C, class D, class E>
	ITask* CreateTask(T ref, A a, B b, C c, D d, E e) {
		return new TaskTemplateE<T, A, B, C, D, E>(ref, a, b, c, d, e);
	}

	template <class T, class A, class B, class C, class D, class E, class F>
	class TaskTemplateF : public TaskOnce {
	public:
		TaskTemplateF(T ref, A a, B b, C c, D d, E e, F f) : callback(ref) { pa = a; pb = b; pc = c; pd = d; pe = e; pf = f; }
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd, pe, pf);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd, pe, pf);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
		typename std::decay<F>::type pf;
	};

	template <class T, class A, class B, class C, class D, class E, class F>
	ITask* CreateTask(T ref, A a, B b, C c, D d, E e, F f) {
		return new TaskTemplateF<T, A, B, C, D, E, F>(ref, a, b, c, d, e, f);
	}

	template <class T, class A, class B, class C, class D, class E, class F, class G>
	class TaskTemplateG : public TaskOnce {
	public:
		TaskTemplateG(T ref, A a, B b, C c, D d, E e, F f, G g) : callback(ref) { pa = a; pb = b; pc = c; pd = d; pe = e; pf = f; pg = g; }
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd, pe, pf, pg);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd, pe, pf, pg);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
		typename std::decay<F>::type pf;
		typename std::decay<F>::type pg;
	};

	template <class T, class A, class B, class C, class D, class E, class F, class G>
	ITask* CreateTask(T ref, A a, B b, C c, D d, E e, F f, G g) {
		return new TaskTemplateG<T, A, B, C, D, E, F, G>(ref, a, b, c, d, e, f, g);
	}

	// ContextFree tasks

	template <class T>
	class ContextFreeTaskTemplate : public TaskOnce {
	public:
		ContextFreeTaskTemplate(T ref) : callback(ref) {}
		virtual void Execute(void* request) override {
			callback();
			delete this;
		}

		T callback;
	};

	template <class T>
	ITask* CreateTaskContextFree(T ref) {
		return new ContextFreeTaskTemplate<T>(ref);
	}

	template <class T, class A>
	class ContextFreeTaskTemplateA : public TaskOnce {
	public:
		ContextFreeTaskTemplateA(T ref, A a) : callback(ref) { pa = a; }
		virtual void Execute(void* request) override {
			callback(pa);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
	};

	template <class T, class A>
	ITask* CreateTaskContextFree(T ref, A a) {
		return new ContextFreeTaskTemplateA<T, A>(ref, a);
	}

	template <class T, class A, class B>
	class ContextFreeTaskTemplateB : public TaskOnce {
	public:
		ContextFreeTaskTemplateB(T ref, A a, B b) : callback(ref) { pa = a; pb = b; }
		virtual void Execute(void* request) override {
			callback(pa, pb);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
	};

	template <class T, class A, class B>
	ITask* CreateTaskContextFree(T ref, A a, B b) {
		return new ContextFreeTaskTemplateB<T, A, B>(ref, a, b);
	}

	template <class T, class A, class B, class C>
	class ContextFreeTaskTemplateC : public TaskOnce {
	public:
		ContextFreeTaskTemplateC(T ref, A a, B b, C c) : callback(ref) { pa = a; pb = b; pc = c; }
		virtual void Execute(void* request) override {
			callback(pa, pb, pc);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
	};

	template <class T, class A, class B, class C>
	ITask* CreateTaskContextFree(T ref, A a, B b, C c) {
		return new ContextFreeTaskTemplateC<T, A, B, C>(ref, a, b, c);
	}

	template <class T, class A, class B, class C, class D>
	class ContextFreeTaskTemplateD : public TaskOnce {
	public:
		ContextFreeTaskTemplateD(T ref, A a, B b, C c, D d) : callback(ref) { pa = a; pb = b; pc = c; pd = d; }
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
	};

	template <class T, class A, class B, class C, class D>
	ITask* CreateTaskContextFree(T ref, A a, B b, C c, D d) {
		return new ContextFreeTaskTemplateD<T, A, B, C, D>(ref, a, b, c, d);
	}

	template <class T, class A, class B, class C, class D, class E>
	class ContextFreeTaskTemplateE : public TaskOnce {
	public:
		ContextFreeTaskTemplateE(T ref, A a, B b, C c, D d, E e) : callback(ref) { pa = a; pb = b; pc = c; pd = d; pe = e; }
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd, pe);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
	};

	template <class T, class A, class B, class C, class D, class E>
	ITask* CreateTaskContextFree(T ref, A a, B b, C c, D d, E e) {
		return new ContextFreeTaskTemplateE<T, A, B, C, D, E>(ref, a, b, c, d, e);
	}

	template <class T, class A, class B, class C, class D, class E, class F>
	class ContextFreeTaskTemplateF : public TaskOnce {
	public:
		ContextFreeTaskTemplateF(T ref, A a, B b, C c, D d, E e, F f) : callback(ref) { pa = a; pb = b; pc = c; pd = d; pe = e; pf = f; }
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd, pe, pf);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
		typename std::decay<F>::type pf;
	};

	template <class T, class A, class B, class C, class D, class E, class F>
	ITask* CreateTaskContextFree(T ref, A a, B b, C c, D d, E e, F f) {
		return new ContextFreeTaskTemplateF<T, A, B, C, D, E, F>(ref, a, b, c, d, e, f);
	}

	template <class T, class A, class B, class C, class D, class E, class F, class G>
	class ContextFreeTaskTemplateG : public TaskOnce {
	public:
		ContextFreeTaskTemplateG(T ref, A a, B b, C c, D d, E e, F f, G g) : callback(ref) { pa = a; pb = b; pc = c; pd = d; pe = e; pf = f; pg = g; }
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd, pe, pf, pg);
			delete this;
		}

		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
		typename std::decay<F>::type pf;
		typename std::decay<G>::type pg;
	};

	template <class T, class A, class B, class C, class D, class E, class F, class G>
	ITask* CreateTaskContextFree(T ref, A a, B b, C c, D d, E e, F f, G g) {
		return new ContextFreeTaskTemplateG<T, A, B, C, D, E, F, G>(ref, a, b, c, d, e, f, g);
	}
#else

	template <typename T, typename... Args>
	class TaskTemplate : public TaskOnce {
	public:
		TaskTemplate(T c, Args&&... args) : callback(c), arguments(std::forward<Args>(args)...) {}

		template <size_t... S>
		void Apply(void* context, bool run, seq<S...>) {
			callback(context, run, std::move(std::get<S>(std::move(arguments)))...);
		}

		virtual void Execute(void* request) override {
			Apply(request, true, gen_seq<sizeof...(Args)>());
			delete this;
		}

		virtual void Abort(void* request) override {
			Apply(request, false, gen_seq<sizeof...(Args)>());
			delete this;
		}

		T callback;
		std::tuple<typename std::decay<Args>::type...> arguments;
	};

	template <typename T, typename... Args>
	ITask* CreateTask(T closure, Args&&... args) {
		return new TaskTemplate<T, Args...>(closure, std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	class ContextFreeTaskTemplate : public TaskOnce {
	public:
		ContextFreeTaskTemplate(T t, Args&&... args) : callback(t), arguments(std::forward<Args>(args)...) {}

		template <size_t... S>
		void Apply(seq<S...>) {
			callback(std::get<S>(arguments)...);
		}

		virtual void Execute(void* request) override {
			Apply(gen_seq<sizeof...(Args)>());
			delete this;
		}

		T callback;
		std::tuple<typename std::decay<Args>::type...> arguments;
	};

	template <typename T, typename... Args>
	ITask* CreateTaskContextFree(T t, Args&&... args) {
		return new ContextFreeTaskTemplate<T, Args...>(t, std::forward<Args>(args)...);
	}

#endif
}


#endif // __ITASK_H__