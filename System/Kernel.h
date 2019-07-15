// Kernel.h
// By PaintDream (paintdream@paintdream.com)
// 2019-3-25
// Grid-Based Task Schedule System
//

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "../Interface/ITask.h"
#include "Tiny.h"
#include "TaskQueue.h"
#include "ThreadPool.h"

namespace PaintsNow {
	// Tasks are scheduled by a Tiny with Warp Index.
	// Do not update warp frequently
	class Kernel;
	class WarpTiny : public TReflected<WarpTiny, SharedTiny> {
	public:
		enum {
			WARP_BITS = 8,
			WARP_INDEX_BEGIN = TINY_CUSTOM_BEGIN,
			WARP_INDEX_END = TINY_CUSTOM_BEGIN << WARP_BITS,
			WARP_CUSTOM_BEGIN = WARP_INDEX_END
		};

		void SetWarpIndex(uint32_t warpIndex);
		uint32_t GetWarpIndex() const;
		void AssertWarp(Kernel& kernel) const;
	};

#define CHECK_THREAD(warpTiny) \
	(MUST_CHECK_REFERENCE_ONCE); \
	if (kernel.GetCurrentWarpIndex() != warpTiny->GetWarpIndex()) { \
		request.Error("Threading routine failed on " #warpTiny); \
		return; \
	}

	// All implementations are public, optimize at your own risk
	class Kernel {
	public:
		Kernel(ThreadPool& threadPool, uint32_t warpCount = 0);
		virtual ~Kernel();

		// Common Functions, can be called at any time
		uint32_t GetWarpCount() const;

		// Cleanup all remaining tasks
		void Clear();

		// Current-Warp corelated functions, will be effected by current warp state
		void QueueRoutine(WarpTiny* warpTiny, ITask* task);
		uint32_t GetCurrentWarpIndex() const;
		void YieldCurrentWarp();

		// Operates on other warp
		void SuspendWarp(uint32_t warp);
		bool ResumeWarp(uint32_t warp);

	protected:
		void QueueRoutineInternal(uint32_t toWarpIndex, uint32_t fromThreadIndex, WarpTiny* warpTiny, ITask* task);

	public:
		// Do not touch it unless you know what you are doing.
		class SubTaskQueue : protected TaskQueue {
		public:
			SubTaskQueue(Kernel* kernel, uint32_t idCount);
			SubTaskQueue(const SubTaskQueue& rhs);
			SubTaskQueue& operator = (const SubTaskQueue& rhs);
			virtual ~SubTaskQueue();

			// Take execution atomically, returns true on success
			bool PreemptExecution();
			// Yield execution
			bool YieldExecution();

			// Block/Allow all tasks preemptions, stacked with internally counting.
			void Suspend();
			bool Resume();

			// Commit Execute request to specified thread pool.
			void Flush(ThreadPool& threadPool);

			// Queue task from specified thread.
			void Push(uint32_t fromThreadIndex, ITask* task, WarpTiny* warpTiny);
			const std::vector<TaskQueue::RingBuffer>& GetRingBuffers() const;

		protected:
			friend class Kernel;
			virtual bool InvokeOperation(std::pair<ITask*, void*>& task, void (ITask::*operation)(void*), void* context) override;
			virtual void Execute(void* context) override;
			virtual void Abort(void* context) override;

			Kernel* kernel;
			TAtomic<uint32_t*> threadWarp;
			TAtomic<int32_t> suspendCount;
			TAtomic<int32_t> queueing;
		};

		friend class SubTaskQueue;

		ThreadPool& threadPool;
		std::vector<SubTaskQueue> taskQueueGrid;
#ifdef _DEBUG
		TAtomic<int32_t> activeTaskCount;
#endif
	};

#if defined(_MSC_VER) && _MSC_VER <= 1200
	template <class T>
	class CoTaskTemplate : public TaskOnce {
	public:
		CoTaskTemplate(Kernel& k, T ref) : kernel(k), callback(ref) {
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(request, true);
			kernel.ResumeWarp(warp);

			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false);
			kernel.ResumeWarp(warp);

			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
	};

	template <class T>
	ITask* CreateCoTask(Kernel& k, T ref) {
		return new CoTaskTemplate<T>(ref);
	}

	template <class T, class A>
	class CoTaskTemplateA : public TaskOnce {
	public:
		CoTaskTemplateA(Kernel& k, T ref, A a) : kernel(k), callback(ref) {
			pa = a;

			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}

		virtual void Execute(void* request) override {
			callback(request, true, pa);
			kernel.ResumeWarp(warp);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa);
			kernel.ResumeWarp(warp);
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
	};

	template <class T, class A>
	ITask* CreateCoTask(Kernel& k, T ref, A a) {
		return new CoTaskTemplateA<T, A>(k, ref, a);
	}

	template <class T, class A, class B>
	class CoTaskTemplateB : public TaskOnce {
	public:
		CoTaskTemplateB(Kernel& k, T ref, A a, B b) : kernel(k), callback(ref) {
			pa = a; pb = b;

			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb);
			kernel.ResumeWarp(warp);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb);
			kernel.ResumeWarp(warp);
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
	};

	template <class T, class A, class B>
	ITask* CreateCoTask(Kernel& k, T ref, A a, B b) {
		return new CoTaskTemplateB<T, A, B>(k, ref, a, b);
	}

	template <class T, class A, class B, class C>
	class CoTaskTemplateC : public TaskOnce {
	public:
		CoTaskTemplateC(Kernel& k, T ref, A a, B b, C c) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc);
			kernel.ResumeWarp(warp);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc);
			kernel.ResumeWarp(warp);
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
	};

	template <class T, class A, class B, class C>
	ITask* CreateCoTask(Kernel& k, T ref, A a, B b, C c) {
		return new CoTaskTemplateC<T, A, B, C>(k, ref, a, b, c);
	}

	template <class T, class A, class B, class C, class D>
	class CoTaskTemplateD : public TaskOnce {
	public:
		CoTaskTemplateD(Kernel& k, T ref, A a, B b, C c, D d) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd);
			kernel.ResumeWarp(warp);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd);
			kernel.ResumeWarp(warp);
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
	};

	template <class T, class A, class B, class C, class D>
	ITask* CreateCoTask(Kernel& k, T ref, A a, B b, C c, D d) {
		return new CoTaskTemplateD<T, A, B, C, D>(k, ref, a, b, c, d);
	}

	template <class T, class A, class B, class C, class D, class E>
	class CoTaskTemplateE : public TaskOnce {
	public:
		CoTaskTemplateE(Kernel& k, T ref, A a, B b, C c, D d, E e) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d; pe = e;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd, pe);
			kernel.ResumeWarp(warp);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd, pe);
			kernel.ResumeWarp(warp);
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
	};

	template <class T, class A, class B, class C, class D, class E>
	ITask* CreateCoTask(Kernel& k, T ref, A a, B b, C c, D d, E e) {
		return new CoTaskTemplateE<T, A, B, C, D, E>(k, ref, a, b, c, d, e);
	}

	template <class T, class A, class B, class C, class D, class E, class F>
	class CoTaskTemplateF : public TaskOnce {
	public:
		CoTaskTemplateF(Kernel& k, T ref, A a, B b, C c, D d, E e, F f) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d; pe = e; pf = f;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd, pe, pf);
			kernel.ResumeWarp(warp);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd, pe, pf);
			kernel.ResumeWarp(warp);
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
		typename std::decay<F>::type pf;
	};

	template <class T, class A, class B, class C, class D, class E, class F>
	ITask* CreateCoTask(Kernel& k, T ref, A a, B b, C c, D d, E e, F f) {
		return new CoTaskTemplateF<T, A, B, C, D, E, F>(k, ref, a, b, c, d, e, f);
	}

	template <class T, class A, class B, class C, class D, class E, class F, class G>
	class CoTaskTemplateG : public TaskOnce {
	public:
		CoTaskTemplateG(Kernel& k, T ref, A a, B b, C c, D d, E e, F f, G g) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d; pe = e; pf = f; pg = g;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(request, true, pa, pb, pc, pd, pe, pf, pg);
			kernel.ResumeWarp(warp);
			delete this;
		}
		virtual void Abort(void* request) override {
			callback(request, false, pa, pb, pc, pd, pe, pf, pg);
			kernel.ResumeWarp(warp);
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
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
	ITask* CreateCoTask(Kernel& k, T ref, A a, B b, C c, D d, E e, F f, G g) {
		return new CoTaskTemplateG<T, A, B, C, D, E, F, G>(k, ref, a, b, c, d, e, f, g);
	}

	// ContextFree tasks

	template <class T>
	class ContextFreeCoTaskTemplate : public TaskOnce {
	public:
		ContextFreeCoTaskTemplate(Kernel& k, T ref) : kernel(k), callback(ref) {
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback();
			Abort(request);
		}

		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
	};

	template <class T>
	ITask* CreateCoTaskContextFree(Kernel& k, T ref) {
		return new ContextFreeCoTaskTemplate<T>(ref);
	}

	template <class T, class A>
	class ContextFreeCoTaskTemplateA : public TaskOnce {
	public:
		ContextFreeCoTaskTemplateA(Kernel& k, T ref, A a) : kernel(k), callback(ref) {
			pa = a;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(pa);
			Abort(request);
		}
		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}


		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
	};

	template <class T, class A>
	ITask* CreateCoTaskContextFree(Kernel& k, T ref, A a) {
		return new ContextFreeCoTaskTemplateA<T, A>(k, ref, a);
	}

	template <class T, class A, class B>
	class ContextFreeCoTaskTemplateB : public TaskOnce {
	public:
		ContextFreeCoTaskTemplateB(Kernel& k, T ref, A a, B b) : kernel(k), callback(ref) {
			pa = a; pb = b;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(pa, pb);
			Abort(request);
		}
		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}


		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
	};

	template <class T, class A, class B>
	ITask* CreateCoTaskContextFree(Kernel& k, T ref, A a, B b) {
		return new ContextFreeCoTaskTemplateB<T, A, B>(k, ref, a, b);
	}

	template <class T, class A, class B, class C>
	class ContextFreeCoTaskTemplateC : public TaskOnce {
	public:
		ContextFreeCoTaskTemplateC(Kernel& k, T ref, A a, B b, C c) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(pa, pb, pc);
			Abort(request);
		}
		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}


		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
	};

	template <class T, class A, class B, class C>
	ITask* CreateCoTaskContextFree(Kernel& k, T ref, A a, B b, C c) {
		return new ContextFreeCoTaskTemplateC<T, A, B, C>(k, ref, a, b, c);
	}

	template <class T, class A, class B, class C, class D>
	class ContextFreeCoTaskTemplateD : public TaskOnce {
	public:
		ContextFreeCoTaskTemplateD(Kernel& k, T ref, A a, B b, C c, D d) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd);
			Abort(request);
		}

		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
	};

	template <class T, class A, class B, class C, class D>
	ITask* CreateCoTaskContextFree(Kernel& k, T ref, A a, B b, C c, D d) {
		return new ContextFreeCoTaskTemplateD<T, A, B, C, D>(k, ref, a, b, c, d);
	}

	template <class T, class A, class B, class C, class D, class E>
	class ContextFreeCoTaskTemplateE : public TaskOnce {
	public:
		ContextFreeCoTaskTemplateE(Kernel& k, T ref, A a, B b, C c, D d, E e) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d; pe = e;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd, pe);
			Abort(request);
		}

		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
	};

	template <class T, class A, class B, class C, class D, class E>
	ITask* CreateCoTaskContextFree(Kernel& k, T ref, A a, B b, C c, D d, E e) {
		return new ContextFreeCoTaskTemplateE<T, A, B, C, D, E>(k, ref, a, b, c, d, e);
	}

	template <class T, class A, class B, class C, class D, class E, class F>
	class ContextFreeCoTaskTemplateF : public TaskOnce {
	public:
		ContextFreeCoTaskTemplateF(Kernel& k, T ref, A a, B b, C c, D d, E e, F f) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d; pe = e; pf = f;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd, pe, pf);
			Abort(request);
		}

		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		typename std::decay<A>::type pa;
		typename std::decay<B>::type pb;
		typename std::decay<C>::type pc;
		typename std::decay<D>::type pd;
		typename std::decay<E>::type pe;
		typename std::decay<F>::type pf;
	};

	template <class T, class A, class B, class C, class D, class E, class F>
	ITask* CreateCoTaskContextFree(Kernel& k, T ref, A a, B b, C c, D d, E e, F f) {
		return new ContextFreeCoTaskTemplateF<T, A, B, C, D, E, F>(k, ref, a, b, c, d, e, f);
	}

	template <class T, class A, class B, class C, class D, class E, class F, class G>
	class ContextFreeCoTaskTemplateG : public TaskOnce {
	public:
		ContextFreeCoTaskTemplateG(Kernel& k, T ref, A a, B b, C c, D d, E e, F f, G g) : kernel(k), callback(ref) {
			pa = a; pb = b; pc = c; pd = d; pe = e; pf = f; pg = g;
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}
		virtual void Execute(void* request) override {
			callback(pa, pb, pc, pd, pe, pf, pg);
			Abort(request);
		}

		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
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
	ITask* CreateCoTaskContextFree(Kernel& k, T ref, A a, B b, C c, D d, E e, F f, G g) {
		return new ContextFreeCoTaskTemplateG<T, A, B, C, D, E, F, G>(k, ref, a, b, c, d, e, f, g);
	}
#else
	template <typename T, typename... Args>
	class CoTaskTemplate : public TaskOnce {
	public:
		CoTaskTemplate(Kernel& k, T c, Args&&... args) : kernel(k), callback(c), arguments(std::forward<Args>(args)...) {
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}

		template <size_t... S>
		void Apply(void* context, bool run, seq<S...>) {
			callback(context, run, std::move(std::get<S>(std::move(arguments)))...);
			kernel.ResumeWarp(warp);
		}

		virtual void Execute(void* request) override {
			Apply(request, true, gen_seq<sizeof...(Args)>());
			delete this;
		}

		virtual void Abort(void* request) override {
			Apply(request, false, gen_seq<sizeof...(Args)>());
			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		std::tuple<typename std::decay<Args>::type...> arguments;
	};

	template <typename T, typename... Args>
	ITask* CreateCoTask(Kernel& kernel, T closure, Args&&... args) {
		return new CoTaskTemplate<T, Args...>(kernel, closure, std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	class ContextFreeCoTaskTemplate : public TaskOnce {
	public:
		ContextFreeCoTaskTemplate(Kernel& k, T t, Args&&... args) : kernel(k), callback(t), arguments(std::forward<Args>(args)...) {
			warp = kernel.GetCurrentWarpIndex();
			assert(warp != ~(uint32_t)0);
			kernel.SuspendWarp(warp);
		}

		template <size_t... S>
		void Apply(seq<S...>) {
			callback(std::get<S>(arguments)...);
		}

		virtual void Execute(void* request) override {
			Apply(gen_seq<sizeof...(Args)>());
			Abort(request);
		}

		virtual void Abort(void* request) override {
			kernel.ResumeWarp(warp);

			delete this;
		}

		Kernel& kernel;
		uint32_t warp;
		T callback;
		std::tuple<typename std::decay<Args>::type...> arguments;
	};

	template <typename T, typename... Args>
	ITask* CreateCoTaskContextFree(Kernel& kernel, T t, Args&&... args) {
		return new ContextFreeCoTaskTemplate<T, Args...>(kernel, t, std::forward<Args>(args)...);
	}
#endif
}

#endif // __KERNEL_H__
