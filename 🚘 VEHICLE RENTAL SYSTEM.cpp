/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║          VELOCE — Premium Vehicle Rental Management System       ║
 * ║                   Enterprise Edition  v2.0.0                     ║
 * ╚══════════════════════════════════════════════════════════════════╝
 *
 * @file        VehicleRentalSystem.cpp
 * @author      VRS Engineering Team
 * @version     2.0.0
 * @standard    C++17
 *
 * @description
 *   Full-featured, production-grade vehicle rental management platform
 *   with booking lifecycle management, dynamic billing engine, customer
 *   registry, and a rich terminal UI with ANSI rendering.
 *
 * @compile
 *   g++ -std=c++17 -O2 -Wall -Wextra -o veloce VehicleRentalSystem.cpp
 */

// ─── Standard Library Imports ────────────────────────────────────────────────
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <random>
#include <limits>
#include <regex>

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 1 ── ANSI Terminal Renderer
// ═════════════════════════════════════════════════════════════════════════════

namespace ANSI {
    // Text Styles
    constexpr const char* RESET       = "\033[0m";
    constexpr const char* BOLD        = "\033[1m";
    constexpr const char* DIM         = "\033[2m";
    constexpr const char* ITALIC      = "\033[3m";
    constexpr const char* UNDERLINE   = "\033[4m";

    // Foreground Colors
    constexpr const char* BLACK       = "\033[30m";
    constexpr const char* RED         = "\033[31m";
    constexpr const char* GREEN       = "\033[32m";
    constexpr const char* YELLOW      = "\033[33m";
    constexpr const char* BLUE        = "\033[34m";
    constexpr const char* MAGENTA     = "\033[35m";
    constexpr const char* CYAN        = "\033[36m";
    constexpr const char* WHITE       = "\033[37m";

    // Bright Foreground
    constexpr const char* BRIGHT_RED     = "\033[91m";
    constexpr const char* BRIGHT_GREEN   = "\033[92m";
    constexpr const char* BRIGHT_YELLOW  = "\033[93m";
    constexpr const char* BRIGHT_BLUE    = "\033[94m";
    constexpr const char* BRIGHT_MAGENTA = "\033[95m";
    constexpr const char* BRIGHT_CYAN    = "\033[96m";
    constexpr const char* BRIGHT_WHITE   = "\033[97m";

    // Background Colors
    constexpr const char* BG_BLACK    = "\033[40m";
    constexpr const char* BG_BLUE     = "\033[44m";
    constexpr const char* BG_CYAN     = "\033[46m";

    inline void clearScreen() { std::cout << "\033[2J\033[H"; }
    inline void moveCursor(int row, int col) {
        std::cout << "\033[" << row << ";" << col << "H";
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 2 ── Custom Exception Hierarchy
// ═════════════════════════════════════════════════════════════════════════════

namespace VRS {

class VRSException : public std::runtime_error {
public:
    explicit VRSException(std::string_view msg)
        : std::runtime_error(std::string(msg)) {}
};

class InvalidInputException  : public VRSException {
public:
    explicit InvalidInputException(std::string_view msg)
        : VRSException(msg) {}
};

class BookingNotFoundException : public VRSException {
public:
    explicit BookingNotFoundException(std::string_view id)
        : VRSException("Booking ID [" + std::string(id) + "] not found in registry.") {}
};

class VehicleUnavailableException : public VRSException {
public:
    explicit VehicleUnavailableException(std::string_view model)
        : VRSException("Vehicle [" + std::string(model) + "] is currently unavailable.") {}
};

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 3 ── Domain Models
// ═════════════════════════════════════════════════════════════════════════════

/**
 * @brief Booking lifecycle state machine
 */
enum class BookingStatus {
    PENDING,    ///< Created, awaiting confirmation
    ACTIVE,     ///< Vehicle handed over to customer
    RETURNED,   ///< Vehicle returned, billing complete
    CANCELLED   ///< Booking voided
};

/**
 * @brief Insurance tier options
 */
enum class InsuranceTier {
    NONE,       ///< No coverage  — Rs. 0/day
    BASIC,      ///< Liability    — Rs. 200/day
    PREMIUM     ///< Full cover   — Rs. 500/day
};

/**
 * @brief Immutable vehicle model descriptor
 */
struct VehicleModel {
    std::string name;
    int         rentPerDay;     ///< Base daily rate (Rs.)
    int         deposit;        ///< Refundable security deposit (Rs.)
    std::string category;       ///< e.g. "Luxury", "Economy", "Sport"
    int         maxPassengers;

    VehicleModel(std::string n, int rate, int dep, std::string cat, int pax)
        : name(std::move(n)), rentPerDay(rate), deposit(dep),
          category(std::move(cat)), maxPassengers(pax) {}
};

/**
 * @brief Vehicle type catalog entry
 */
struct VehicleType {
    std::string              typeName;
    std::string              icon;
    int                      damageCharge;  ///< Flat damage challan (Rs.)
    int                      lateFeePerDay; ///< Extra charge per overdue day
    std::vector<VehicleModel> models;
};

/**
 * @brief Registered customer profile
 */
struct Customer {
    std::string id;
    std::string fullName;
    std::string phone;
    std::string cnic;       ///< National ID
    int         totalRentals = 0;

    std::string displayName() const {
        return fullName + " (" + phone + ")";
    }
};

/**
 * @brief Core booking record — complete rental transaction
 */
struct Booking {
    std::string     bookingId;
    std::string     customerId;
    std::string     vehicleType;
    std::string     vehicleModel;
    int             rentPerDay;
    int             depositPaid;
    int             plannedDays;
    int             actualDays       = 0;
    InsuranceTier   insurance        = InsuranceTier::NONE;
    BookingStatus   status           = BookingStatus::PENDING;
    bool            isDamaged        = false;
    int             damageCharge     = 0;
    std::string     bookingTimestamp;
    std::string     returnTimestamp;

    // ── Derived computations ──────────────────────────────────────────────

    int baseRent() const {
        int days = (actualDays > 0 ? actualDays : plannedDays);
        return rentPerDay * days;
    }

    int insuranceCost() const {
        int days = (actualDays > 0 ? actualDays : plannedDays);
        switch (insurance) {
            case InsuranceTier::BASIC:   return 200 * days;
            case InsuranceTier::PREMIUM: return 500 * days;
            default:                     return 0;
        }
    }

    int lateFee(int lateFeeRate) const {
        if (actualDays <= plannedDays) return 0;
        return (actualDays - plannedDays) * lateFeeRate;
    }

    int totalBill(int lateFeeRate) const {
        return baseRent() + insuranceCost() + damageCharge + lateFee(lateFeeRate);
    }

    std::string statusLabel() const {
        switch (status) {
            case BookingStatus::PENDING:   return "PENDING";
            case BookingStatus::ACTIVE:    return "ACTIVE";
            case BookingStatus::RETURNED:  return "RETURNED";
            case BookingStatus::CANCELLED: return "CANCELLED";
            default:                       return "UNKNOWN";
        }
    }

    std::string insuranceLabel() const {
        switch (insurance) {
            case InsuranceTier::BASIC:   return "Basic  (Rs.200/day)";
            case InsuranceTier::PREMIUM: return "Premium(Rs.500/day)";
            default:                     return "None";
        }
    }
};

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 4 ── Utility & Helper Layer
// ═════════════════════════════════════════════════════════════════════════════

namespace Utils {

/**
 * @brief ISO-8601 timestamp from system clock
 */
std::string timestamp() {
    auto now    = std::chrono::system_clock::now();
    auto t      = std::chrono::system_clock::to_time_t(now);
    std::tm tm  = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/**
 * @brief Generate a unique booking ID in the format VRS-XXXXX
 */
std::string generateBookingId() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(10000, 99999);
    return "VRS-" + std::to_string(dist(rng));
}

/**
 * @brief Generate a customer ID
 */
std::string generateCustomerId() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(1000, 9999);
    return "CUS-" + std::to_string(dist(rng));
}

/**
 * @brief Format an integer as a rupee string with commas
 */
std::string formatRupees(int amount) {
    std::string s = std::to_string(amount);
    int n = static_cast<int>(s.length()) - 3;
    while (n > 0) { s.insert(n, ","); n -= 3; }
    return "Rs. " + s;
}

/**
 * @brief Repeat a string n times (supports multi-byte UTF-8 chars)
 */
std::string repeat(const std::string& s, int n) {
    std::string result;
    result.reserve(s.size() * static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) result += s;
    return result;
}

/**
 * @brief Repeat a single-byte char n times (ASCII only)
 */
std::string repeat(char c, int n) {
    return std::string(static_cast<size_t>(n), c);
}

/**
 * @brief Center-pad a string within a field of given width
 */
std::string center(const std::string& s, int width, char fill = ' ') {
    int pad = width - static_cast<int>(s.length());
    if (pad <= 0) return s;
    int left  = pad / 2;
    int right = pad - left;
    return std::string(left, fill) + s + std::string(right, fill);
}

/**
 * @brief Flush cin and get a validated integer in [min, max]
 */
int getValidInt(int min, int max, const std::string& prompt = "") {
    int value;
    while (true) {
        if (!prompt.empty()) std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << ANSI::BRIGHT_RED << "  ✗ Invalid — enter a number between "
                  << min << " and " << max << ": " << ANSI::RESET;
    }
}

/**
 * @brief Read a non-empty trimmed line from stdin
 */
std::string getLine(const std::string& prompt = "") {
    std::string line;
    while (true) {
        if (!prompt.empty()) std::cout << prompt;
        std::getline(std::cin, line);
        // trim
        size_t start = line.find_first_not_of(" \t\r\n");
        size_t end   = line.find_last_not_of(" \t\r\n");
        if (start != std::string::npos) return line.substr(start, end - start + 1);
        std::cout << ANSI::BRIGHT_RED << "  ✗ Field cannot be empty. Try again: "
                  << ANSI::RESET;
    }
}

/**
 * @brief Prompt for yes/no — returns true for 'y'/'Y'
 */
bool getYesNo(const std::string& prompt) {
    while (true) {
        std::cout << prompt << " [y/n]: ";
        char c; std::cin >> c;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (c == 'y' || c == 'Y') return true;
        if (c == 'n' || c == 'N') return false;
        std::cout << ANSI::BRIGHT_RED << "  ✗ Please enter y or n.\n" << ANSI::RESET;
    }
}

} // namespace Utils

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 5 ── Terminal UI Renderer
// ═════════════════════════════════════════════════════════════════════════════

namespace UI {

constexpr int WIDTH = 68;

void horizontalRule(const std::string& c = "─") {
    std::cout << ANSI::DIM << Utils::repeat(c, WIDTH/3) << ANSI::RESET << "\n";
}

void boxTop()    { std::cout << ANSI::CYAN << "╔" << Utils::repeat("═", WIDTH-2) << "╗\n" << ANSI::RESET; }
void boxBottom() { std::cout << ANSI::CYAN << "╚" << Utils::repeat("═", WIDTH-2) << "╝\n" << ANSI::RESET; }
void boxMid()    { std::cout << ANSI::CYAN << "╠" << Utils::repeat("═", WIDTH-2) << "╣\n" << ANSI::RESET; }

void boxLine(const std::string& text, const char* color = ANSI::RESET) {
    int inner = WIDTH - 4;
    std::string padded = text.substr(0, static_cast<size_t>(inner));
    padded += std::string(static_cast<size_t>(std::max(0, inner - static_cast<int>(text.length()))), ' ');
    std::cout << ANSI::CYAN << "║ " << ANSI::RESET
              << color << padded << ANSI::RESET
              << ANSI::CYAN << " ║\n" << ANSI::RESET;
}

void boxLineCenter(const std::string& text, const char* color = ANSI::BRIGHT_WHITE) {
    int inner = WIDTH - 4;
    std::string padded = Utils::center(text, inner);
    std::cout << ANSI::CYAN << "║ " << ANSI::RESET
              << color << padded << ANSI::RESET
              << ANSI::CYAN << " ║\n" << ANSI::RESET;
}

void blankBoxLine() {
    std::cout << ANSI::CYAN << "║"
              << std::string(static_cast<size_t>(WIDTH-2), ' ')
              << "║\n" << ANSI::RESET;
}

void printBanner() {
    ANSI::clearScreen();
    boxTop();
    blankBoxLine();
    boxLineCenter("V  E  L  O  C  E", ANSI::BRIGHT_CYAN);
    boxLineCenter("Premium Vehicle Rental Management System", ANSI::BRIGHT_WHITE);
    boxLineCenter("Enterprise Edition  ·  v2.0.0", ANSI::DIM);
    blankBoxLine();
    boxMid();
    boxLine("  " + Utils::timestamp(), ANSI::DIM);
    boxBottom();
    std::cout << "\n";
}

void sectionHeader(const std::string& title) {
    std::cout << "\n" << ANSI::BOLD << ANSI::BRIGHT_CYAN;
    std::cout << "  ┌─ " << title << " ";
    int fill = WIDTH - 6 - static_cast<int>(title.length());
    if (fill > 0) std::cout << Utils::repeat("─", fill/3);
    std::cout << "┐\n" << ANSI::RESET;
}

void sectionFooter() {
    std::cout << ANSI::BOLD << ANSI::BRIGHT_CYAN;
    std::cout << "  └" << Utils::repeat("─", (WIDTH - 3)/3) << "┘\n";
    std::cout << ANSI::RESET;
}

void success(const std::string& msg) {
    std::cout << "\n  " << ANSI::BRIGHT_GREEN << "✔  " << msg << ANSI::RESET << "\n";
}

void error(const std::string& msg) {
    std::cout << "\n  " << ANSI::BRIGHT_RED << "✘  " << msg << ANSI::RESET << "\n";
}

void info(const std::string& msg) {
    std::cout << "  " << ANSI::BRIGHT_YELLOW << "ℹ  " << msg << ANSI::RESET << "\n";
}

void keyValue(const std::string& key, const std::string& value,
              const char* valColor = ANSI::BRIGHT_WHITE) {
    std::cout << "  " << ANSI::DIM << std::left << std::setw(26) << key
              << ANSI::RESET << ANSI::BOLD << valColor << value
              << ANSI::RESET << "\n";
}

void tableHeader(const std::string& col1, int w1,
                 const std::string& col2, int w2,
                 const std::string& col3 = "", int w3 = 0) {
    std::cout << "  " << ANSI::BG_CYAN << ANSI::BLACK << ANSI::BOLD
              << " " << std::left << std::setw(w1) << col1
              << " " << std::setw(w2) << col2;
    if (!col3.empty())
        std::cout << " " << std::setw(w3) << col3;
    std::cout << " " << ANSI::RESET << "\n";
    std::cout << "  " << ANSI::DIM
              << Utils::repeat("─", (w1 + w2 + w3 + (col3.empty() ? 3 : 4))/3)
              << ANSI::RESET << "\n";
}

void prompt(const std::string& label) {
    std::cout << "\n  " << ANSI::BOLD << ANSI::BRIGHT_CYAN
              << "▶  " << label << ": " << ANSI::RESET;
}

void menuItem(int idx, const std::string& label, const char* icon = "  ") {
    std::cout << "  " << ANSI::DIM << "[" << ANSI::RESET
              << ANSI::BOLD << ANSI::BRIGHT_YELLOW << idx << ANSI::RESET
              << ANSI::DIM << "]" << ANSI::RESET
              << " " << icon << " " << label << "\n";
}

void statusBadge(const std::string& status) {
    if      (status == "ACTIVE")    std::cout << ANSI::BG_CYAN << ANSI::BLACK;
    else if (status == "RETURNED")  std::cout << ANSI::BRIGHT_GREEN;
    else if (status == "CANCELLED") std::cout << ANSI::BRIGHT_RED;
    else                            std::cout << ANSI::BRIGHT_YELLOW;
    std::cout << " " << status << " " << ANSI::RESET;
}

void pressEnter() {
    std::cout << "\n  " << ANSI::DIM << "Press ENTER to continue..." << ANSI::RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace UI

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 6 ── Vehicle Catalog
// ═════════════════════════════════════════════════════════════════════════════

class VehicleCatalog {
public:
    const std::vector<VehicleType>& types() const { return catalog_; }

    const VehicleType& typeAt(size_t idx) const {
        if (idx >= catalog_.size())
            throw VRS::InvalidInputException("Vehicle type index out of range.");
        return catalog_[idx];
    }

    const VehicleModel& modelAt(size_t typeIdx, size_t modelIdx) const {
        const auto& vt = typeAt(typeIdx);
        if (modelIdx >= vt.models.size())
            throw VRS::InvalidInputException("Model index out of range.");
        return vt.models[modelIdx];
    }

    static VehicleCatalog build() {
        VehicleCatalog c;
        c.catalog_ = {
            {
                "Car", "🚗", 8000, 1500,
                {
                    {"Lamborghini Huracán",  15000, 50000, "Hypercar",  2},
                    {"Ferrari 488 GTB",      14000, 45000, "Supercar",  2},
                    {"Honda Civic 2024",      5500,  8000, "Sedan",     5},
                    {"Toyota Corolla",        4500,  6000, "Economy",   5},
                    {"BMW 5 Series",         11000, 20000, "Executive", 5},
                }
            },
            {
                "Bike", "🏍", 5000, 800,
                {
                    {"Yamaha R1",   2000, 5000, "Sport",       2},
                    {"Honda CG125", 1000, 2000, "Commuter",    2},
                    {"Ducati Panigale V4", 2700, 8000, "Track",2},
                    {"Harley-Davidson Iron 883", 3500, 12000, "Cruiser", 2},
                }
            },
            {
                "SUV", "🚙", 12000, 2000,
                {
                    {"Toyota Fortuner",  8500, 15000, "Lifestyle", 7},
                    {"Kia Sportage",     8000, 12000, "Compact",   5},
                    {"Audi Q7",         10000, 25000, "Luxury",    7},
                    {"Land Cruiser V8", 13500, 35000, "Premium",   8},
                }
            },
            {
                "Van", "🚐", 10000, 2500,
                {
                    {"Toyota HiAce",     6000, 10000, "Passenger", 12},
                    {"Ford Transit",     7000, 12000, "Cargo",     3 },
                    {"Mercedes Vito",    9000, 18000, "Premium",   8 },
                }
            }
        };
        return c;
    }

private:
    std::vector<VehicleType> catalog_;
};

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 7 ── Customer Registry
// ═════════════════════════════════════════════════════════════════════════════

class CustomerRegistry {
public:
    Customer& registerCustomer() {
        Customer c;
        c.id = Utils::generateCustomerId();

        UI::sectionHeader("NEW CUSTOMER REGISTRATION");
        std::cout << "\n";
        UI::info("Please provide the following details.\n");

        std::cout << "  "; c.fullName = Utils::getLine("Full Name         : ");
        std::cout << "  "; c.phone    = Utils::getLine("Phone Number      : ");
        std::cout << "  "; c.cnic     = Utils::getLine("CNIC / Passport   : ");

        customers_[c.id] = std::move(c);
        UI::success("Customer registered. ID: " + customers_[c.id].id);
        UI::sectionFooter();
        return customers_[c.id];
    }

    std::optional<std::reference_wrapper<Customer>> find(const std::string& id) {
        auto it = customers_.find(id);
        if (it != customers_.end()) return std::ref(it->second);
        return std::nullopt;
    }

    const std::unordered_map<std::string, Customer>& all() const {
        return customers_;
    }

    size_t count() const { return customers_.size(); }

private:
    std::unordered_map<std::string, Customer> customers_;
};

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 8 ── Booking Engine
// ═════════════════════════════════════════════════════════════════════════════

class BookingEngine {
public:
    explicit BookingEngine(const VehicleCatalog& catalog)
        : catalog_(catalog) {}

    /**
     * @brief Create a new ACTIVE booking
     */
    Booking& createBooking(const Customer& customer) {
        UI::sectionHeader("NEW RENTAL BOOKING");
        std::cout << "\n";
        UI::keyValue("Customer", customer.fullName, ANSI::BRIGHT_CYAN);
        UI::keyValue("Customer ID", customer.id, ANSI::DIM);
        std::cout << "\n";

        // ── Vehicle Type Selection ────────────────────────────────────────
        std::cout << "  " << ANSI::BOLD << "Select Vehicle Type\n" << ANSI::RESET;
        UI::tableHeader("No.", 4, "Type", 12, "Damage Charge", 15);
        const auto& types = catalog_.types();
        for (size_t i = 0; i < types.size(); ++i) {
            std::cout << "  " << ANSI::BRIGHT_YELLOW << " [" << (i+1) << "] "
                      << ANSI::RESET
                      << std::left << std::setw(14) << (types[i].icon + "  " + types[i].typeName)
                      << ANSI::BRIGHT_RED << Utils::formatRupees(types[i].damageCharge)
                      << ANSI::RESET << "\n";
        }

        UI::prompt("Type No");
        int typeIdx = Utils::getValidInt(1, static_cast<int>(types.size())) - 1;
        const VehicleType& vt = catalog_.typeAt(typeIdx);

        // ── Model Selection ───────────────────────────────────────────────
        std::cout << "\n  " << ANSI::BOLD << "Available Models — " << vt.typeName << "\n" << ANSI::RESET;
        UI::tableHeader("No.", 4, "Model", 30, "Rate / Day", 14);
        for (size_t i = 0; i < vt.models.size(); ++i) {
            const auto& m = vt.models[i];
            std::cout << "  " << ANSI::BRIGHT_YELLOW << " [" << (i+1) << "] " << ANSI::RESET
                      << std::left << std::setw(32) << (m.name + "  " + ANSI::DIM + "[" + m.category + "]" + ANSI::RESET)
                      << ANSI::BRIGHT_GREEN << Utils::formatRupees(m.rentPerDay) << ANSI::RESET
                      << "\n";
        }

        UI::prompt("Model No");
        int modelIdx = Utils::getValidInt(1, static_cast<int>(vt.models.size())) - 1;
        const VehicleModel& vm = catalog_.modelAt(typeIdx, modelIdx);

        // ── Rental Duration ───────────────────────────────────────────────
        std::cout << "\n";
        UI::prompt("Rental Duration (days, 1–365)");
        int days = Utils::getValidInt(1, 365);

        // ── Insurance ─────────────────────────────────────────────────────
        std::cout << "\n  " << ANSI::BOLD << "Insurance Coverage\n" << ANSI::RESET;
        UI::menuItem(1, "No Insurance        — Rs. 0/day",    "🔓");
        UI::menuItem(2, "Basic Liability     — Rs. 200/day",  "🛡 ");
        UI::menuItem(3, "Full Premium Cover  — Rs. 500/day",  "⭐");

        UI::prompt("Insurance Option");
        int insChoice = Utils::getValidInt(1, 3);
        InsuranceTier ins = (insChoice == 2) ? InsuranceTier::BASIC
                          : (insChoice == 3) ? InsuranceTier::PREMIUM
                                             : InsuranceTier::NONE;

        // ── Build Booking Record ──────────────────────────────────────────
        Booking b;
        b.bookingId        = Utils::generateBookingId();
        b.customerId       = customer.id;
        b.vehicleType      = vt.typeName;
        b.vehicleModel     = vm.name;
        b.rentPerDay       = vm.rentPerDay;
        b.depositPaid      = vm.deposit;
        b.plannedDays      = days;
        b.insurance        = ins;
        b.status           = BookingStatus::ACTIVE;
        b.bookingTimestamp = Utils::timestamp();

        // ── Cost Summary ──────────────────────────────────────────────────
        std::cout << "\n";
        UI::horizontalRule();
        std::cout << "  " << ANSI::BOLD << ANSI::BRIGHT_WHITE
                  << "  BOOKING SUMMARY\n" << ANSI::RESET;
        UI::horizontalRule();
        UI::keyValue("  Booking ID",    b.bookingId,                   ANSI::BRIGHT_CYAN);
        UI::keyValue("  Vehicle",       b.vehicleType + " › " + b.vehicleModel);
        UI::keyValue("  Category",      vt.models[modelIdx].category,  ANSI::DIM);
        UI::keyValue("  Duration",      std::to_string(days) + " day(s)");
        UI::keyValue("  Daily Rate",    Utils::formatRupees(b.rentPerDay), ANSI::BRIGHT_GREEN);
        UI::keyValue("  Insurance",     b.insuranceLabel());
        UI::keyValue("  Security Dep.", Utils::formatRupees(b.depositPaid), ANSI::BRIGHT_YELLOW);
        UI::horizontalRule("─");
        int estimated = b.baseRent() + b.insuranceCost();
        UI::keyValue("  ESTIMATED TOTAL", Utils::formatRupees(estimated), ANSI::BRIGHT_GREEN);
        std::cout << "  " << ANSI::DIM << "  (deposit refundable on return)\n" << ANSI::RESET;
        UI::horizontalRule();

        if (!Utils::getYesNo("\n  Confirm booking?")) {
            b.status = BookingStatus::CANCELLED;
            UI::error("Booking cancelled.");
            bookings_[b.bookingId] = std::move(b);
            UI::sectionFooter();
            return bookings_[b.bookingId];
        }

        bookings_[b.bookingId] = std::move(b);
        UI::success("Booking confirmed!  ID: " + bookings_[b.bookingId].bookingId);
        UI::sectionFooter();
        return bookings_[b.bookingId];
    }

    /**
     * @brief Process a vehicle return and generate final invoice
     */
    void processReturn(CustomerRegistry& registry) {
        UI::sectionHeader("VEHICLE RETURN PROCESSING");
        std::cout << "\n";

        UI::prompt("Enter Booking ID (e.g. VRS-12345)");
        std::string bid = Utils::getLine();

        auto it = bookings_.find(bid);
        if (it == bookings_.end())
            throw VRS::BookingNotFoundException(bid);

        Booking& b = it->second;

        if (b.status != BookingStatus::ACTIVE) {
            UI::error("Booking [" + bid + "] is not in ACTIVE state (status: " + b.statusLabel() + ").");
            return;
        }

        // Retrieve vehicle type for late fee rate
        const VehicleType* vtPtr = nullptr;
        for (const auto& vt : catalog_.types())
            if (vt.typeName == b.vehicleType) { vtPtr = &vt; break; }

        int lateFeeRate   = vtPtr ? vtPtr->lateFeePerDay : 1500;
        int damageCharge  = vtPtr ? vtPtr->damageCharge  : 8000;

        // ── Return details ────────────────────────────────────────────────
        std::cout << "\n";
        UI::keyValue("Booking ID",  b.bookingId, ANSI::BRIGHT_CYAN);
        UI::keyValue("Vehicle",     b.vehicleType + " › " + b.vehicleModel);
        auto custOpt = registry.find(b.customerId);
        std::string custName = custOpt ? custOpt->get().fullName : "Unknown";
        UI::keyValue("Customer",    custName);
        UI::keyValue("Planned Days", std::to_string(b.plannedDays));

        std::cout << "\n";
        UI::prompt("Actual Days Used");
        b.actualDays = Utils::getValidInt(1, 730);

        b.isDamaged = Utils::getYesNo("\n  Vehicle damaged?");
        if (b.isDamaged) b.damageCharge = damageCharge;

        b.status           = BookingStatus::RETURNED;
        b.returnTimestamp  = Utils::timestamp();

        // Update customer rental count
        if (auto cust = registry.find(b.customerId))
            cust->get().totalRentals++;

        // ── Final Invoice ─────────────────────────────────────────────────
        printInvoice(b, lateFeeRate);
        UI::sectionFooter();
    }

    /**
     * @brief Display all bookings or filter by customer ID
     */
    void listBookings(const std::string& filterCustomerId = "") const {
        UI::sectionHeader("BOOKING REGISTRY");
        std::cout << "\n";

        std::vector<const Booking*> view;
        for (const auto& [id, b] : bookings_) {
            if (filterCustomerId.empty() || b.customerId == filterCustomerId)
                view.push_back(&b);
        }

        if (view.empty()) {
            UI::info("No bookings found.");
            UI::sectionFooter();
            return;
        }

        UI::tableHeader("Booking ID", 12, "Vehicle", 28, "Status", 10);
        for (const auto* b : view) {
            std::cout << "  " << ANSI::BRIGHT_CYAN << std::left << std::setw(13)
                      << b->bookingId << ANSI::RESET
                      << std::setw(30)
                      << (b->vehicleType + " › " + b->vehicleModel).substr(0, 28)
                      << " ";
            UI::statusBadge(b->statusLabel());
            std::cout << "\n";
        }
        std::cout << "\n  " << ANSI::DIM << "Total: " << view.size()
                  << " record(s).\n" << ANSI::RESET;
        UI::sectionFooter();
    }

    /**
     * @brief Lookup and display a single booking's full details
     */
    void lookupBooking() const {
        UI::sectionHeader("BOOKING DETAIL LOOKUP");
        UI::prompt("Booking ID");
        std::string bid = Utils::getLine();

        auto it = bookings_.find(bid);
        if (it == bookings_.end())
            throw VRS::BookingNotFoundException(bid);

        const Booking& b = it->second;
        std::cout << "\n";
        UI::horizontalRule();
        UI::keyValue("  Booking ID",      b.bookingId,        ANSI::BRIGHT_CYAN);
        UI::keyValue("  Customer ID",     b.customerId,       ANSI::DIM);
        UI::keyValue("  Vehicle",         b.vehicleType + " › " + b.vehicleModel);
        UI::keyValue("  Insurance",       b.insuranceLabel());
        UI::keyValue("  Planned Days",    std::to_string(b.plannedDays));
        UI::keyValue("  Actual Days",     b.actualDays > 0 ? std::to_string(b.actualDays) : "—");
        UI::keyValue("  Booked On",       b.bookingTimestamp, ANSI::DIM);
        UI::keyValue("  Returned On",     b.returnTimestamp.empty() ? "—" : b.returnTimestamp, ANSI::DIM);
        UI::keyValue("  Security Dep.",   Utils::formatRupees(b.depositPaid), ANSI::BRIGHT_YELLOW);
        UI::keyValue("  Status",          b.statusLabel());
        UI::horizontalRule();
        UI::sectionFooter();
    }

    size_t bookingCount() const { return bookings_.size(); }

    int totalRevenue() const {
        int rev = 0;
        for (const auto& [id, b] : bookings_)
            if (b.status == BookingStatus::RETURNED)
                rev += b.baseRent() + b.insuranceCost() + b.damageCharge;
        return rev;
    }

private:
    /**
     * @brief Render a professional final invoice to stdout
     */
    void printInvoice(const Booking& b, int lateFeeRate) const {
        std::cout << "\n";
        UI::boxTop();
        UI::boxLineCenter("VELOCE RENTAL SERVICES — FINAL INVOICE", ANSI::BOLD);
        UI::boxLine("  " + Utils::repeat("─", (UI::WIDTH - 6)/3));
        UI::boxLine("  Booking ID   : " + b.bookingId, ANSI::BRIGHT_CYAN);
        UI::boxLine("  Vehicle      : " + b.vehicleType + " › " + b.vehicleModel);
        UI::boxLine("  Insurance    : " + b.insuranceLabel());
        UI::boxLine("  Planned Days : " + std::to_string(b.plannedDays));
        UI::boxLine("  Actual Days  : " + std::to_string(b.actualDays));
        UI::boxLine("  Booked On    : " + b.bookingTimestamp, ANSI::DIM);
        UI::boxLine("  Returned On  : " + b.returnTimestamp,  ANSI::DIM);
        UI::boxMid();

        auto line = [&](const std::string& label, int amount, const char* col = ANSI::BRIGHT_WHITE) {
            std::string val = Utils::formatRupees(amount);
            std::string row = "  " + std::string(28, ' ');
            row = "  " + label;
            int pad = 40 - static_cast<int>(label.length());
            row += std::string(std::max(1, pad), '.');
            row += val;
            UI::boxLine(row, col);
        };

        line("Base Rent (" + std::to_string(b.actualDays) + " days × "
             + Utils::formatRupees(b.rentPerDay) + ")",
             b.baseRent(), ANSI::BRIGHT_WHITE);

        if (b.insuranceCost() > 0)
            line("Insurance (" + b.insuranceLabel() + ")", b.insuranceCost(), ANSI::YELLOW);

        if (b.isDamaged)
            line("Damage Charge (challan)", b.damageCharge, ANSI::BRIGHT_RED);

        int lf = b.lateFee(lateFeeRate);
        if (lf > 0)
            line("Late Return Fee (" + std::to_string(b.actualDays - b.plannedDays)
                 + " day(s) overdue)", lf, ANSI::BRIGHT_RED);

        UI::boxMid();
        line("GROSS TOTAL", b.totalBill(lateFeeRate), ANSI::BRIGHT_GREEN);
        line("Security Deposit (refund)", -b.depositPaid, ANSI::BRIGHT_YELLOW);

        int net = b.totalBill(lateFeeRate) - b.depositPaid;
        UI::boxMid();
        line("NET AMOUNT DUE", std::max(0, net), ANSI::BRIGHT_GREEN);

        if (net < 0) {
            UI::blankBoxLine();
            UI::boxLine("  ⚠  Refund of " + Utils::formatRupees(-net) + " to be issued.", ANSI::BRIGHT_YELLOW);
        }

        UI::blankBoxLine();
        UI::boxLineCenter("Thank you for choosing Veloce.", ANSI::DIM);
        UI::boxLineCenter("Drive responsibly. ❤", ANSI::DIM);
        UI::boxBottom();
    }

    const VehicleCatalog&                    catalog_;
    std::unordered_map<std::string, Booking> bookings_;
};

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 9 ── Analytics Dashboard
// ═════════════════════════════════════════════════════════════════════════════

class Dashboard {
public:
    static void show(const BookingEngine& engine,
                     const CustomerRegistry& registry,
                     const VehicleCatalog& catalog) {
        UI::sectionHeader("SYSTEM ANALYTICS");
        std::cout << "\n";

        UI::keyValue("  Total Customers",  std::to_string(registry.count()), ANSI::BRIGHT_CYAN);
        UI::keyValue("  Total Bookings",   std::to_string(engine.bookingCount()), ANSI::BRIGHT_CYAN);
        UI::keyValue("  Lifetime Revenue", Utils::formatRupees(engine.totalRevenue()), ANSI::BRIGHT_GREEN);

        std::cout << "\n  " << ANSI::BOLD << "Fleet Overview\n" << ANSI::RESET;
        UI::tableHeader("Type", 10, "Models", 8, "Damage Challan", 16);
        for (const auto& vt : catalog.types()) {
            std::cout << "  " << ANSI::BRIGHT_WHITE << std::left << std::setw(12)
                      << (vt.icon + "  " + vt.typeName)
                      << std::setw(10) << vt.models.size()
                      << ANSI::BRIGHT_RED << Utils::formatRupees(vt.damageCharge)
                      << ANSI::RESET << "\n";
        }

        UI::sectionFooter();
    }
};

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 10 ── Main Application Controller
// ═════════════════════════════════════════════════════════════════════════════

class VeloceApp {
public:
    VeloceApp()
        : catalog_(VehicleCatalog::build()),
          engine_(catalog_) {}

    void run() {
        while (true) {
            UI::printBanner();
            renderMainMenu();

            UI::prompt("Select Option");
            int choice = Utils::getValidInt(1, 7);

            try {
                switch (choice) {
                    case 1: handleNewBooking();   break;
                    case 2: handleReturn();       break;
                    case 3: engine_.listBookings();
                            UI::pressEnter();     break;
                    case 4: engine_.lookupBooking();
                            UI::pressEnter();     break;
                    case 5: handleNewCustomer();  break;
                    case 6: Dashboard::show(engine_, registry_, catalog_);
                            UI::pressEnter();     break;
                    case 7: exitApp();            return;
                }
            } catch (const VRS::VRSException& ex) {
                UI::error(ex.what());
                UI::pressEnter();
            } catch (const std::exception& ex) {
                UI::error(std::string("Unexpected error: ") + ex.what());
                UI::pressEnter();
            }
        }
    }

private:
    void renderMainMenu() const {
        std::cout << ANSI::BOLD << ANSI::BRIGHT_WHITE
                  << "  MAIN MENU\n" << ANSI::RESET;
        UI::horizontalRule();
        UI::menuItem(1, "New Rental Booking",          "🚗");
        UI::menuItem(2, "Process Vehicle Return",      "🔄");
        UI::menuItem(3, "View All Bookings",           "📋");
        UI::menuItem(4, "Lookup Booking by ID",        "🔍");
        UI::menuItem(5, "Register New Customer",       "👤");
        UI::menuItem(6, "Analytics Dashboard",         "📊");
        UI::menuItem(7, "Exit System",                 "🚪");
        UI::horizontalRule();
    }

    void handleNewBooking() {
        if (registry_.count() == 0) {
            UI::info("No customers registered. Please register a customer first.");
            handleNewCustomer();
        }

        UI::sectionHeader("BOOKING — CUSTOMER SELECTION");
        std::cout << "\n";
        UI::menuItem(1, "Register New Customer");
        UI::menuItem(2, "Existing Customer (enter ID)");
        UI::prompt("Choose");
        int ch = Utils::getValidInt(1, 2);

        Customer* customer = nullptr;
        if (ch == 1) {
            customer = &registry_.registerCustomer();
        } else {
            UI::prompt("Customer ID");
            std::string cid = Utils::getLine();
            auto opt = registry_.find(cid);
            if (!opt) {
                UI::error("Customer ID [" + cid + "] not found. Register first.");
                customer = &registry_.registerCustomer();
            } else {
                customer = &opt->get();
            }
        }

        if (customer) engine_.createBooking(*customer);
        UI::pressEnter();
    }

    void handleReturn() {
        engine_.processReturn(registry_);
        UI::pressEnter();
    }

    void handleNewCustomer() {
        registry_.registerCustomer();
        UI::pressEnter();
    }

    void exitApp() const {
        ANSI::clearScreen();
        UI::boxTop();
        UI::blankBoxLine();
        UI::boxLineCenter("Thank you for using VELOCE", ANSI::BRIGHT_CYAN);
        UI::boxLineCenter("Premium Vehicle Rental Management", ANSI::DIM);
        UI::blankBoxLine();
        UI::boxBottom();
        std::cout << "\n";
    }

    VehicleCatalog    catalog_;
    CustomerRegistry  registry_;
    BookingEngine     engine_;
};

} // namespace VRS

// ═════════════════════════════════════════════════════════════════════════════
// SECTION 11 ── Entry Point
// ═════════════════════════════════════════════════════════════════════════════

int main() {
    try {
        VRS::VeloceApp app;
        app.run();
    } catch (const std::exception& ex) {
        std::cerr << "\n[FATAL] Unhandled exception: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
