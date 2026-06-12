#include <iostream>
#include <limits>
#include <string>
#include "ministry.h"

using namespace std;

int main() {
    system("chcp 1251 > nul");
    
    Ministry ministry;
    int choice;

    do {
        // Очистка экрана перед выводом меню
        system("cls");
        
        cout << "\n=== ЧИНОВНИКИ ===\n";
        cout << "Минимальная стоимость получения лицензии\n";
        
        if (ministry.isDataLoaded()) {
            cout << "\nСтатус данных: ЗАГРУЖЕНЫ\n\n";
        } else {
            cout << "\nСтатус данных: НЕ ЗАГРУЖЕНЫ\n\n";
        }

        cout << "1) Ввести данные с клавиатуры\n";
        cout << "2) Работа с файлом\n";
        cout << "3) Сформировать случайное министерство\n";
        cout << "4) Показать текущие данные и решение\n";
        cout << "0) Завершить работу\n";
        cout << "Выберите действие (0-4): ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nОшибка: необходимо ввести целое число.\n";
            cout << "Нажмите Enter для продолжения...";
            cin.get();
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        try {
            switch (choice) {
            case 1:
                ministry.inputFromKeyboard();
                break;
            case 2:
                ministry.loadFromFile();
                break;
            case 3:
                ministry.generateRandom();
                break;
            case 4:
                ministry.showDataAndSolution();
                break;
            case 0:
                cout << "\nРабота программы завершена.\n";
                break;
            default:
                cout << "\nОшибка: допустимый диапазон от 0 до 4.\n";
                cout << "Нажмите Enter для продолжения...";
                cin.get();
            }
        } catch (const MinistryException& e) {
            cout << "\nОшибка: " << e.what() << endl;
            cout << "Нажмите Enter для продолжения...";
            cin.get();
        } catch (const exception& e) {
            cout << "\nНепредвиденная ошибка: " << e.what() << endl;
            cout << "Нажмите Enter для продолжения...";
            cin.get();
        }
        
    } while (choice != 0);

    return 0;
}