#pragma once

//-----------------------------------------------------------------------------
// Kahn's algorithm
class Graph
{
public:
	struct Edge
	{
		int from;
		int to;
	};

	Graph(int vertices) : vertices(vertices)
	{
	}

	void AddEdge(int from, int to)
	{
		edges.push_back({ from, to });
	}

	bool Sort()
	{
		vector<int> S;

		for(int i = 0; i < vertices; ++i)
		{
			bool any = false;
			for(auto& e : edges)
			{
				if(e.to == i)
				{
					any = true;
					break;
				}
			}
			if(!any)
				S.push_back(i);
		}

		while(!S.empty())
		{
			int n = S.back();
			S.pop_back();
			result.push_back(n);

			for(auto it = edges.begin(), end = edges.end(); it != end; )
			{
				if(it->from == n)
				{
					int m = it->to;
					it = edges.erase(it);
					end = edges.end();

					bool any = false;
					for(auto& e : edges)
					{
						if(e.to == m)
						{
							any = true;
							break;
						}
					}
					if(!any)
						S.push_back(m);
				}
				else
					++it;
			}
		}

		// if there any edges left, graph has cycles
		return edges.empty();
	}

	vector<int>& GetResult() { return result; }

private:
	vector<int> result;
	vector<Edge> edges;
	int vertices;
};

//-----------------------------------------------------------------------------
// Tree iterator
template<typename T>
struct TreeItem
{
	struct Iterator
	{
	private:
		T* current;
		int index, depth;
		bool up;

	public:
		Iterator(T* current, bool up) : current(current), index(-1), depth(0), up(up) {}

		bool operator != (const Iterator& it) const
		{
			return it.current != current;
		}

		void operator ++ ()
		{
			while(current)
			{
				if(index == -1)
				{
					if(current->childs.empty())
					{
						index = current->index;
						current = current->parent;
						--depth;
					}
					else
					{
						index = -1;
						current = current->childs.front();
						++depth;
						break;
					}
				}
				else
				{
					if(index == current->childs.size())
					{
						index = current->index;
						current = current->parent;
						--depth;
					}
					else if(index + 1 == current->childs.size())
					{
						if(up)
						{
							++index;
							break;
						}
						else
						{
							index = current->index;
							current = current->parent;
							--depth;
						}
					}
					else
					{
						current = current->childs[index + 1];
						index = -1;
						++depth;
						break;
					}
				}
			}
		}

		T* GetCurrent() { return current; }
		int GetDepth() { return depth; }
		bool IsUp() { return index != -1; }
	};

	T* parent;
	vector<T*> childs;
	int index;

	Iterator begin(bool up = false)
	{
		return Iterator(static_cast<T*>(this), up);
	}

	Iterator end()
	{
		return Iterator(nullptr, false);
	}
};