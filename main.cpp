#include "ministry.h"

#include <iostream>
#include <limits>

int main() {
  std::system("chcp 1251 > nul");

  Ministry ministry;
  int choice;

  std::cout << "\n=== ЧИНОВНИКИ ===\n";
  std::cout << "Минимальная стоимость получения лицензии\n";

  do {
    if (ministry.isDataLoaded()) {
      std::cout << "\nСтатус данных: ЗАГРУЖЕНЫ\n\n";
    } else {
      std::cout << "\nСтатус данных: НЕ ЗАГРУЖЕНЫ\n\n";
    }

    std::cout << "1) Ввести данные с клавиатуры\n";
    std::cout << "2) Работа с файлом\n";
    std::cout << "3) Сформировать случайное министерство\n";
    std::cout << "4) Показать текущие данные и решение\n";
    std::cout << "0) Завершить работу\n";
    std::cout << "Выберите действие (0-4): ";

    if (!(std::cin >> choice)) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "\nОшибка: необходимо ввести целое число.\n";
      std::cout << "Нажмите Enter для продолжения...";
      std::cin.get();
      continue;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
          std::cout << "\nРабота программы завершена.\n";
          break;
        default:
          std::cout << "\nОшибка: допустимый диапазон от 0 до 4.\n";
          std::cout << "Нажмите Enter для продолжения...";
          std::cin.get();
      }
    } catch (const MinistryException& e) {
      std::cout << "\nОшибка: " << e.what() << std::endl;
      std::cout << "Нажмите Enter для продолжения...";
      std::cin.get();
    } catch (const std::exception& e) {
      std::cout << "\nНепредвиденная ошибка: " << e.what() << std::endl;
      std::cout << "Нажмите Enter для продолжения...";
      std::cin.get();
    }

  } while (choice != 0);

  return 0;
}