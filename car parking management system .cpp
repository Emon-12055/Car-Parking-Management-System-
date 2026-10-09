#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cctype> // for tolower()
#include <algorithm> // for transform()

using namespace std;

int ticketCounter = 1000;

// Function to convert string to lowercase
string toLower(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
              [](unsigned char c){ return tolower(c); });
    return lowerStr;
}

// ---------------- VEHICLE CLASSES ----------------
class Vehicle {
protected:
    string ticketID;
    string vehicleNumber;
    string ownerName;
    string vehicleType;
    time_t entryTime;

public:
    Vehicle(string tID, string vNum, string vType, string owner) {
        ticketID = tID;
        vehicleNumber = vNum;
        vehicleType = vType;
        ownerName = owner;
        entryTime = time(0);
    }

    virtual void displayInfo() {
        cout << "Ticket ID: " << ticketID << endl;
        cout << "Vehicle Number: " << vehicleNumber << endl;
        cout << "Owner Name: " << ownerName << endl;
        cout << "Vehicle Type: " << vehicleType << endl;
        cout << "Entry Time: " << ctime(&entryTime);
    }

    virtual double calculateFee() = 0;

    string getTicketID() const { return ticketID; }
    string getVehicleNumber() const { return vehicleNumber; }
    string getVehicleType() const { return vehicleType; }
    time_t getEntryTime() const { return entryTime; }

    virtual ~Vehicle() {}
};

class Bike : public Vehicle {
public:
    Bike(string tID, string vNum, string owner) : Vehicle(tID, vNum, "bike", owner) {}
    double calculateFee() override { return 10.0; }
};

class Car : public Vehicle {
public:
    Car(string tID, string vNum, string owner) : Vehicle(tID, vNum, "car", owner) {}
    double calculateFee() override { return 20.0; }
};

class Truck : public Vehicle {
public:
    Truck(string tID, string vNum, string owner) : Vehicle(tID, vNum, "truck", owner) {}
    double calculateFee() override { return 30.0; }
};

// ---------------- PARKING SLOT CLASS ----------------
class ParkingSlot {
private:
    int slotID;
    bool occupied;
    Vehicle* vehicle;

public:
    ParkingSlot(int id) : slotID(id), occupied(false), vehicle(nullptr) {}

    bool isOccupied() const { return occupied; }
    int getSlotID() const { return slotID; }

    void assignVehicle(Vehicle* v) {
        vehicle = v;
        occupied = true;
    }

    void removeVehicle() {
        delete vehicle;
        vehicle = nullptr;
        occupied = false;
    }

    Vehicle* getVehicle() const { return vehicle; }

    ~ParkingSlot() {
        if (vehicle) delete vehicle;
    }
};

// ---------------- PARKING LOT CLASS ----------------
class ParkingLot {
private:
    vector<ParkingSlot> slots;
    int totalSlots;

public:
    ParkingLot(int n) : totalSlots(n) {
        for (int i = 1; i <= n; ++i) slots.emplace_back(i);
        loadParkedVehicles();
    }

    void parkVehicle() {
        string vNum, vType, owner;
        cout << "\nEnter Vehicle Number: ";
        cin >> vNum;
        cout << "Enter Owner Name: ";
        cin.ignore();
        getline(cin, owner);
        cout << "Enter Vehicle Type (bike/car/truck): ";
        cin >> vType;

        vType = toLower(vType);
        string ticketID = "T" + to_string(ticketCounter++);
        Vehicle* v = nullptr;

        if (vType == "bike") v = new Bike(ticketID, vNum, owner);
        else if (vType == "car") v = new Car(ticketID, vNum, owner);
        else if (vType == "truck") v = new Truck(ticketID, vNum, owner);
        else {
            cout << "Invalid vehicle type.\n";
            return;
        }

        for (auto& slot : slots) {
            if (!slot.isOccupied()) {
                slot.assignVehicle(v);
                cout << "Vehicle Parked. Ticket ID: " << ticketID << " Slot ID: " << slot.getSlotID() << endl;
                saveParkedVehicle(v);
                return;
            }
        }

        cout << "No parking slots available.\n";
        delete v;
    }

    void removeVehicle() {
        string ticket;
        cout << "\nEnter Ticket ID: ";
        cin >> ticket;

        for (auto& slot : slots) {
            if (slot.isOccupied() && slot.getVehicle()->getTicketID() == ticket) {
                Vehicle* v = slot.getVehicle();
                time_t now = time(0);
                double hours = difftime(now, v->getEntryTime()) / 3600;
                if (hours < 1) hours = 1;
                double fee = hours * v->calculateFee();

                cout << "\n--- Vehicle Exit Details ---\n";
                v->displayInfo();
                cout << fixed << setprecision(2);
                cout << "Duration: " << hours << " hours\n";
                cout << "Parking Fee: Tk." << fee << "\n";

                saveToHistory(v, hours, fee);
                slot.removeVehicle();
                return;
            }
        }

        cout << "Ticket not found.\n";
    }

    void showParkedVehicles() {
        cout << "\n--- Currently Parked Vehicles ---\n";
        bool found = false;
        for (auto& slot : slots) {
            if (slot.isOccupied()) {
                slot.getVehicle()->displayInfo();
                cout << endl;
                found = true;
            }
        }
        if (!found) {
            cout << "No vehicles currently parked.\n";
        }
    }

    void searchVehicle() {
        string keyword;
        cout << "\nEnter Vehicle Number or Ticket ID to search: ";
        cin >> keyword;

        for (auto& slot : slots) {
            if (slot.isOccupied()) {
                Vehicle* v = slot.getVehicle();
                if (v->getVehicleNumber() == keyword || v->getTicketID() == keyword) {
                    cout << "Vehicle found at Slot " << slot.getSlotID() << ":\n";
                    v->displayInfo();
                    return;
                }
            }
        }

        cout << "Vehicle not found.\n";
    }

    void saveParkedVehicle(Vehicle* v) {
        ofstream fout("parked_vehicles.txt", ios::app);
        if (fout.is_open()) {
            fout << v->getTicketID() << "," << v->getVehicleNumber() << ","
                 << v->getVehicleType() << "," << v->getEntryTime() << endl;
            fout.close();
        } else {
            cerr << "Error: Unable to save parked vehicle data.\n";
        }
    }

    void saveToHistory(Vehicle* v, double hours, double fee) {
        ofstream fout("parking_history.txt", ios::app);
        if (fout.is_open()) {
            fout << v->getTicketID() << "," << v->getVehicleNumber() << ","
                 << v->getVehicleType() << "," << v->getEntryTime() << ","
                 << hours << "," << fee << endl;
            fout.close();
        } else {
            cerr << "Error: Unable to save parking history.\n";
        }
    }

    void loadParkedVehicles() {
        ifstream fin("parked_vehicles.txt");
        if (!fin.is_open()) {
            return; // No existing file, first run
        }

        string line;
        while (getline(fin, line)) {
            stringstream ss(line);
            string ticketID, vNum, vType, entryStr;
            getline(ss, ticketID, ',');
            getline(ss, vNum, ',');
            getline(ss, vType, ',');
            getline(ss, entryStr);

            try {
                time_t entry = stoi(entryStr);
                Vehicle* v = nullptr;
                if (vType == "bike") v = new Bike(ticketID, vNum, "Unknown");
                else if (vType == "car") v = new Car(ticketID, vNum, "Unknown");
                else if (vType == "truck") v = new Truck(ticketID, vNum, "Unknown");

                if (v) {
                    bool assigned = false;
                    for (auto& slot : slots) {
                        if (!slot.isOccupied()) {
                            slot.assignVehicle(v);
                            assigned = true;
                            break;
                        }
                    }
                    if (!assigned) {
                        delete v;
                    }
                }
            } catch (...) {
                cerr << "Error parsing parked vehicle data: " << line << endl;
            }
        }
        fin.close();
    }

    int totalVehiclesParked() const {
        int count = 0;
        for (auto& slot : slots) {
            if (slot.isOccupied()) count++;
        }
        return count;
    }

    double totalAmountEarned() {
        double total = 0.0;
        ifstream fin("parking_history.txt");
        if (!fin.is_open()) {
            return 0.0; // No history yet
        }

        string line;
        while (getline(fin, line)) {
            stringstream ss(line);
            string token;
            vector<string> tokens;
            while (getline(ss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 6) {
                try {
                    total += stod(tokens[5]);
                } catch (...) {
                    cerr << "Error parsing fee from: " << line << endl;
                }
            }
        }
        fin.close();
        return total;
    }
};

// ---------------- USER AND ADMIN LOGIN SYSTEM ----------------
bool login(bool isAdmin) {
    string username, password;
    if (isAdmin) {
        cout << "\n===== Admin Login =====\n";
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;

        if (username == "admin" && password == "1234") {
            return true;
        }
        cout << "Invalid admin credentials.\n";
        return false;
    } else {
        cout << "\n===== User Login =====\n";
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;

        // Check if user exists
        bool userExists = false;
        ifstream fin("users.txt");
        if (fin.is_open()) {
            string fileUsername, filePassword;
            while (fin >> fileUsername >> filePassword) {
                if (fileUsername == username) {
                    userExists = true;
                    break;
                }
            }
            fin.close();
        }

        if (!userExists) {
            cout << "User not found. Would you like to register? (y/n): ";
            char choice;
            cin >> choice;
            if (tolower(choice) == 'y') {
                ofstream fout("users.txt", ios::app);
                if (fout.is_open()) {
                    fout << username << " " << password << endl;
                    fout.close();
                    cout << "User registered successfully!\n";
                    return true;
                } else {
                    cerr << "Error: Unable to register user.\n";
                    return false;
                }
            } else {
                return false;
            }
        }

        // Verify credentials
        ifstream fin2("users.txt");
        if (fin2.is_open()) {
            string fileUsername, filePassword;
            while (fin2 >> fileUsername >> filePassword) {
                if (fileUsername == username && filePassword == password) {
                    fin2.close();
                    return true;
                }
            }
            fin2.close();
        }
        cout << "Invalid username or password.\n";
        return false;
    }
}

// ---------------- MAIN ----------------
int main() {
    bool isAdmin = false;
    string userChoice;

    cout << "===== Welcome to ParkEase System =====\n";
    cout << "Are you an admin (y/n)? ";
    cin >> userChoice;
    if (tolower(userChoice[0]) == 'y') isAdmin = true;

    if (!login(isAdmin)) {
        cout << "Access Denied.\n";
        return 0;
    }

    ParkingLot lot(10);
    int choice;
    bool exitProgram = false;

    do {
        cout << "\n===== ParkEase Menu =====\n";
        cout << "1. Park Vehicle\n";
        cout << "2. Remove Vehicle\n";
        cout << "3. Show Parked Vehicles\n";
        cout << "4. Search Vehicle\n";
        if (isAdmin) {
            cout << "5. Show Total Vehicles Parked\n";
            cout << "6. Show Total Amount Earned\n";
            cout << "7. Exit\n";
        } else {
            cout << "5. Exit\n";
        }
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: lot.parkVehicle(); break;
            case 2:
                if (isAdmin) {
                    lot.removeVehicle();
                } else {
                    cout << "You don't have permission to remove vehicles.\n";
                }
                break;
            case 3: lot.showParkedVehicles(); break;
            case 4: lot.searchVehicle(); break;
            case 5:
                if (isAdmin) {
                    cout << "\nTotal Vehicles Currently Parked: " << lot.totalVehiclesParked() << endl;
                } else {
                    cout << "Exiting...\n";
                    exitProgram = true;
                }
                break;
            case 6:
                if (isAdmin) {
                    cout << fixed << setprecision(2);
                    cout << "\nTotal Amount Earned: TK." << lot.totalAmountEarned() << endl;
                } else {
                    cout << "Invalid choice.\n";
                }
                break;
            case 7:
                if (isAdmin) {
                    cout << "Exiting...\n";
                    exitProgram = true;
                } else {
                    cout << "Invalid choice.\n";
                }
                break;
            default: cout << "Invalid choice.\n";
        }
    } while (!exitProgram);

    return 0;
}




