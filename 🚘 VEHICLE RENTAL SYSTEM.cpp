#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Model {
    string name;
    int rentPerDay;
};

struct VehicleType {
    string typeName;
    int challan;
    vector<Model> models;
};

class VehicleRentalSystem {
private:
    vector<VehicleType> vehicleDB;

    int getValidInput(int min, int max) const {
        int choice;
        cin >> choice;

        while (cin.fail() || choice < min || choice > max) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Try again: ";
            cin >> choice;
        }
        return choice;
    }

public:
    VehicleRentalSystem() {
        vehicleDB = {
            {"Car", 2200, {{"Lamborghini",15000},{"Ferrari",14000},{"Civic",5500}}},
            {"Bike",10000, {{"Yamaha R1",2000},{"CG125",1000},{"Ducati",2700}}},
            {"SUV",10000, {{"Fortuner",8500},{"Sportage",8000},{"Audi Q7",10000}}}
        };
    }

    void viewVehicles() const {
        cout << "\nAvailable Vehicle Types:\n";
        for (size_t i = 0; i < vehicleDB.size(); ++i) {
            cout << i + 1 << ". " << vehicleDB[i].typeName << endl;
        }
    }

    void rentVehicle() {
        viewVehicles();
        cout << "\nSelect vehicle type: ";
        int typeIdx = getValidInput(1, vehicleDB.size()) - 1;

        const VehicleType& selected = vehicleDB[typeIdx];

        cout << "\nModels:\n";
        for (size_t i = 0; i < selected.models.size(); ++i) {
            cout << i + 1 << ". " << selected.models[i].name
                 << " (Rs." << selected.models[i].rentPerDay << "/day)\n";
        }

        cout << "\nSelect model: ";
        int modelIdx = getValidInput(1, selected.models.size()) - 1;

        cout << "Enter days: ";
        int days = getValidInput(1, 365);

        int total = selected.models[modelIdx].rentPerDay * days;

        cout << "\nTotal Rent: Rs." << total << endl;
    }

    void returnVehicle() {
        viewVehicles();
        cout << "\nSelect vehicle type: ";
        int typeIdx = getValidInput(1, vehicleDB.size()) - 1;

        const VehicleType& selected = vehicleDB[typeIdx];

        cout << "Select model: ";
        int modelIdx = getValidInput(1, selected.models.size()) - 1;

        cout << "Enter days used: ";
        int days = getValidInput(1, 365);

        int total = selected.models[modelIdx].rentPerDay * days;

        char damaged;
        cout << "Damaged? (y/n): ";
        cin >> damaged;

        if (damaged == 'y' || damaged == 'Y') {
            total += selected.challan;
        }

        cout << "Final Bill: Rs." << total << endl;
    }

    void run() {
        while (true) {
            cout << "\n1. View\n2. Rent\n3. Return\n4. Exit\nChoice: ";
            int choice = getValidInput(1, 4);

            switch (choice) {
                case 1: viewVehicles(); break;
                case 2: rentVehicle(); break;
                case 3: returnVehicle(); break;
                case 4: cout << "Goodbye!\n"; return;
            }
        }
    }
};

int main() {
    VehicleRentalSystem system;
    system.run();
}
