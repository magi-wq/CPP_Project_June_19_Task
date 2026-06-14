#include "ministry.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

// ==================== IntList ====================
IntList::IntList() : head_(nullptr) {}

IntList::~IntList() {
  clear();
}

void IntList::clear() {
  while (head_) {
    IntNode* temp = head_;
    head_ = head_->next;
    delete temp;
  }
}

void IntList::add(int value) {
  if (!head_) {
    head_ = new IntNode(value);
    return;
  }
  IntNode* current = head_;
  while (current->next) {
    current = current->next;
  }
  current->next = new IntNode(value);
}

int IntList::get(int index) const {
  IntNode* current = head_;
  for (int i = 0; i < index && current; ++i) {
    current = current->next;
  }
  if (!current) {
    throw MinistryException("Выход за границы списка подчинённых");
  }
  return current->data;
}

int IntList::size() const {
  int cnt = 0;
  IntNode* current = head_;
  while (current) {
    ++cnt;
    current = current->next;
  }
  return cnt;
}

bool IntList::isEmpty() const {
  return head_ == nullptr;
}

// ==================== Official ====================
Official::Official(int i, int b, int boss)
    : id(i), bribe(b), bossId(boss) {}

// ==================== Ministry ====================
Ministry::Ministry()
    : N_(0),
      officials_(nullptr),
      min_cost_cache_(nullptr),
      best_child_(nullptr),
      data_loaded_(false) {}

Ministry::~Ministry() {
  clearMemory();
}

void Ministry::clearMemory() {
  if (officials_) {
    for (int i = 1; i <= N_; ++i) {
      delete officials_[i];
    }
    delete[] officials_;
    officials_ = nullptr;
  }
  delete[] min_cost_cache_;
  min_cost_cache_ = nullptr;
  delete[] best_child_;
  best_child_ = nullptr;
  N_ = 0;
  data_loaded_ = false;
}

int Ministry::readIntFromConsole(const std::string& prompt, int min_val,
                                  int max_val) {
  int value;
  std::string input;
  while (true) {
    std::cout << prompt;
    std::getline(std::cin, input);

    if (input.empty()) {
      std::cout << "Ошибка: строка не может быть пустой.\n";
      continue;
    }

    std::stringstream ss(input);
    if (!(ss >> value)) {
      std::cout << "Ошибка: необходимо ввести целое число.\n";
      continue;
    }

    char remaining;
    if (ss >> remaining) {
      std::cout << "Ошибка: введены лишние символы после числа.\n";
      continue;
    }

    if (value < min_val || value > max_val) {
      std::cout << "Ошибка: допустимый диапазон от " << min_val << " до "
                << max_val << ".\n";
      continue;
    }

    return value;
  }
}

void Ministry::validateAndStore(int n, int* bribes, int* bosses) {
  N_ = n;
  officials_ = new Official*[N_ + 1]();
  min_cost_cache_ = new long long[N_ + 1];
  best_child_ = new int[N_ + 1];

  for (int i = 1; i <= N_; ++i) {
    if (bribes[i] < 0 || bribes[i] > 1000000) {
      throw MinistryException("Взятка вне диапазона [0, 1000000]");
    }
    if (bosses[i] < 0 || bosses[i] > N_) {
      throw MinistryException("Номер начальника вне диапазона [0, N]");
    }
    if (bosses[i] == i) {
      throw MinistryException("Чиновник не может быть начальником самому себе");
    }
    officials_[i] = new Official(i, bribes[i], bosses[i]);
    min_cost_cache_[i] = -1;
    best_child_[i] = 0;
  }

  int root_count = 0;
  for (int i = 1; i <= N_; ++i) {
    if (bosses[i] == 0) {
      root_count++;
    }
  }
  if (root_count != 1) {
    throw MinistryException(
        "Должен быть ровно один главный чиновник (начальник = 0)");
  }

  if (isCycleExists()) {
    throw MinistryException("Обнаружен цикл в структуре подчинения");
  }

  buildSubordinationLists();
  data_loaded_ = true;
}

void Ministry::inputFromKeyboard() {
  clearMemory();

  std::cout << "\n--- Ввод с клавиатуры ---\n";
  std::cout << "Введите данные о чиновниках и их начальниках.\n";
  std::cout << "Нумерация чиновников начинается с 1.\n";
  std::cout << "Начальник 0 означает главного чиновника.\n\n";

  int n = readIntFromConsole("Количество чиновников N (целое число от 1 до 1000): ",
                              1, 1000);

  int* bribes = new int[n + 1];
  int* bosses = new int[n + 1];
  bool root_assigned = false;

  std::cout << "\nВвод взяток:\n";
  for (int i = 1; i <= n; ++i) {
    bribes[i] = readIntFromConsole(
        "Чиновник " + std::to_string(i) + ", взятка (от 0 до 1000000): ", 0,
        1000000);
  }

  std::cout << "\nВвод начальников:\n";
  for (int i = 1; i <= n; ++i) {
    while (true) {
      int boss = readIntFromConsole(
          "Чиновник " + std::to_string(i) +
              " (0 - главный, 1.." + std::to_string(n) + "): ",
          0, n);

      if (boss == 0) {
        if (root_assigned) {
          std::cout << "Ошибка: главный чиновник уже был указан ранее. "
                       "Попробуйте снова.\n";
          continue;
        }
        root_assigned = true;
      }

      if (boss == i) {
        std::cout << "Ошибка: чиновник не может быть начальником самому себе. "
                     "Попробуйте снова.\n";
        continue;
      }

      bosses[i] = boss;
      break;
    }
  }

  if (!root_assigned) {
    delete[] bribes;
    delete[] bosses;
    throw MinistryException("Не указан главный чиновник (начальник = 0)");
  }

  try {
    validateAndStore(n, bribes, bosses);
    std::cout << "\nДанные успешно загружены.\n";
    std::cout << "Нажмите Enter для продолжения...";
    std::cin.get();
  } catch (...) {
    delete[] bribes;
    delete[] bosses;
    clearMemory();
    throw;
  }

  delete[] bribes;
  delete[] bosses;
}

void Ministry::loadFromFile() {
  clearMemory();
  std::string filename;

  std::cout << "\n--- Загрузка из файла ---\n";
  std::cout << "Формат файла:\n";
  std::cout << "  строка 1: количество чиновников N\n";
  std::cout << "  строка 2: N целых чисел - взятки\n";
  std::cout << "  строка 3: N целых чисел - начальники\n";
  std::cout << "Имя файла: ";
  std::getline(std::cin, filename);

  if (filename.empty()) {
    filename = "input.txt";
  }

  std::ifstream file(filename);
  if (!file.is_open()) {
    throw MinistryException("Файл не найден или не может быть открыт");
  }

  int n;
  std::string line;

  if (!std::getline(file, line)) {
    throw MinistryException("Файл пуст");
  }
  std::stringstream ss1(line);
  if (!(ss1 >> n)) {
    throw MinistryException("В первой строке файла должно быть целое число N");
  }
  if (n < 1 || n > 1000) {
    throw MinistryException("N должно быть от 1 до 1000");
  }

  int* bribes = new int[n + 1];
  int* bosses = new int[n + 1];

  if (!std::getline(file, line)) {
    delete[] bribes;
    delete[] bosses;
    throw MinistryException(
        "Во второй строке файла должно быть N целых взяток");
  }
  std::stringstream ss2(line);
  for (int i = 1; i <= n; ++i) {
    if (!(ss2 >> bribes[i])) {
      delete[] bribes;
      delete[] bosses;
      throw MinistryException(
          "Во второй строке файла должно быть N целых взяток");
    }
    if (bribes[i] < 0 || bribes[i] > 1000000) {
      delete[] bribes;
      delete[] bosses;
      throw MinistryException("Взятка вне диапазона [0, 1000000]");
    }
  }

  if (!std::getline(file, line)) {
    delete[] bribes;
    delete[] bosses;
    throw MinistryException(
        "В третьей строке файла должно быть N целых номеров начальников");
  }
  std::stringstream ss3(line);
  for (int i = 1; i <= n; ++i) {
    if (!(ss3 >> bosses[i])) {
      delete[] bribes;
      delete[] bosses;
      throw MinistryException(
          "В третьей строке файла должно быть N целых номеров начальников");
    }
    if (bosses[i] < 0 || bosses[i] > n) {
      delete[] bribes;
      delete[] bosses;
      throw MinistryException("Номер начальника вне диапазона [0, N]");
    }
  }

  try {
    validateAndStore(n, bribes, bosses);
    std::cout << "\nДанные из файла успешно загружены.\n";
    std::cout << "Нажмите Enter для продолжения...";
    std::cin.get();
  } catch (...) {
    delete[] bribes;
    delete[] bosses;
    clearMemory();
    throw;
  }

  delete[] bribes;
  delete[] bosses;
}

void Ministry::generateRandom() {
  clearMemory();

  std::cout << "\n--- Случайная генерация ---\n";
  int n = readIntFromConsole(
      "Количество чиновников N (целое число от 1 до 1000): ", 1, 1000);

  std::srand(static_cast<unsigned>(std::time(nullptr)));

  int* bribes = new int[n + 1];
  int* bosses = new int[n + 1];

  for (int i = 1; i <= n; ++i) {
    bribes[i] = std::rand() % 1001;
  }

  bosses[1] = 0;
  for (int i = 2; i <= n; ++i) {
    int parent = 1 + std::rand() % (i - 1);
    bosses[i] = parent;
  }

  try {
    validateAndStore(n, bribes, bosses);
    std::cout << "\nСлучайное министерство успешно создано.\n";
    std::cout << "Нажмите Enter для продолжения...";
    std::cin.get();
  } catch (...) {
    delete[] bribes;
    delete[] bosses;
    clearMemory();
    throw;
  }

  delete[] bribes;
  delete[] bosses;
}

void Ministry::buildSubordinationLists() {
  for (int i = 1; i <= N_; ++i) {
    officials_[i]->subordinates.clear();
  }
  for (int i = 1; i <= N_; ++i) {
    int boss = officials_[i]->bossId;
    if (boss != 0) {
      officials_[boss]->subordinates.add(i);
    }
  }
}

bool Ministry::isCycleExists() const {
  std::vector<int> state(N_ + 1, 0);

  for (int i = 1; i <= N_; ++i) {
    if (state[i] != 0) continue;

    int cur = i;
    while (true) {
      if (state[cur] == 1) return true;
      if (state[cur] == 2) break;
      state[cur] = 1;
      int next = officials_[cur]->bossId;
      if (next == 0) break;
      cur = next;
    }

    cur = i;
    while (state[cur] != 2 && state[cur] != 0) {
      state[cur] = 2;
      int next = officials_[cur]->bossId;
      if (next == 0) break;
      cur = next;
    }
  }
  return false;
}

void Ministry::validateStructure() const {
  int root_count = 0;
  for (int i = 1; i <= N_; ++i) {
    int bribe_val = officials_[i]->bribe;
    if (bribe_val < 0 || bribe_val > 1000000) {
      throw MinistryException("Взятка вне допустимого диапазона");
    }
    int boss = officials_[i]->bossId;
    if (boss < 0 || boss > N_) {
      throw MinistryException("Номер начальника вне диапазона");
    }
    if (boss == i) {
      throw MinistryException(
          "Чиновник не может быть начальником самому себе");
    }
    if (boss == 0) {
      root_count++;
    }
  }
  if (root_count != 1) {
    throw MinistryException("Должен быть ровно один главный чиновник");
  }
}

long long Ministry::computeMinCost(int id) {
  if (min_cost_cache_[id] != -1) {
    return min_cost_cache_[id];
  }

  long long best_value = 0;
  int chosen_child = 0;
  IntList& subs = officials_[id]->subordinates;

  if (subs.isEmpty()) {
    best_value = 0;
  } else {
    bool first = true;
    for (int i = 0; i < subs.size(); ++i) {
      int child = subs.get(i);
      long long child_cost = computeMinCost(child);
      if (first || child_cost < best_value) {
        best_value = child_cost;
        chosen_child = child;
        first = false;
      }
    }
  }

  best_child_[id] = chosen_child;
  min_cost_cache_[id] = static_cast<long long>(officials_[id]->bribe) + best_value;
  return min_cost_cache_[id];
}

void Ministry::restorePath(int id, std::vector<int>& path) const {
  if (best_child_[id] != 0) {
    restorePath(best_child_[id], path);
  }
  path.push_back(id);
}

void Ministry::printTree(int id, int level) const {
  for (int i = 0; i < level; ++i) {
    std::cout << "  ";
  }
  std::cout << "[" << id << "] (взятка=" << officials_[id]->bribe << ")\n";

  IntList& subs = officials_[id]->subordinates;
  for (int i = 0; i < subs.size(); ++i) {
    int child = subs.get(i);
    printTree(child, level + 1);
  }
}

void Ministry::showDataAndSolution() {
  if (!data_loaded_) {
    std::cout << "\nОшибка: данные ещё не загружены. "
                 "Сначала выберите пункт 1, 2 или 3.\n";
    std::cout << "Нажмите Enter для продолжения...";
    std::cin.get();
    return;
  }

  std::cout << "\n=== ТЕКУЩАЯ СТРУКТУРА МИНИСТЕРСТВА ===\n";
  std::cout << "Всего чиновников: " << N_ << "\n";

  int root = -1;
  for (int i = 1; i <= N_; ++i) {
    if (officials_[i]->bossId == 0) {
      root = i;
      break;
    }
  }
  std::cout << "Главный чиновник: " << root << "\n";

  std::cout << "\nСписок (id : взятка, начальник):\n";
  for (int i = 1; i <= N_; ++i) {
    std::cout << "  " << i << " : " << officials_[i]->bribe
              << ", начальник=" << officials_[i]->bossId << "\n";
  }

  std::cout << "\nДревовидная структура:\n";
  if (root != -1) {
    printTree(root, 0);
  }

  for (int i = 1; i <= N_; ++i) {
    min_cost_cache_[i] = -1;
    best_child_[i] = 0;
  }

  long long total_min = computeMinCost(root);
  std::vector<int> path;
  restorePath(root, path);

  std::cout << "\n=== РЕЗУЛЬТАТ ===\n";
  std::cout << "Минимальная сумма для получения лицензии: " << total_min
            << " у.е.\n";
  std::cout << "Цепочка получения подписей (снизу вверх): ";
  for (std::size_t i = 0; i < path.size(); ++i) {
    if (i > 0) std::cout << " -> ";
    std::cout << path[i];
  }
  std::cout << "\n";
  std::cout << "\nНажмите Enter для продолжения...";
  std::cin.get();
}