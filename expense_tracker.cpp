// Smart Digital Expense Tracker with Budget Prediction
// BCA I Semester - Lab Sheet 09
// Compile: g++ -std=c++11 expense_tracker.cpp -o expense_tracker

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <limits>
#include <ctime>
#include <cstdio>
using namespace std;

const int MAX_EXPENSES = 1000;

struct Expense {
    int id;
    string username, date, category, description;
    double amount;
};

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string readLine(const string &prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

bool validDate(const string &date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (int i = 0; i < 10; ++i)
        if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) return false;
    int month = stoi(date.substr(5, 2));
    int day = stoi(date.substr(8, 2));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

string currentMonth() {
    time_t now = time(0);
    tm *local = localtime(&now);
    char buffer[8];
    strftime(buffer, sizeof(buffer), "%Y-%m", local);
    return string(buffer);
}

int currentDay() {
    time_t now = time(0);
    tm *local = localtime(&now);
    return local->tm_mday;
}

int daysInCurrentMonth() {
    time_t now = time(0);
    tm *local = localtime(&now);
    int year = local->tm_year + 1900;
    int month = local->tm_mon + 1;
    const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0)))
        return 29;
    return days[month - 1];
}

bool usernameExists(const string &username) {
    ifstream file("users.txt");
    string u, p;
    while (file >> u >> p)
        if (u == username) return true;
    return false;
}

bool registerUser() {
    cout << "\n--- User Registration ---\n";
    string username = readLine("Create username (no spaces): ");
    string password = readLine("Create password (no spaces): ");
    if (username.empty() || password.empty() ||
        username.find(' ') != string::npos || password.find(' ') != string::npos) {
        cout << "Username and password must be non-empty and contain no spaces.\n";
        return false;
    }
    if (usernameExists(username)) {
        cout << "Username already exists.\n";
        return false;
    }
    ofstream file("users.txt", ios::app);
    file << username << ' ' << password << '\n';
    cout << "Registration successful. Please log in.\n";
    return true;
}

bool loginUser(string &loggedInUser) {
    cout << "\n--- Login ---\n";
    string username = readLine("Username: ");
    string password = readLine("Password: ");
    ifstream file("users.txt");
    string u, p;
    while (file >> u >> p) {
        if (u == username && p == password) {
            loggedInUser = username;
            cout << "Login successful. Welcome, " << username << "!\n";
            return true;
        }
    }
    cout << "Invalid username or password.\n";
    return false;
}

int loadExpenses(Expense expenses[]) {
    ifstream file("expenses.txt");
    string line;
    int count = 0;
    while (getline(file, line) && count < MAX_EXPENSES) {
        stringstream ss(line);
        string idText, amountText;
        Expense e;
        if (!getline(ss, idText, '|') ||
            !getline(ss, e.username, '|') ||
            !getline(ss, e.date, '|') ||
            !getline(ss, e.category, '|') ||
            !getline(ss, amountText, '|') ||
            !getline(ss, e.description)) continue;
        try {
            e.id = stoi(idText);
            e.amount = stod(amountText);
        } catch (...) { continue; }
        expenses[count++] = e;
    }
    return count;
}

void saveExpenses(Expense expenses[], int count) {
    ofstream file("expenses.txt");
    for (int i = 0; i < count; ++i) {
        file << expenses[i].id << '|'
             << expenses[i].username << '|'
             << expenses[i].date << '|'
             << expenses[i].category << '|'
             << fixed << setprecision(2) << expenses[i].amount << '|'
             << expenses[i].description << '\n';
    }
}

double getBudget(const string &username, const string &month) {
    ifstream file("budgets.txt");
    string u, m;
    double amount;
    while (file >> u >> m >> amount)
        if (u == username && m == month) return amount;
    return 0.0;
}

void setBudget(const string &username) {
    string month = readLine("Enter month (YYYY-MM): ");
    if (month.size() != 7 || month[4] != '-') {
        cout << "Invalid month format.\n";
        return;
    }
    double amount;
    cout << "Enter monthly budget: ";
    if (!(cin >> amount) || amount <= 0) {
        clearInput();
        cout << "Enter a valid positive amount.\n";
        return;
    }
    clearInput();

    ifstream in("budgets.txt");
    ofstream out("budgets_temp.txt");
    string u, m;
    double oldAmount;
    bool updated = false;
    while (in >> u >> m >> oldAmount) {
        if (u == username && m == month) {
            out << u << ' ' << m << ' ' << amount << '\n';
            updated = true;
        } else out << u << ' ' << m << ' ' << oldAmount << '\n';
    }
    if (!updated) out << username << ' ' << month << ' ' << amount << '\n';
    in.close(); out.close();
    remove("budgets.txt");
    rename("budgets_temp.txt", "budgets.txt");
    cout << "Budget saved for " << month << ".\n";
}

void budgetStatus(const string &username, const string &month, double spent) {
    double budget = getBudget(username, month);
    if (budget <= 0) {
        cout << "No budget has been set for " << month << ".\n";
        return;
    }
    cout << "Budget: Rs. " << fixed << setprecision(2) << budget
         << " | Spent: Rs. " << spent
         << " | Remaining: Rs. " << budget - spent << '\n';
    if (spent > budget) cout << "WARNING: Monthly budget exceeded!\n";
    else if (spent >= budget * 0.80) cout << "Warning: You have used at least 80% of your budget.\n";
    else cout << "You are within your budget.\n";
}

void addExpense(const string &username, Expense expenses[], int &count) {
    if (count >= MAX_EXPENSES) {
        cout << "Expense storage is full.\n";
        return;
    }
    Expense e;
    e.id = 1;
    for (int i = 0; i < count; ++i)
        if (expenses[i].id >= e.id) e.id = expenses[i].id + 1;
    e.username = username;
    cout << "\n--- Add Daily Expense ---\n";
    e.date = readLine("Date (YYYY-MM-DD): ");
    if (!validDate(e.date)) {
        cout << "Invalid date format or date values.\n";
        return;
    }
    cout << "Categories: Food, Travel, Shopping, Education, Bills, Health, Other\n";
    e.category = readLine("Category: ");
    e.description = readLine("Description: ");
    cout << "Amount (Rs.): ";
    if (!(cin >> e.amount) || e.amount <= 0) {
        clearInput();
        cout << "Enter a valid positive amount.\n";
        return;
    }
    clearInput();
    if (e.category.empty() || e.description.find('|') != string::npos ||
        e.category.find('|') != string::npos || e.username.find('|') != string::npos) {
        cout << "Category/description cannot be empty or contain |.\n";
        return;
    }
    expenses[count++] = e;
    saveExpenses(expenses, count);
    cout << "Expense added successfully. ID: " << e.id << '\n';

    string month = e.date.substr(0, 7);
    double spent = 0;
    for (int i = 0; i < count; ++i)
        if (expenses[i].username == username && expenses[i].date.substr(0, 7) == month)
            spent += expenses[i].amount;
    budgetStatus(username, month, spent);
}

void viewHistory(const string &username, Expense expenses[], int count) {
    cout << "\n--- Expense History ---\n";
    bool found = false;
    // Sort the current user's expenses by date (ascending).
    Expense own[MAX_EXPENSES];
    int n = 0;
    for (int i = 0; i < count; ++i)
        if (expenses[i].username == username) own[n++] = expenses[i];

    for (int i = 0; i < n - 1; ++i)
        for (int j = 0; j < n - i - 1; ++j)
            if (own[j].date > own[j + 1].date) {
                Expense temp = own[j]; own[j] = own[j + 1]; own[j + 1] = temp;
            }

    cout << left << setw(6) << "ID" << setw(13) << "Date"
         << setw(16) << "Category" << setw(12) << "Amount" << "Description\n";
    for (int i = 0; i < n; ++i) {
        found = true;
        cout << left << setw(6) << own[i].id << setw(13) << own[i].date
             << setw(16) << own[i].category << setw(12)
             << fixed << setprecision(2) << own[i].amount << own[i].description << '\n';
    }
    if (!found) cout << "No expenses found.\n";
}

void categoryReport(const string &username, Expense expenses[], int count) {
    string month = readLine("Enter month (YYYY-MM): ");
    double totals[7] = {0};
    const string categories[7] = {"Food","Travel","Shopping","Education","Bills","Health","Other"};
    double overall = 0;
    for (int i = 0; i < count; ++i) {
        if (expenses[i].username == username && expenses[i].date.substr(0, 7) == month) {
            overall += expenses[i].amount;
            bool matched = false;
            for (int j = 0; j < 7; ++j)
                if (expenses[i].category == categories[j]) {
                    totals[j] += expenses[i].amount;
                    matched = true;
                }
            if (!matched) totals[6] += expenses[i].amount;
        }
    }
    cout << "\n--- Category-wise Expense: " << month << " ---\n";
    for (int i = 0; i < 7; ++i)
        cout << left << setw(12) << categories[i] << "Rs. "
             << fixed << setprecision(2) << totals[i] << '\n';
    cout << "Total: Rs. " << overall << '\n';
}

void monthlyReport(const string &username, Expense expenses[], int count) {
    string month = readLine("Enter month (YYYY-MM): ");
    double total = 0;
    int entries = 0;
    for (int i = 0; i < count; ++i)
        if (expenses[i].username == username && expenses[i].date.substr(0, 7) == month) {
            total += expenses[i].amount;
            ++entries;
        }
    cout << "\n--- Monthly Expense Report: " << month << " ---\n";
    cout << "Number of expenses: " << entries << '\n';
    cout << "Total expense: Rs. " << fixed << setprecision(2) << total << '\n';
    budgetStatus(username, month, total);
}

void predictExpense(const string &username, Expense expenses[], int count) {
    string month = readLine("Enter month (YYYY-MM): ");
    double spent = 0;
    for (int i = 0; i < count; ++i)
        if (expenses[i].username == username && expenses[i].date.substr(0, 7) == month)
            spent += expenses[i].amount;

    int day = currentDay();
    int days = daysInCurrentMonth();
    // Prediction uses the current date only for the current month.
    if (month != currentMonth()) {
        cout << "Prediction is available for the current month (" << currentMonth() << ").\n";
        return;
    }
    double predicted = (spent / day) * days;
    cout << "\n--- Expense Prediction ---\n";
    cout << "Spent so far: Rs. " << fixed << setprecision(2) << spent << '\n';
    cout << "Estimated month-end expense: Rs. " << predicted << '\n';
    double budget = getBudget(username, month);
    if (budget > 0) {
        cout << "Monthly budget: Rs. " << budget << '\n';
        if (predicted > budget) cout << "Prediction: Your budget may be exceeded.\n";
        else cout << "Prediction: You are likely to stay within your budget.\n";
    } else cout << "Set a monthly budget to compare the prediction.\n";
}

void searchExpense(const string &username, Expense expenses[], int count) {
    string key = readLine("Enter expense ID, date, category or description keyword: ");
    bool found = false;
    for (int i = 0; i < count; ++i) {
        if (expenses[i].username == username) {
            string all = to_string(expenses[i].id) + " " + expenses[i].date + " " +
                         expenses[i].category + " " + expenses[i].description;
            if (all.find(key) != string::npos) {
                cout << "ID: " << expenses[i].id << " | " << expenses[i].date
                     << " | " << expenses[i].category << " | Rs. "
                     << fixed << setprecision(2) << expenses[i].amount
                     << " | " << expenses[i].description << '\n';
                found = true;
            }
        }
    }
    if (!found) cout << "No matching expense found.\n";
}

void deleteExpense(const string &username, Expense expenses[], int &count) {
    int id;
    cout << "Enter expense ID to delete: ";
    if (!(cin >> id)) { clearInput(); cout << "Invalid ID.\n"; return; }
    clearInput();
    for (int i = 0; i < count; ++i) {
        if (expenses[i].username == username && expenses[i].id == id) {
            for (int j = i; j < count - 1; ++j) expenses[j] = expenses[j + 1];
            --count;
            saveExpenses(expenses, count);
            cout << "Expense deleted successfully.\n";
            return;
        }
    }
    cout << "Expense ID not found.\n";
}

void userMenu(const string &username) {
    Expense expenses[MAX_EXPENSES];
    int count = loadExpenses(expenses);
    int choice;
    do {
        cout << "\n========== SMART DIGITAL EXPENSE TRACKER ==========\n"
             << "Logged in as: " << username << "\n"
             << "1. Add Daily Expense\n"
             << "2. View Expense History\n"
             << "3. Category-wise Expense\n"
             << "4. Monthly Expense Report\n"
             << "5. Set Monthly Budget\n"
             << "6. Budget Warning / Status\n"
             << "7. Expense Prediction\n"
             << "8. Search Expense\n"
             << "9. Delete Expense\n"
             << "0. Logout\n"
             << "Enter choice: ";
        if (!(cin >> choice)) { clearInput(); cout << "Please enter a number.\n"; continue; }
        clearInput();
        switch (choice) {
            case 1: addExpense(username, expenses, count); break;
            case 2: viewHistory(username, expenses, count); break;
            case 3: categoryReport(username, expenses, count); break;
            case 4: monthlyReport(username, expenses, count); break;
            case 5: setBudget(username); break;
            case 6: {
                string month = readLine("Enter month (YYYY-MM): ");
                double total = 0;
                for (int i = 0; i < count; ++i)
                    if (expenses[i].username == username && expenses[i].date.substr(0, 7) == month)
                        total += expenses[i].amount;
                budgetStatus(username, month, total);
                break;
            }
            case 7: predictExpense(username, expenses, count); break;
            case 8: searchExpense(username, expenses, count); break;
            case 9: deleteExpense(username, expenses, count); break;
            case 0: cout << "Logged out successfully.\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);
}

int main() {
    int choice;
    string username;
    cout << "===============================================\n"
         << "       SMART DIGITAL EXPENSE TRACKER\n"
         << "          WITH BUDGET PREDICTION\n"
         << "===============================================\n";
    do {
        cout << "\n1. Register\n2. Login\n0. Exit\nEnter choice: ";
        if (!(cin >> choice)) { clearInput(); cout << "Please enter a number.\n"; continue; }
        clearInput();
        switch (choice) {
            case 1: registerUser(); break;
            case 2:
                if (loginUser(username)) userMenu(username);
                break;
            case 0: cout << "Thank you for using the Expense Tracker.\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);
    return 0;
}
