#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <queue>
#include <unordered_set>

using namespace std;

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

	int f() const {
		return h_cost + g_cost;
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
	CompareNode(MemoryPool* p) : pool(p) {}

	bool operator()(uint32_t a, uint32_t b) const {
		const Node& na = pool->get(a);
		const Node& nb = pool->get(b);
		if (na.f() == nb.f()) {
			return na.h_cost > nb.h_cost;
		}
		return na.f() > nb.f();
	}
};
// 使い方:
// CompareNode comp(&pool);
// std::priority_queue<uint32_t, vector<uint32_t>, CompareNode> open_set(comp);

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
				size = val;
				out_state.size = size;
			} else {
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

class ManhattanDistance {
	vector<vector<int>> res;
	vector<pair<int, int>> place;
	public:
		vector<int> vec;
		ManhattanDistance(int n) {
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
		int dist(vector<int>& a, int n) {
			int sum = 0;
			for (int i = 0; i < n * n; ++i) {
				if (a[i] == 0) continue;
				int y = i / n, x = i % n;
				sum += abs(y - place[a[i]].first) + abs(x - place[a[i]].second);
			}
			return sum;
		}
};

bool checkSolvable(State& init, ManhattanDistance& md) {
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

void solvePuzzle(State init, ManhattanDistance md) {
	MemoryPool mp;
	CompareNode comp(&mp);
	priority_queue<uint32_t, vector<uint32_t>, CompareNode> open_set(comp);
	unordered_set<vector<int>, BoardHash, BoardEqual> closed_set;

	int zero = -1;
	for (int i = 0; i < (int)init.board.size(); ++i) {
		if (init.board[i] == 0) {
			zero = i;
			break;
		}
	}
	uint32_t start_idx = mp.allocate(init.board, UINT32_MAX, md.dist(init.board, init.size), 0, zero);
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
            int new_h = md.dist(neighbor_board, init.size);

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

		cout << "Solved in " << path.size() - 1 << " moves!" << endl;
		cout << "Time complexity (Total states selected): " << closed_set.size() << endl;
		cout << "Size complexity (Max states in memory): " << mp.size() << endl;
	}
}

int main(int argc, char ** argv) {
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <puzzle_file>" << endl;
		return 1;
	}
	State init;
	if (!loadPuzzle(argv[1], init)) {
		return 1;
	}
	Timer timer;
	ManhattanDistance md(init.size);
	if (!checkSolvable(init, md)) {
		cout << "Oh, Cannot solve puzzle!" << endl;
		return 0;
	}
	// TODO: 1. 解の存在判定
	solvePuzzle(init, md);
    // TODO: 2. A* 探索アルゴリズムの実行
    // TODO: 3. 結果の出力
	cout << "Execution Time: " << timer.elapsed() << "ms" << endl;
	return 0;
}
