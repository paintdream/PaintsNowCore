// Graph.h
// By PaintDream (paintdream@paintdream.com)
// 2018-7-30
//

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "Tiny.h"
#include <set>
#include <map>
#include <sstream>

namespace PaintsNow {
	template <class N>
	class Graph;

	template <class P>
	class GraphPort : public TReflected<GraphPort<P>, P> {
	public:
		typedef TReflected<GraphPort<P>, P> BaseClass;
		GraphPort(Tiny* n = nullptr) : node(n) {}
		void Link(GraphPort* p, Tiny::FLAG f = Tiny::TINY_PINNED, Tiny::FLAG t = 0) {
			targetPorts[p] = f;
			p->targetPorts[this] = t;
		}

		void UnLink(GraphPort* p) {
			targetPorts.erase(p);
			p->targetPorts.erase(this);
		}

		void Cleanup() {
			for (typename std::map<GraphPort*, Tiny::FLAG>::iterator it = targetPorts.begin(); it != targetPorts.end(); ++it) {
				it->first->targetPorts.erase(this);
			}

			targetPorts.clear();
		}

		TObject<IReflect>& operator () (IReflect& reflect) {
			BaseClass::operator () (reflect);
			return *this;
		}

		Tiny* GetNode() const {
			return node;
		}

		void SetNode(Tiny* n) {
			node = n;
		}

		typedef std::map<GraphPort*, Tiny::FLAG> PortMap;
		const std::map<GraphPort*, Tiny::FLAG>& GetTargetPortMap() const {
			return targetPorts;
		}

	protected:
		Tiny* node;
		std::map<GraphPort*, Tiny::FLAG> targetPorts;
	};

	// T Must be derived from Tiny
	template <class T, class P>
	class GraphNode : public T {
	public:
		typedef P Port;
	private:
		class PortsReflector : public IReflect {
		public:
			PortsReflector(GraphNode* t, std::map<String, Port*>& ports, bool r) : IReflect(true, false), thisNode(t), nodePorts(ports), recursive(r) {}
			virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
				if (!s.IsBasicObject()) {
					if (s.IsIterator()) {
						IIterator& it = static_cast<IIterator&>(s);
						Unique subType = it.GetPrototypeUnique();
						Unique refSubType = it.GetPrototypeReferenceUnique();
						if (subType != refSubType && refSubType->IsClass(UniqueType<Port>::Get())) {
							size_t i = 0;
							while (it.Next()) {
								std::stringstream ss;
								ss << name << "[" << i++ << "]";
								TShared<Port>& p = *reinterpret_cast<TShared<Port>*>(it.Get());
								assert(p);

								Property(*p(), subType, it.GetPrototypeReferenceUnique(), ss.str().c_str(), ptr, &p, nullptr);
							}
						}
					} else {
						Port* p = s.QueryInterface(UniqueType<Port>());
						if (p != nullptr) {
							p->SetNode(thisNode);
							nodePorts[path + name] = p;
						} else if (recursive) {
							String orgPath = path;
							path = path + name + ".";
							s(*this);
							path = orgPath;
						}
					}
				}
			}

			virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta) {}

		private:
			std::map<String, Port*>& nodePorts;
			GraphNode* thisNode;
			String path;
			bool recursive;
		};

	public:
		void ReflectNodePorts(bool recursive = false) {
			// use reflected tech to get node ports
			std::map<String, Port*> newNodePorts;
			PortsReflector reflector(this, newNodePorts, recursive);
			(*this)(reflector);
			std::swap(nodePorts, newNodePorts);
		}

		void Cleanup() {
			for (typename std::map<String, Port*>::iterator it = nodePorts.begin(); it != nodePorts.end(); ++it) {
				it->second->Cleanup();
			}
		}

		Port* operator [] (const String& key) const {
			typename std::map<String, Port*>::const_iterator p = nodePorts.find(key);
			return p == nodePorts.end() ? nullptr : p->second;
		}

		const std::map<String, Port*>& GetPortMap() const {
			return nodePorts;
		}

	protected:
		std::map<String, Port*> nodePorts;
	};

	template <class N>
	class Graph {
	public:
		typedef N Node;
		typedef typename N::Port Port;

		void AddNode(Node* node) {
			node->ReferenceObject();
			std::binary_insert(allNodes, node);
		}

		void RemoveNode(Node* node) {
			node->Cleanup();
			std::binary_erase(allNodes, node);
			node->ReleaseObject();
		}

		bool HasNode(Node* node) {
			return std::binary_find(allNodes.begin(), allNodes.end(), node) != allNodes.end();
		}

		~Graph() {
			for (size_t i = 0; i < allNodes.size(); i++) {
				Node* node = allNodes[i];
				node->Cleanup();
				node->ReleaseObject();
			}
		}

		// removes all nodes
		template <class F>
		void Optimize(F f) {
			std::vector<Node*> importantNodes;
			std::vector<Node*> finalNodes;

			for (size_t i = 0; i < allNodes.size(); i++) {
				Node* n = allNodes[i];
				if (f(n)) {
					importantNodes.push_back(n);
				}
			}

			while (!importantNodes.empty()) {
				std::vector<Node*> newNodes;
				for (size_t i = 0; i < importantNodes.size(); i++) {
					Node* node = importantNodes[i];
					for (typename std::map<String, Port*>::const_iterator pt = node->GetPortMap().begin(); pt != node->GetPortMap().end(); ++pt) {
						Port* port = pt->second;
						for (typename std::map<Port*, Tiny::FLAG>::const_iterator qt = port->GetTargetPortMap().begin(); qt != port->GetTargetPortMap().end(); ++qt) {
							Node* p = static_cast<Node*>(qt->first->GetNode());

							if (std::binary_find(finalNodes.begin(), finalNodes.end(), p) == finalNodes.end()) {
								std::binary_insert(finalNodes, p);
								newNodes.push_back(p);
							}
						}
					}
				}

				std::swap(newNodes, importantNodes);
			}

			// removed all unreferenced nodes, quickly!
			for (size_t j = 0; j < allNodes.size(); j++) {
				Node* node = allNodes[j];

				if (std::binary_find(finalNodes.begin(), finalNodes.end(), node) == finalNodes.end()) {
					node->Cleanup();
					node->ReleaseObject();
				}
			}

			std::swap(finalNodes, allNodes);
		}

		template <class F, class B>
		bool IterateTopological(F& t, B& b) const {
			std::vector<Node*> preNodes = allNodes;

			while (!preNodes.empty() && b()) {
				std::vector<Node*> lockedNodes;
				for (size_t i = 0; i < preNodes.size(); i++) {
					Node* node = preNodes[i];
					for (typename std::map<String, Port*>::const_iterator pt = node->GetPortMap().begin(); pt != node->GetPortMap().end(); ++pt) {
						Port* port = pt->second;
						for (typename Port::PortMap::const_iterator qt = port->GetTargetPortMap().begin(); qt != port->GetTargetPortMap().end(); ++qt) {
							if (qt->second & Tiny::TINY_PINNED) {
								Port* targetPort = static_cast<Port*>(qt->first);
								std::binary_insert(lockedNodes, static_cast<Node*>(targetPort->GetNode()));
							}
						}
					}
				}

				// no more available nodes, got cycle. exit.
				if (preNodes.size() == lockedNodes.size()) {
					return false;
				}

				for (size_t j = 0; j < preNodes.size(); j++) {
					Node* node = preNodes[j];
					if (std::binary_find(lockedNodes.begin(), lockedNodes.end(), node) == lockedNodes.end()) {
						if (!t(node)) {
							return false;
						}
					}
				}

				std::swap(lockedNodes, preNodes);
			}

			return true;
		}

	protected:
		std::vector<Node*> allNodes;
	};
}


#endif // __GRAPH_H__