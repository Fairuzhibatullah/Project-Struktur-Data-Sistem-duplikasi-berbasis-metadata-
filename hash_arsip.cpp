#include "include/HashEngine.h"
#include "include/ArsipManager.h"

int main() {
    HashEngine engine;
    ArsipManager manager(&engine, "Hash Table", "hash_database.csv");
    manager.runMenu();
    return 0;
}
