#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

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

struct State {
	int size;
	vector<int> board;
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
			res.assign(n, vector<int>(n, 0));
			vec.assign(n * n, 0);
			place.assign(n * n, {0, 0});
			char dir = 'R'; // L U R D
			int dy = 0, dx = -1;
			int i = 1;
			while (i < n * n) {
				if (dir == 'R') {
					if (check(dy, dx + 1, n) && res[dy][dx + 1] == 0) {
						++dx;
						res[dy][dx] = i;
					} else {
						dir = 'D';
						continue;
					}
				} else if (dir == 'D') {
					if (check(dy + 1, dx, n) && res[dy + 1][dx] == 0) {
						++dy;
						res[dy][dx] = i;
					} else {
						dir = 'L';
						continue;
					}
				} else if (dir == 'L') {
					if (check(dy, dx - 1, n) && res[dy][dx - 1] == 0) {
						--dx;
						res[dy][dx] = i;
					} else {
						dir = 'U';
						continue;
					}
				} else if (dir == 'U') {
					if (check(dy - 1, dx, n) && res[dy - 1][dx] == 0) {
						--dy;
						res[dy][dx] = i;
					} else {
						dir = 'R';
						continue;
					}
				}
				place[i] = {dy, dx};
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
				sum += abs(y - place[a[i]].first + x - place[a[i]].second);
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
    // TODO: 2. A* 探索アルゴリズムの実行
    // TODO: 3. 結果の出力
	cout << "Execution Time: " << timer.elapsed() << "ms" << endl;
	return 0;
}
