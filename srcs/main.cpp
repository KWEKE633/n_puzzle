#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <string_view>

using namespace std;

#define MAX_SIZE 17

#include <chrono>
class Timer {
	chrono::system_clock::time_point start_time;
	public:
		Timer() {
			start_time = chrono::system_clock::now();
		}
		double elapsed() {
			auto now = chrono::system_clock::now();
			return chrono::duration_cast<chrono::microseconds>(now - start_time).count() / 1e3; // ms
		}
};

int dy[] = {-1, 0, 1, 0};
int dx[] = {0, 1, 0, -1};

struct State {
	int size;
	vector<int> board;
};

struct Node {
	vector<int> board;
	uint32_t parents_index;
	int h_cost;
	int g_cost;
	int zero_pos;

	int f(int w) const {
		return w * h_cost + g_cost;
	}
};

class MemoryPool {
	vector<Node> pool;
	public:
		MemoryPool(size_t initial_capacity = 1000000) {
			pool.reserve(initial_capacity);
		}

		uint32_t allocate(vector<int> board, uint32_t parent, int h, int g, int zero) {
			if (pool.size() == pool.capacity()) {
				pool.reserve(pool.capacity() * 2);
			}
			uint32_t index = pool.size();
			pool.push_back({std::move(board), parent, h, g, zero}); // 必ずここを通る！
			return index;
		}

		const Node& get(uint32_t index) const {
			return pool[index];
		}

		Node& get(uint32_t index) {
			return pool[index];
		}

		size_t size() const {
			return pool.size();
		}
};

struct BoardHash {
	size_t operator()(const vector<int>& board) const {
		size_t hash = 14695981039346656037ull; // FNV hash offset
		for (int val : board) {
			hash ^= val;
			hash *= 1099511628211ull; // FNV hash prime
		}
		return hash;
	}
};

struct BoardEqual {
	bool operator()(const vector<int>& a, const vector<int>& b) const {
		return a == b;
	}
};

struct CompareNode {
	MemoryPool* pool;
	int weight;
	CompareNode(MemoryPool* p, int w) : pool(p), weight(w) {}

	bool operator()(uint32_t a, uint32_t b) const {
		const Node& na = pool->get(a);
		const Node& nb = pool->get(b);
		if (na.f(weight) == nb.f(weight)) {
			return na.h_cost > nb.h_cost;
		}
		return na.f(weight) > nb.f(weight);
	}
};

bool loadPuzzle(const string& filename, State& out_state) {
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Error: Cannot open file " << filename << endl;
		return false;
	}
	string line;
	vector<int> numbers;
	int size = -1;

	while (getline(file, line)) {
		if (line.empty()) {
			continue;
		}
		size_t comment_pos = line.find('#');
		if (comment_pos != string::npos) {
			line = line.substr(0, comment_pos);
		}

		stringstream ss(line);
		int val;
		while (ss >> val) {
			if (size == -1) {
				if (val <= 2 || MAX_SIZE <= val) return false;
				size = val;
				out_state.size = size;
			} else {
				if (val <= 2 || MAX_SIZE * MAX_SIZE <= val) return false;
				numbers.push_back(val);
			}
		}
	}
	int total_cells = size * size;
	if ((int)numbers.size() != total_cells) return false;
	vector<bool> seen(total_cells, false);
	for (int i = 0; i < total_cells; ++i) {
		int val = numbers[i];
		if (val < 0 || val >= total_cells) return false;
		if (seen[val]) return false;
		seen[val] = true;
	}
	out_state.board = std::move(numbers);
	return true;
}

bool check(int i, int j, int n) {
	return 0 <= i && i < n && 0 <= j && j < n;
}

class FenwickTree {
	vector<int> tree;
	public:
		FenwickTree(int size) : tree(size + 1, 0) {}

		void add(int i, int delta) {
			for (; i < (int)tree.size(); i += i & -i) {
				tree[i] += delta;
			}
		}

		int query(int i) {
			int sum = 0;
			for (; i > 0; i -= i & -i) {
				sum += tree[i];
			}
			return sum;
		}
};

int countInv(const vector<int>& board) {
	int max_val = board.size();
	FenwickTree bit(max_val);

	int inv = 0;
	int inserted = 0;

	for (int val : board) {
		if (val == 0) continue;
		inv += inserted - bit.query(val);
		bit.add(val, 1);
		++inserted;
	}

	return inv;
}

class Heuristic {
	vector<vector<int>> res;
	vector<pair<int, int>> place;
	public:
		vector<int> vec;
		Heuristic(int n) {
			res.assign(n, vector<int>(n, -1));
			vec.assign(n * n, 0);
			place.assign(n * n, {0, 0});
			char dir = 'R'; // L U R D
			int dy = 0, dx = -1;
			int i = 1;
			while (i <= n * n) {
				int val = (i == n * n) ? 0 : i;
				if (dir == 'R') {
					if (check(dy, dx + 1, n) && res[dy][dx + 1] == -1) {
						++dx;
						res[dy][dx] = val;
					} else {
						dir = 'D';
						continue;
					}
				} else if (dir == 'D') {
					if (check(dy + 1, dx, n) && res[dy + 1][dx] == -1) {
						++dy;
						res[dy][dx] = val;
					} else {
						dir = 'L';
						continue;
					}
				} else if (dir == 'L') {
					if (check(dy, dx - 1, n) && res[dy][dx - 1] == -1) {
						--dx;
						res[dy][dx] = val;
					} else {
						dir = 'U';
						continue;
					}
				} else if (dir == 'U') {
					if (check(dy - 1, dx, n) && res[dy - 1][dx] == -1) {
						--dy;
						res[dy][dx] = val;
					} else {
						dir = 'R';
						continue;
					}
				}
				place[val] = {dy, dx};
				++i;
			}
			for (int y = 0; y < n; ++y) {
				for (int x = 0; x < n; ++x) {
					vec[y * n + x] = res[y][x];
				}
			}
		}

		int linearConflict(const vector<int>& a, int n) {
			int conflict = 0;

			// 1. 行(Row)のコンフリクト判定
			for (int y = 0; y < n; ++y) {
				for (int x1 = 0; x1 < n - 1; ++x1) {
					for (int x2 = x1 + 1; x2 < n; ++x2) {
						int t1 = a[y * n + x1]; // 左側のタイル
						int t2 = a[y * n + x2]; // 右側のタイル
						if (t1 == 0 || t2 == 0) continue;

						// 両方のタイルが「現在の行(y)」を本来の目的地(place.first)としているか？
						if (place[t1].first == y && place[t2].first == y) {
							// 本来のX座標(place.second)が逆転しているか？
							if (place[t1].second > place[t2].second) {
								conflict += 2; // 避けるために+2手必要
							}
						}
					}
				}
				}

				// 2. 列(Column)のコンフリクト判定
				for (int x = 0; x < n; ++x) {
				for (int y1 = 0; y1 < n - 1; ++y1) {
					for (int y2 = y1 + 1; y2 < n; ++y2) {
						int t1 = a[y1 * n + x]; // 上側のタイル
						int t2 = a[y2 * n + x]; // 下側のタイル
						if (t1 == 0 || t2 == 0) continue;

						// 両方のタイルが「現在の列(x)」を本来の目的地としているか？
						if (place[t1].second == x && place[t2].second == x) {
							// 本来のY座標が逆転しているか？
							if (place[t1].first > place[t2].first) {
								conflict += 2;
							}
						}
					}
				}
			}
			return conflict;
		}

		int manhattan(const vector<int>& a, int n) {
			int sum = 0;
			for (int i = 0; i < n * n; ++i) {
				if (a[i] == 0) continue;
				int y = i / n, x = i % n;
				sum += abs(y - place[a[i]].first) + abs(x - place[a[i]].second);
			}
			return sum + linearConflict(a, n);
		}

		int hamming(const vector<int>& a, int n) {
			int misplaced = 0;
			for (int i = 0; i < n * n; ++i) {
				if (a[i] == 0) continue;
				int y = i / n, x = i % n;
				if (y != place[a[i]].first || x != place[a[i]].second) {
					misplaced++;
				}
			}
			return misplaced;
		}

		int euclidean(const vector<int>& a, int n) {
			double sum = 0;
			for (int i = 0; i < n * n; ++i) {
				if (a[i] == 0) continue;
				int y = i / n, x = i % n;
				double dy = y - place[a[i]].first;
				double dx = x - place[a[i]].second;
				sum += sqrt(dy * dy + dx * dx);
			}
			return (int)sum;
		}
};

bool checkSolvable(State& init, Heuristic& md) {
	int I_init = countInv(init.board);
	int I_target = countInv(md.vec);
	int R_init = 0, R_target = 0;
	// bool ok = false;
	for (int i = 0; i < (int)init.board.size(); ++i) {
		if (init.board[i] == 0) {
			R_init = i / init.size;
			// if (ok) break;
			// ok = true;
		}
		if (md.vec[i] == 0) {
			R_target = i / init.size;
			// if (ok) break;
			// ok = true;
		}
	}
	if (init.size % 2 == 1) {
		if (I_init % 2 == I_target % 2) return true;
		else return false;
	} else {
		if ((I_init + R_init) % 2 == (I_target + R_target) % 2) return true;
		else return false;
	}
}

template <typename HeuristicFunc>
void solvePuzzle(State init, HeuristicFunc h_func, int weight = 1) {
	MemoryPool mp;
	CompareNode comp(&mp, weight);
	priority_queue<uint32_t, vector<uint32_t>, CompareNode> open_set(comp);
	unordered_set<vector<int>, BoardHash, BoardEqual> closed_set;

	int zero = -1;
	for (int i = 0; i < (int)init.board.size(); ++i) {
		if (init.board[i] == 0) {
			zero = i;
			break;
		}
	}
	int initial_h = h_func(init.board, init.size);
	uint32_t start_idx = mp.allocate(init.board, UINT32_MAX, initial_h, 0, zero);
	open_set.push(start_idx);

	uint32_t goal_idx = UINT32_MAX;

	while (!open_set.empty()) {
		uint32_t current_idx = open_set.top();
		open_set.pop();

		// const Node& nd = mp.get(current_idx);

		vector<int> current_board = mp.get(current_idx).board;
		int current_g = mp.get(current_idx).g_cost;
		int current_zero = mp.get(current_idx).zero_pos;
		int current_h = mp.get(current_idx).h_cost;
		if (current_h == 0) {
			goal_idx = current_idx;
			break;
		}
		if (closed_set.contains(current_board)) continue;
		closed_set.insert(current_board);
		int z_y = current_zero / init.size;
		int z_x = current_zero % init.size;

		for (int i = 0; i < 4; ++i) {
			int ny = z_y + dy[i];
            int nx = z_x + dx[i];

			if (!check(ny, nx, init.size)) continue;

			int new_zero = ny * init.size + nx;

			vector<int> neighbor_board = current_board;
			swap(neighbor_board[current_zero], neighbor_board[new_zero]);
			if (closed_set.contains(neighbor_board)) continue;

			int new_g = current_g + 1;
            int new_h = h_func(neighbor_board, init.size);

			uint32_t neighbor_idx = mp.allocate(neighbor_board, current_idx, new_h, new_g, new_zero);
            open_set.push(neighbor_idx);	
		}
	}

	if (goal_idx != UINT32_MAX) {
		vector<uint32_t> path;
		uint32_t curr = goal_idx;

		while (curr != UINT32_MAX) {
			path.push_back(curr);
			curr = mp.get(curr).parents_index;
		}

		cout << "\n==========================================" << endl;
		cout << "            🎯 SOLUTION PATH 🎯            " << endl;
		cout << "==========================================" << endl;

		int move_cnt = 0;
		for (int p = (int)path.size() - 1; p >= 0; --p) {
			uint32_t node_idx = path[p];
			const Node& node = mp.get(node_idx);

			if (move_cnt == 0) {
				cout << "\n[📊 Initial State]" << endl;
			} else {
				cout << "\n[➡️ Move " << move_cnt << "]" << endl;
			}

			for (int y = 0; y < init.size; ++y) {
				for (int x = 0; x < init.size; ++x) {
					int val = node.board[y * init.size + x];
					if (val == 0) cout << "0\t";
					else cout << val << "\t";
				}
				cout << "\n";
			}
			cout << "------------------------------------------" << endl;
			move_cnt++;
		}

		cout << "\n[🏁 Search Summary]" << endl;
		cout << "Total moves (Solution depth) : " << path.size() - 1 << endl;
		cout << "Time complexity (Total states selected): " << closed_set.size() << endl;
		cout << "Size complexity (Max states in memory): " << mp.size() << endl;
	}
}

int main(int argc, char **argv) {
	int idx = -1, weight = 1;
	char c = 'm'; // default は マンハッタン
	string_view arg1(argv[1]);
	if (argc == 2) {
		idx = 1;
	} else if (argc == 3) {
		idx = 2;
		if (arg1 == "-m") c = 'm';
		else if (arg1 == "-h") c = 'h';
		else if (arg1 == "-e") c = 'e';
		else {
			cerr << "Usage: " << argv[1] << " option not found." << endl;
			return 1;
		}
	} else if (argc == 4) {
		idx = 3;
		if (arg1 == "-m") c = 'm';
		else if (arg1 == "-h") c = 'h';
		else if (arg1 == "-e") c = 'e';
		else {
			cerr << "Usage: " << argv[1] << " option not found." << endl;
			return 1;
		}
		string_view arg2(argv[2]);
		if (arg2 == "-g")  weight = 5; // 調整可能
		else {
			cerr << "Usage: " << argv[2] << " not found." << endl;
			return 1;
		}
	}
	if (idx == -1) {
		cerr << "Usage: " << "Invalid argment number." << endl;
		return 1;
	}
	State init;
	if (!loadPuzzle(argv[idx], init)) {
		return 1;
	}
	Timer timer;
	Heuristic md(init.size);
	if (!checkSolvable(init, md)) {
		cout << "Oh, Cannot solve puzzle!" << endl;
		return 0;
	}
	if (c == 'm') {
		solvePuzzle(init, [&md](const vector<int>& b, int n) {
			return md.manhattan(b, n);
		}, weight);
	} 
	else if (c == 'e') {
		solvePuzzle(init, [&md](const vector<int>& b, int n) {
			return md.euclidean(b, n);
		}, weight);
	} 
	else {
		solvePuzzle(init, [&md](const vector<int>& b, int n) {
			return md.hamming(b, n);
		}, weight);
	}
	cout << "Execution Time: " << timer.elapsed() << "ms" << endl;
	return 0;
}
