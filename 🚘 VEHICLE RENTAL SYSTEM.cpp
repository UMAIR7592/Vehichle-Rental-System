#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

// Color Codes
#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define LIGHTGRAY   "\033[37m"
#define PURPLE      "\033[38;5;129m"

// Structure for Vehicle Model
struct Model {
    string name;
    string emoji;
    string color;
    int rentPerDay;
};

// Structure for Vehicle Type
struct VehicleType {
    string typeName;
    string typeEmoji;
    string color;
    int challan; // specific challan amount
    vector<Model> models;
};

// Vehicle Data
vector<VehicleType> vehicleDB = {
    {"Car", "🚗", RED, 2200, {
        {"Lamborghini", "🦁", YELLOW, 15000},
        {"Ferrari", "🐎", RED, 14000},
        {"Toyota Corolla", "🦊", GREEN, 5000},
        {"Honda Civic", "🐯", BLUE, 5500},
        {"BMW", "🐻", MAGENTA, 12000},
        {"Audi", "🐺", CYAN, 12500},
        {"Mercedes", "🦄", PURPLE, 13000}
    }},
    {"Bike", "🏍️", MAGENTA, 10000, {
        {"Yamaha R1", "🐍", RED, 2000},
        {"Suzuki GS150", "🐸", GREEN, 1200},
        {"Honda CG125", "🐦", BLUE, 1000},
        {"Kawasaki Ninja", "🐲", YELLOW, 2500},
        {"BMW S1000RR", "🦂", MAGENTA, 3000},
        {"Ducati", "🐙", CYAN, 2700},
        {"Harley Davidson", "🦅", PURPLE, 3500}
    }},
    {"SUV", "🚙", BLUE, 10000, {
        {"Range Rover", "🐘", RED, 9000},
        {"Land Cruiser", "🦏", GREEN, 9500},
        {"Fortuner", "🦓", BLUE, 8500},
        {"MG HS", "🦍", YELLOW, 8000},
        {"Hyundai Tucson", "🦌", MAGENTA, 8200},
        {"Kia Sportage", "🦢", CYAN, 8000},
        {"Audi Q7", "🦚", PURPLE, 10000}
    }},
    {"Bus", "🚌", YELLOW, 10000, {
        {"Daewoo", "🦀", RED, 6000},
        {"Foton", "🐡", GREEN, 6200},
        {"Toyota Coaster", "🐬", BLUE, 6100},
        {"Volvo Bus", "🐳", YELLOW, 6300},
        {"Yutong", "🦐", MAGENTA, 6500},
        {"Hyundai Bus", "🐙", CYAN, 6400},
        {"King Long", "🦞", PURPLE, 6600}
    }},
    {"Truck", "🚛", GREEN, 10000, {
        {"Isuzu", "🐪", RED, 7000},
        {"Hino", "🐫", GREEN, 7500},
        {"Mazda", "🦙", BLUE, 6800},
        {"Volvo", "🦧", YELLOW, 7200},
        {"Mitsubishi", "🐘", MAGENTA, 7100},
        {"Scania", "🦒", CYAN, 7300},
        {"MAN", "🦬", PURPLE, 7400}
    }},
    {"Van", "🚐", CYAN, 10000, {
        {"Suzuki Every", "🐰", RED, 3500},
        {"Toyota HiAce", "🐿️", GREEN, 4000},
        {"Hyundai H-1", "🦝", BLUE, 4200},
        {"Ford Transit", "🦔", YELLOW, 4500},
        {"Kia Carnival", "🦘", MAGENTA, 4600},
        {"Nissan Urvan", "🦨", CYAN, 4700},
        {"Chevrolet Van", "🦥", PURPLE, 4800}
    }},
    {"Rickshaw", "🛺", LIGHTGRAY, 10000, {
        {"Loader", "📦", RED, 800},
        {"Passenger", "🧍", GREEN, 700},
        {"Qingqi", "🧑‍🔧", BLUE, 750},
        {"Hybrid Rickshaw", "🧃", YELLOW, 900},
        {"Pink Rickshaw", "🎀", MAGENTA, 950},
        {"Electric Rickshaw", "🔋", CYAN, 1000},
        {"CNG Rickshaw", "🛢️", PURPLE, 850}
    }}
};

// Display all Vehicles
void viewVehicles() {
    cout << "\n🌟 Available Vehicle Types:\n";
    for (size_t i = 0; i < vehicleDB.size(); ++i) {
        cout << i + 1 << ". " << vehicleDB[i].color
             << vehicleDB[i].typeEmoji << " " << vehicleDB[i].typeName << RESET << endl;
    }
}

// Rent a Vehicle
void rentVehicle() {
    viewVehicles();
    int typeChoice;
    cout << "\n🚘 Enter Vehicle Type Number to Rent: ";
    cin >> typeChoice;
    if (typeChoice < 1 || typeChoice > vehicleDB.size()) return;

    VehicleType selected = vehicleDB[typeChoice - 1];
    cout << "\n🔍 Available " << selected.color << selected.typeName << RESET << " Models:\n";

    for (size_t i = 0; i < selected.models.size(); ++i) {
        cout << i + 1 << ". " << selected.models[i].color
             << selected.models[i].emoji << " " << selected.models[i].name
             << " - Rs. " << selected.models[i].rentPerDay << " per day" << RESET << endl;
    }

    cout << "\n🧠 " << YELLOW << "POINT TO PONDER: " << RESET
         << RED << "If you break any part of a " << selected.typeName
         << ", you will be fined Rs. " << selected.challan << "!" << RESET << endl;

    int modelChoice, days;
    cout << "\n🚗 Choose Model Number: ";
    cin >> modelChoice;
    if (modelChoice < 1 || modelChoice > selected.models.size()) return;

    cout << "📆 Enter number of days to rent: ";
    cin >> days;

    int rent = selected.models[modelChoice - 1].rentPerDay * days;

    cout << GREEN << "\n✅ Vehicle rented successfully!" << RESET << endl;
    cout << CYAN << "🧾 Total Rent (without damage): Rs. " << rent << RESET << endl;
}

// Return Vehicle
void returnVehicle() {
    viewVehicles();
    int typeChoice;
    cout << "\n🚘 Enter Vehicle Type Number to Return: ";
    cin >> typeChoice;
    if (typeChoice < 1 || typeChoice > vehicleDB.size()) return;

    VehicleType selected = vehicleDB[typeChoice - 1];
    int modelChoice, days;
    cout << "\n🔍 Enter Model Number Returned: ";
    cin >> modelChoice;
    if (modelChoice < 1 || modelChoice > selected.models.size()) return;

    cout << "📆 Enter Days Used: ";
    cin >> days;

    int baseRent = selected.models[modelChoice - 1].rentPerDay * days;
    int total = baseRent;

    char damage;
    cout << "⚠️ Was the vehicle broken/damaged? (y/n): ";
    cin >> damage;

    if (damage == 'y' || damage == 'Y') {
        total += selected.challan;
        cout << RED << "🚨 Challan of Rs. " << selected.challan << " added due to damage!" << RESET << endl;
    }

    cout << GREEN << "\n✅ Vehicle returned successfully!" << RESET << endl;
    cout << CYAN << "🧾 Final Bill: Rs. " << total << RESET << endl;
}

// Main Menu
int main() {
    cout << "\n🚀 " << YELLOW << "VEHICLE RENTAL SYSTEM" << RESET
         << "\n📌 " << MAGENTA << "Curated by UMAIR IMRAN\n" << RESET;

    while (true) {
        cout << "\n🔸 MAIN MENU\n";
        cout << "1. View Vehicles\n";
        cout << "2. Rent a Vehicle\n";
        cout << "3. Return a Vehicle\n";
        cout << "4. Exit\n";

        int choice;
        cout << "\n➡️  Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: viewVehicles(); break;
            case 2: rentVehicle(); break;
            case 3: returnVehicle(); break;
            case 4: cout << GREEN << "\n👋 Thanks for using our system!\n" << RESET; return 0;
            default: cout << RED << "❌ Invalid choice. Try again.\n" << RESET;
        }
    }
}
