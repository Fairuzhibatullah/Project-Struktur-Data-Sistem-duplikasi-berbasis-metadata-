#include "include/VectorEngine.h"
#include "include/ArsipManager.h"

int main() {
    VectorEngine engine;
    ArsipManager manager(&engine, "Vector", "vector_database.csv");
    manager.runMenu();
    return 0;
}