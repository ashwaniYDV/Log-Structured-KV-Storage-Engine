#include <iostream>
#include "KeyValueStore.h"

using namespace std;

int main() {
    KeyValueStore kvStore;

    while(true) {
        cout << "Enter 0 to exit. Enter 1 to get item.  Enter 2 to put item.  Enter 3 to delete item" << std::endl;
        
        int choice;
        cin >> choice;
        if (choice == 0) {
            break;
        } else if (choice == 1) {
            cout << "Enter key: ";
            std::string key;
            cin >> key;
            std::cout << kvStore.get(key).value << std::endl;
        } else if (choice == 2) {
            cout << "Enter key: ";
            std::string key;
            cin >> key;
            cout << "Enter value: ";
            string value;
            cin >> value;
            kvStore.put(key, value);
        } else if (choice == 3) {
            cout << "Enter key: ";
            std::string key;
            cin >> key;
            kvStore.remove(key);
        } else {
            std::cout << "Enter correct choice." << std::endl;
        }
    }

    return 0;
}