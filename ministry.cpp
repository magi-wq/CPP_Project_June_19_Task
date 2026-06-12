#include "ministry.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <vector>

using namespace std;

// ==================== IntList ====================
IntList::IntList() : head(nullptr) {}

IntList::~IntList() { 
    clear(); 
}

void IntList::clear() {
    while (head) {
        IntNode* temp = head;
        head = head->next;
        delete temp;
    }
}

void IntList::add(int value) {
    if (!head) {
        head = new IntNode(value);
        return;
    }
    IntNode* current = head;
    while (current->next) {
        current = current->next;
    }
    current->next = new IntNode(value);
}

int IntList::get(int index) const {
    IntNode* current = head;
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
    IntNode* current = head;
    while (current) {
        ++cnt;
        current = current->next;
    }
    return cnt;
}

bool IntList::isEmpty() const { 
    return head == nullptr; 
}

// ==================== Official ====================
Official::Official(int i, int b, int boss) 
    : id(i), bribe(b), bossId(boss) {}

// ==================== Ministry ====================
Ministry::Ministry() 
    : N(0), officials(nullptr), minCostCache(nullptr), bestChild(nullptr), dataLoaded(false) {}

Ministry::~Ministry() { 
    clearMemory(); 
}

void Ministry::clearMemory() {
    if (officials) {
        for (int i = 1; i <= N; ++i) {
            delete officials[i];
        }
        delete[] officials;
        officials = nullptr;
    }
    delete[] minCostCache;
    minCostCache = nullptr;
    delete[] bestChild;
    bestChild = nullptr;
    N = 0;
    dataLoaded = false;
}

int Ministry::readIntFromConsole(const string& prompt, int minVal, int maxVal) {
    int value;
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        if (input.empty()) {
            cout << "Ошибка: строка не может быть пустой.\n";
            continue;
        }
        
        stringstream ss(input);
        if (!(ss >> value)) {
            cout << "Ошибка: необходимо ввести целое число.\n";
            continue;
        }
        
        char remaining;
        if (ss >> remaining) {
            cout << "Ошибка: введены лишние символы после числа.\n";
            continue;
        }
        
        if (value < minVal || value > maxVal) {
            cout << "Ошибка: допустимый диапазон от " << minVal << " до " << maxVal << ".\n";
            continue;
        }
        
        return value;
    }
}

void Ministry::validateAndStore(int n, int* bribes, int* bosses) {
    N = n;
    officials = new Official*[N + 1]();
    minCostCache = new long long[N + 1];
    bestChild = new int[N + 1];

    for (int i = 1; i <= N; ++i) {
        if (bribes[i] < 0 || bribes[i] > 1000000) {
            throw MinistryException("Взятка вне диапазона [0, 1000000]");
        }
        if (bosses[i] < 0 || bosses[i] > N) {
            throw MinistryException("Номер начальника вне диапазона [0, N]");
        }
        if (bosses[i] == i) {
            throw MinistryException("Чиновник не может быть начальником самому себе");
        }
        officials[i] = new Official(i, bribes[i], bosses[i]);
        minCostCache[i] = -1;
        bestChild[i] = 0;
    }

    int rootCount = 0;
    for (int i = 1; i <= N; ++i) {
        if (bosses[i] == 0) {
            rootCount++;
        }
    }
    if (rootCount != 1) {
        throw MinistryException("Должен быть ровно один главный чиновник (начальник = 0)");
    }

    if (isCycleExists()) {
        throw MinistryException("Обнаружен цикл в структуре подчинения");
    }

    buildSubordinationLists();
    dataLoaded = true;
}

void Ministry::inputFromKeyboard() {
    clearMemory();
    
    cout << "\n--- Ввод с клавиатуры ---\n";
    cout << "Введите данные о чиновниках и их начальниках.\n";
    cout << "Нумерация чиновников начинается с 1.\n";
    cout << "Начальник 0 означает главного чиновника.\n\n";
    
    int n = readIntFromConsole("Количество чиновников N (целое число от 1 до 1000): ", 1, 1000);

    int* bribes = new int[n + 1];
    int* bosses = new int[n + 1];
    bool rootAssigned = false;

    cout << "\nВвод взяток:\n";
    for (int i = 1; i <= n; ++i) {
        bribes[i] = readIntFromConsole("Чиновник " + to_string(i) + ", взятка (от 0 до 1000000): ", 0, 1000000);
    }

    cout << "\nВвод начальников:\n";
    for (int i = 1; i <= n; ++i) {
        while (true) {
            int boss = readIntFromConsole("Чиновник " + to_string(i) + ", начальник (0 - главный, 1.." + to_string(n) + "): ", 0, n);
            
            if (boss == 0) {
                if (rootAssigned) {
                    cout << "Ошибка: главный чиновник уже был указан ранее. Попробуйте снова.\n";
                    continue;
                }
                rootAssigned = true;
            }
            
            if (boss == i) {
                cout << "Ошибка: чиновник не может быть начальником самому себе. Попробуйте снова.\n";
                continue;
            }
            
            bosses[i] = boss;
            break;
        }
    }

    if (!rootAssigned) {
        delete[] bribes;
        delete[] bosses;
        throw MinistryException("Не указан главный чиновник (начальник = 0)");
    }

    try {
        validateAndStore(n, bribes, bosses);
        cout << "\nДанные успешно загружены.\n";
        cout << "Нажмите Enter для продолжения...";
        cin.get();
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
    string filename;
    
    cout << "\n--- Загрузка из файла ---\n";
    cout << "Формат файла:\n";
    cout << "  строка 1: количество чиновников N\n";
    cout << "  строка 2: N целых чисел - взятки\n";
    cout << "  строка 3: N целых чисел - начальники\n";
    cout << "Имя файла: ";
    getline(cin, filename);
    
    if (filename.empty()) {
        filename = "input.txt";
    }

    ifstream file(filename);
    if (!file.is_open()) {
        throw MinistryException("Файл не найден или не может быть открыт");
    }

    int n;
    string line;
    
    if (!getline(file, line)) {
        throw MinistryException("Файл пуст");
    }
    stringstream ss1(line);
    if (!(ss1 >> n)) {
        throw MinistryException("В первой строке файла должно быть целое число N");
    }
    if (n < 1 || n > 1000) {
        throw MinistryException("N должно быть от 1 до 1000");
    }

    int* bribes = new int[n + 1];
    int* bosses = new int[n + 1];

    if (!getline(file, line)) {
        delete[] bribes;
        delete[] bosses;
        throw MinistryException("Во второй строке файла должно быть N целых взяток");
    }
    stringstream ss2(line);
    for (int i = 1; i <= n; ++i) {
        if (!(ss2 >> bribes[i])) {
            delete[] bribes;
            delete[] bosses;
            throw MinistryException("Во второй строке файла должно быть N целых взяток");
        }
        if (bribes[i] < 0 || bribes[i] > 1000000) {
            delete[] bribes;
            delete[] bosses;
            throw MinistryException("Взятка вне диапазона [0, 1000000]");
        }
    }

    if (!getline(file, line)) {
        delete[] bribes;
        delete[] bosses;
        throw MinistryException("В третьей строке файла должно быть N целых номеров начальников");
    }
    stringstream ss3(line);
    for (int i = 1; i <= n; ++i) {
        if (!(ss3 >> bosses[i])) {
            delete[] bribes;
            delete[] bosses;
            throw MinistryException("В третьей строке файла должно быть N целых номеров начальников");
        }
        if (bosses[i] < 0 || bosses[i] > n) {
            delete[] bribes;
            delete[] bosses;
            throw MinistryException("Номер начальника вне диапазона [0, N]");
        }
    }

    try {
        validateAndStore(n, bribes, bosses);
        cout << "\nДанные из файла успешно загружены.\n";
        cout << "Нажмите Enter для продолжения...";
        cin.get();
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
    
    cout << "\n--- Случайная генерация ---\n";
    int n = readIntFromConsole("Количество чиновников N (целое число от 1 до 1000): ", 1, 1000);
    
    srand(static_cast<unsigned>(time(nullptr)));

    int* bribes = new int[n + 1];
    int* bosses = new int[n + 1];

    for (int i = 1; i <= n; ++i) {
        bribes[i] = rand() % 1001;
    }

    bosses[1] = 0;
    for (int i = 2; i <= n; ++i) {
        int parent = 1 + rand() % (i - 1);
        bosses[i] = parent;
    }

    try {
        validateAndStore(n, bribes, bosses);
        cout << "\nСлучайное министерство успешно создано.\n";
        cout << "Нажмите Enter для продолжения...";
        cin.get();
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
    for (int i = 1; i <= N; ++i) {
        officials[i]->subordinates.clear();
    }
    for (int i = 1; i <= N; ++i) {
        int boss = officials[i]->bossId;
        if (boss != 0) {
            officials[boss]->subordinates.add(i);
        }
    }
}

bool Ministry::isCycleExists() const {
    vector<int> state(N + 1, 0);
    
    for (int i = 1; i <= N; ++i) {
        if (state[i] != 0) continue;
        
        int cur = i;
        while (true) {
            if (state[cur] == 1) return true;
            if (state[cur] == 2) break;
            state[cur] = 1;
            int next = officials[cur]->bossId;
            if (next == 0) break;
            cur = next;
        }
        
        cur = i;
        while (state[cur] != 2 && state[cur] != 0) {
            state[cur] = 2;
            int next = officials[cur]->bossId;
            if (next == 0) break;
            cur = next;
        }
    }
    return false;
}

void Ministry::validateStructure() const {
    int rootCount = 0;
    for (int i = 1; i <= N; ++i) {
        int bribeVal = officials[i]->bribe;
        if (bribeVal < 0 || bribeVal > 1000000) {
            throw MinistryException("Взятка вне допустимого диапазона");
        }
        int boss = officials[i]->bossId;
        if (boss < 0 || boss > N) {
            throw MinistryException("Номер начальника вне диапазона");
        }
        if (boss == i) {
            throw MinistryException("Чиновник не может быть начальником самому себе");
        }
        if (boss == 0) {
            rootCount++;
        }
    }
    if (rootCount != 1) {
        throw MinistryException("Должен быть ровно один главный чиновник");
    }
}

long long Ministry::computeMinCost(int id) {
    if (minCostCache[id] != -1) {
        return minCostCache[id];
    }
    
    long long bestValue = 0;
    int chosenChild = 0;
    IntList& subs = officials[id]->subordinates;
    
    if (subs.isEmpty()) {
        bestValue = 0;
    } else {
        bool first = true;
        for (int i = 0; i < subs.size(); ++i) {
            int child = subs.get(i);
            long long childCost = computeMinCost(child);
            if (first || childCost < bestValue) {
                bestValue = childCost;
                chosenChild = child;
                first = false;
            }
        }
    }
    
    bestChild[id] = chosenChild;
    minCostCache[id] = static_cast<long long>(officials[id]->bribe) + bestValue;
    return minCostCache[id];
}

void Ministry::restorePath(int id, vector<int>& path) const {
    if (bestChild[id] != 0) {
        restorePath(bestChild[id], path);
    }
    path.push_back(id);
}

void Ministry::printTree(int id, int level) const {
    for (int i = 0; i < level; ++i) {
        cout << "  ";
    }
    cout << "[" << id << "] (взятка=" << officials[id]->bribe << ")\n";
    
    IntList& subs = officials[id]->subordinates;
    for (int i = 0; i < subs.size(); ++i) {
        int child = subs.get(i);
        printTree(child, level + 1);
    }
}

void Ministry::showDataAndSolution() {
    if (!dataLoaded) {
        cout << "\nОшибка: данные ещё не загружены. Сначала выберите пункт 1, 2 или 3.\n";
        cout << "Нажмите Enter для продолжения...";
        cin.get();
        return;
    }

    cout << "\n=== ТЕКУЩАЯ СТРУКТУРА МИНИСТЕРСТВА ===\n";
    cout << "Всего чиновников: " << N << "\n";
    
    int root = -1;
    for (int i = 1; i <= N; ++i) {
        if (officials[i]->bossId == 0) {
            root = i;
            break;
        }
    }
    cout << "Главный чиновник: " << root << "\n";
    
    cout << "\nСписок (id : взятка, начальник):\n";
    for (int i = 1; i <= N; ++i) {
        cout << "  " << i << " : " << officials[i]->bribe << ", начальник=" << officials[i]->bossId << "\n";
    }

    cout << "\nДревовидная структура:\n";
    if (root != -1) {
        printTree(root, 0);
    }

    for (int i = 1; i <= N; ++i) {
        minCostCache[i] = -1;
        bestChild[i] = 0;
    }
    
    long long totalMin = computeMinCost(root);
    vector<int> path;
    restorePath(root, path);

    cout << "\n=== РЕЗУЛЬТАТ ===\n";
    cout << "Минимальная сумма для получения лицензии: " << totalMin << " у.е.\n";
    cout << "Цепочка получения подписей (снизу вверх): ";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) cout << " -> ";
        cout << path[i];
    }
    cout << "\n";
    cout << "\nНажмите Enter для продолжения...";
    cin.get();
}