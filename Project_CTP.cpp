#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <windows.h> 
#include <unistd.h>
#include <cmath>

//zmina koloru tekstu w terminalu
#define RED_TEXT     "\x1B[31m"
#define GREEN_TEXT   "\x1B[32m"
#define RESET_TEXT   "\x1B[0m"

using namespace std;


class Transaction {
public:
    Transaction() : info(""), amount(0.00), success(true) {}

    Transaction(const string& info, double amount, bool success = true)
        : info(info), amount(amount), success(success) {}

    const string& getInfo() const { return info; }
    double getAmount() const { return amount; }
    bool isSuccess() const { return success; }

private:
    string info; // kupno lub sprzedaż, jakiej kryptowaluty, ile jej
    double amount;
    bool success; //zwraca wartosc czy tranzakcja sie udala
};

class UserAccount {
public:
    UserAccount(const string& name, double balance)
     : username(name), balance(balance), bitcoinAmount(0.00), dogecoinAmount(0.00), numTransactions(0) {}

    const string& getUsername() const { return username; }
    double getBalance() const { return balance; }
    double getBitcoinAmount() const { return bitcoinAmount; }
    double getDogecoinAmount() const { return dogecoinAmount; }
    const vector<Transaction>& getUserTransactions() const { return transactions; } //przechowuje tranzakcje wykonane przez uzytkownika

    void ShowMoney();
    void DepositMoney(double amount);
    bool WithdrawMoney(double amount);
    bool BuyCrypto(const string& cryptoName, double amount, double price);
    bool SellCrypto(const string& cryptoName, double amount, double price);
    void CheckTransactions() const;
    double CalculateTotalEquity(double bitcoinPrice, double dogecoinPrice) const; //dodaje to co ma w portfelu + kryptowluty po aktualnym kursie

private:
    string username;
    double balance;
    double bitcoinAmount;
    double dogecoinAmount;
    vector<Transaction> transactions;
    int numTransactions;
};

//Funckjca dodająca pieniądze na konto
void UserAccount::DepositMoney(double amount) {
    balance += amount;
    transactions.push_back(Transaction("Deposit", amount));
    numTransactions++;
    cout << GREEN_TEXT << "Successful deposit" << RESET_TEXT << endl;
}

//Funkcja wypłacająca pieniądze z konta
bool UserAccount::WithdrawMoney(double amount) {
    if (amount <= balance) {
        balance -= amount;
        transactions.push_back(Transaction("Withdrawal ", amount));
        numTransactions++;
        return true;
    } else {
        cout << RED_TEXT << "Not enough money" << RESET_TEXT << endl;
        return false;
    }
}

//Funckja pokazująca ilość pieniędzy na koncie
void UserAccount::ShowMoney() {
    cout << "You have: " << balance << " $" << endl;
}

//Funckcja pozwalająca na zakup cryptowalut
bool UserAccount::BuyCrypto(const string& cryptoName, double amount, double price) {
    double cost = amount * price;
    cost = round(cost * 100.0) / 100.0;
    if (cost <= balance) {
        balance -= cost;
        if (cryptoName == "Bitcoin") {
            bitcoinAmount += amount;
        } else if (cryptoName == "Dogecoin") {
            dogecoinAmount += amount;
        }
        transactions.push_back(Transaction("Buy " + to_string(amount) + " " + cryptoName, cost));
        numTransactions++;
        return true;
    } else {
        return false;
    }
}

//Funkcja pozwalająca na sprzedaż cryptowalut
bool UserAccount::SellCrypto(const string& cryptoName, double amount, double price) {
    if (cryptoName == "Bitcoin" && amount <= bitcoinAmount) {
        double earn = amount * price;
        earn = round(earn * 100.0) / 100.0;
        balance += earn;
        bitcoinAmount -= amount;
        transactions.push_back(Transaction("Sell " + to_string(amount) + " " + cryptoName, earn));
        numTransactions++;
        return true;
    } else if (cryptoName == "Dogecoin" && amount <= dogecoinAmount) {
        double earn = amount * price;
        earn = round(earn * 100.0) / 100.0;
        balance += earn;
        dogecoinAmount -= amount;
        transactions.push_back(Transaction("Sell " + to_string(amount) + " " + cryptoName, earn));
        numTransactions++;
        return true;
    } else {
        return false;
    }
}

//Funkcja wyświetlająca wykonane tranzakcje
void UserAccount::CheckTransactions() const {
    cout << "Transaction History: " << endl;
    for (int i = 0; i < numTransactions; i++) {
        cout << transactions[i].getInfo() << " " << setprecision(2) << fixed << transactions[i].getAmount() << " $" << endl;
    }
}

//Funkcja licząca całkowity hajs
double UserAccount::CalculateTotalEquity(double bitcoinPrice, double dogecoinPrice) const {
    double totalEquity = balance + (bitcoinAmount * bitcoinPrice) + (dogecoinAmount * dogecoinPrice);
    return totalEquity;
}


class Crypto {
public:
    Crypto(const string& name, double price) : name(name), price(price) {
            lastPrices = new double[historySize];
            for (int i = 0; i < historySize; i++) {
                lastPrices[i] = price;
            }
    }

    ~Crypto() {
        delete[] lastPrices; //destruktor klasy zwalniający pamięć zaalokowaną przez tablice
    }

    const string& getName() const { return name; }
    double getPrice() const { return price; }
    void setPrice(double newPrice) {price = newPrice;}
    double getLastPrice() const { return lastPrices[historyIndex - 1];} 
    void updatePrice(double newPrice);
    double percentageChange(int days) const;

private:
    static const int historySize = 10;
    string name;
    double price;
    double* lastPrices;
    int historyIndex = 0;

    void shiftHistory(); 
};

//Funckja aktualizująca cenę kryptowalut po zmianie dnia
void Crypto::updatePrice(double newPrice) {
    shiftHistory();
    price = newPrice;
    lastPrices[historyIndex] = newPrice;
}
//Funckja przejścia do kolejnego dnia
void Crypto::shiftHistory() {
    historyIndex = (historyIndex + 1) % historySize;
}

//Funckja obliczająca procent roznicy miedzy kursami z kolejnych dni
double Crypto::percentageChange(int days) const {
    if (days < 2) {
        return 0.0;
    }

    double priceBefore = lastPrices[(days - 2) % historySize];
    priceBefore = round(priceBefore * 100.0) / 100.0;
    double priceTemp = round(price * 100.0) / 100.0;
    double percent = ((priceTemp - priceBefore) / priceBefore) * 100;
    percent = ceil(percent * 100.0) / 100.0;
    return percent;
}


class Bot {
public:
    Bot(UserAccount& user,  Crypto* crypto, int numCryptos) : user(user), crypto(crypto), numCryptos(numCryptos), botUser(user), botTransactions() {}

    void symulation();
    void showBotTransactions();
    const vector<Transaction>& getBotTransactions() const { return botTransactions; } // przechowuje tranzakcje wykonane przez bota
    double CalculateTotalBotEquity(double bitcoinPrice, double dogecoinPrice) const; // dodaje to co ma w portfelu + kryptowluty po aktualnym kursie

private:
    const UserAccount& user;
    Crypto* crypto;
    int numCryptos;
    UserAccount botUser;
    vector<Transaction> botTransactions;

    double getRandomAmount();
    bool shouldBuy(const string& cryptoName);
    bool shouldSell(const string& cryptoName);
    void justBuy(const string& cryptoName, double amount, double price);
    void justSell(const string& cryptoName, double amount, double price);
    
};

//Funkcja symulująca zadania wykonywane przez bota pod konkretnymi warunkami
void Bot::symulation() {
    for (int i = 0; i < numCryptos; i++) {
        string cryptoName = crypto[i].getName();

        if(shouldBuy(cryptoName)) {
            double budget = botUser.getBalance() * 0.15; // 15% wszystkich dostępnych pieniędzy
            double buyAmount = min( budget / crypto[i].getPrice(), getRandomAmount());
            justBuy(cryptoName, buyAmount, crypto[i].getPrice());
        }

        if(shouldSell(cryptoName)) {
            if (cryptoName == "Bitcoin" && botUser.getBitcoinAmount() != 0) {
                double sellAmount = 0.2 * botUser.getBitcoinAmount();
                justSell(cryptoName, sellAmount, crypto[i].getPrice());
            }
            if (cryptoName == "Dogecoin" && botUser.getDogecoinAmount() != 0) {
                double sellAmount = 0.6 * botUser.getDogecoinAmount();
                justSell(cryptoName, sellAmount, crypto[i].getPrice());
            }
        }
    }    
}

//Funckja która wykonuje zakup kryptowaluty przez bota
void Bot::justBuy(const string& cryptoName, double amount, double price){
    botUser.BuyCrypto(cryptoName, amount, price);
    double cost = amount * price;
    cost = round(cost * 100.0) / 100.0;
    botTransactions.push_back(Transaction("Buy " + to_string(amount) + " " + cryptoName, cost));
}
    
//Funkcja sprzedaży kryptowaluty przez bota    
void Bot::justSell(const string& cryptoName, double amount, double price){
  if (cryptoName == "Bitcoin" && botUser.getBitcoinAmount() >= amount) {
        botUser.SellCrypto(cryptoName, amount, price);
        double cost = amount * price;
        cost = round(cost * 100.0) / 100.0;
        botTransactions.push_back(Transaction("Sell " + to_string(amount) + " " + cryptoName, cost));
    } else if (cryptoName == "Dogecoin" && botUser.getDogecoinAmount() >= amount) {
        botUser.SellCrypto(cryptoName, amount, price);
        double cost = amount * price;
        cost = round(cost * 100.0) / 100.0;
        botTransactions.push_back(Transaction("Sell " + to_string(amount) + " " + cryptoName, cost));
    }
}

//Funkcja wyświetlająca tranzakcje wykonane przez bota
void Bot::showBotTransactions() {
    cout << "Bot transactions: " << endl;
    for (const Transaction& transaction : botTransactions) {
        cout << transaction.getInfo() << setprecision(2) << fixed << transaction.getAmount() << " $" << endl;
    }
}

//symuluje losową liczbę miedzy 0.1 a 10.00
double Bot::getRandomAmount() {
    return (rand() % 100 + 1) * 0.01;
}

//Funkcja sprawdzająca czy opłaca się kupić daną kryptowalute
bool Bot::shouldBuy(const string& cryptoName) {
    for (int i = 0; i < numCryptos; i++) {
        if (cryptoName == crypto[i].getName()) {
            return crypto[i].getPrice() > crypto[i].getLastPrice() * 1.01;
        }
    }
    return false;
}

//Funckja sprawdzająca czy opłaca się sprzedać daną kryptowalute
bool Bot::shouldSell(const string& cryptoName) {
    for (int i = 0; i < numCryptos; i++) {
        if (cryptoName == crypto[i].getName()) {
            return botUser.getBitcoinAmount() > 0 && crypto[i].getPrice() > crypto[i].getLastPrice() * 0.98;
        }
    }
    return false;
}

//Funkcja obliczjąca całkowity dorobek bota
double Bot::CalculateTotalBotEquity(double bitcoinPrice, double dogecoinPrice) const {
    double botEquity = botUser.getBalance(); 

    botEquity += botUser.getBitcoinAmount() * bitcoinPrice + botUser.getDogecoinAmount() * dogecoinPrice;

    return botEquity;
}


class Menu {
public:
    Menu(UserAccount& user,  Crypto* crypto, int numCryptos) : user(user), crypto(crypto), numCryptos(numCryptos), bot(user, crypto, numCryptos) {}

    void showMenu1();
    void showMenu2();
    void processChoice1(int choice);
    void processChoice2(int choice);
    void run1();
    void run2();
    void saveTransactions() const;

private:
    UserAccount& user;
    Crypto* crypto;
    Bot bot;
    int numCryptos;
    int numberOfDay = 1;

    void depositMoney();
    void withdrawMoney();
    void buyCrypto();
    void sellCrypto();
    void checkTransactions();
    void checkBotTransactions();
    void showTotalEquity();
    void showTotalEquityofBot();
    void updateCryptoPrices();
    void showPrices();
};

//Funkcja wyświetlająca menu dla gry bez bota
void Menu::showMenu1() {
    cout << "Crypto Trading Platform" << endl;
    cout << "It is your " << numberOfDay << ". in game" << endl;
    cout << "1. Deposit Money" << endl;
    cout << "2. Withdraw Money" << endl;
    cout << "3. Buy Crypto" << endl;
    cout << "4. Sell Crypto" << endl;
    cout << "5. Check Transactions" << endl;
    cout << "6. Total Equity" << endl;
    cout << "7. Next Day" << endl;
    cout << "0. Quit" << endl;
    cout << "Enter your choice: ";
}

//Funkcja wyświetlająca menu dla gry z bota
void Menu::showMenu2() {
    cout << "Crypto Trading Platform" << endl;
    cout << "It is your " << numberOfDay << ". in game" << endl;
    cout << "1. Buy Crypto" << endl;
    cout << "2. Sell Crypto" << endl;
    cout << "3. Check Transactions" << endl;
    cout << "4. Check Bot Transactions" << endl;
    cout << "5. Total Equity" << endl;
    cout << "6. Total Equity of bot" << endl;
    cout << "7. Prices of crypto" << endl;
    cout << "8. Next Day" << endl;
    cout << "0. Quit" << endl;
    cout << "Enter your choice: ";
}

//Funkcja zmieniająca cenę kryptowalut
void Menu::updateCryptoPrices() {
    for (int i=0; i < numCryptos; ++i) {
        double curr = crypto[i].getPrice();
        double change = curr * (0.05 * (2.0 * rand() / RAND_MAX -1.0));
        double newP = curr + change;
        crypto[i].setPrice(newP);
        crypto[i].updatePrice(newP);
    }
    numberOfDay = numberOfDay + 1;
    cout << "The prices have been changed" << endl;
    cout << endl;
}

//Funkcja wykonująca wybór zadania w grze bez bota
void Menu::processChoice1(int choice) {
    switch (choice) {
    case 1:
        depositMoney();
        break;
    case 2:
        withdrawMoney();
        break;
    case 3:
        buyCrypto();
        break;
    case 4:
        sellCrypto();
        break;
    case 5:
        checkTransactions();
        break;
    case 6:
        showTotalEquity();
        break;
    case 7:
        updateCryptoPrices();
        break;
    case 0:
        saveTransactions();
        cout << "Goodbye!" << endl;
        break;
    default:
        cout << "Wrong choice. Please try again." << endl;
    }
}

//Funkcja wykonująca wybór zadania w grze z botem
void Menu::processChoice2(int choice) {
    switch (choice) {
    case 1:
        buyCrypto();
        break;
    case 2:
        sellCrypto();
        break;
    case 3:
        checkTransactions();
        break;
    case 4:
        checkBotTransactions();
        break;
    case 5:
        showTotalEquity();
        break;
    case 6:
        showTotalEquityofBot();
        break;
    case 7:
        showPrices();
        break;
    case 8:
        updateCryptoPrices();
        break;
    case 0:
        saveTransactions();
        cout << "Goodbye!" << endl;
        break;
    default:
        cout << "Wrong choice. Please try again." << endl;
    }
}

//Funkcja wyświetlająca działanie wpłaty
void Menu::depositMoney() {
    double depositAmount;
    cout << "Enter the amount to deposit [ $ ]: ";
    cin >> depositAmount;
    user.DepositMoney(depositAmount);
    cout << endl;
}

//Funckaj wyświetlająca działanie wypłaty
void Menu::withdrawMoney() {
    double withdrawAmount;
    cout << "Enter the amount to withdraw [ $ ]: ";
    cin >> withdrawAmount;
    bool success = user.WithdrawMoney(withdrawAmount);
    if (success) {
        cout << GREEN_TEXT << "Withdrawal successful." << RESET_TEXT << endl;
    }
    cout << endl;
}

//Funckcja wyświetlająca działanie kupna kryptowaluty
void Menu::buyCrypto() {
    int cryptoChoice;
    cout << "Select a crypto to buy: " << endl;
    for (int i = 0; i < numCryptos; ++i) {
        cout << i + 1 << ". " << crypto[i].getName() << endl;
    }
    cout << "Enter your choice: ";
    cin >> cryptoChoice;
    cout << endl;

    if (cryptoChoice >= 1 && cryptoChoice <= numCryptos) {
        string cryptoName = crypto[cryptoChoice - 1].getName();
        double cryptoPrice = crypto[cryptoChoice - 1].getPrice();
        double buyAmount;
        user.ShowMoney();
        cout << "The current price for 1 " << cryptoName << " is: " << cryptoPrice << " $" << endl;
        cout << "Enter the amount of " << cryptoName << " to buy: ";
        cin >> buyAmount;
        bool success = user.BuyCrypto(cryptoName, buyAmount, cryptoPrice);
        cout << endl;
        if (success) {
            cout << "Bought " << setprecision(5) << buyAmount << " " << cryptoName << " for " << setprecision(2) << fixed << buyAmount * cryptoPrice << " $" << endl;
            cout << GREEN_TEXT << "Purchase successful." << RESET_TEXT << endl;
        } else {
            cout << RED_TEXT << "You don't have enough money" << RESET_TEXT << endl;

        }
    } else {
        cout << "Wrong choice" << endl;
    }
    cout << endl;
}

//Funcja wyświetlająca działanie sprzedaży kryptowaluty
void Menu::sellCrypto() {
    int cryptoChoice;
    cout << "Select a crypto to sell: " << endl;
    for (int i = 0; i < numCryptos; ++i) {
        cout << i + 1 << ". " << crypto[i].getName() << endl;
    }
    cout << "Enter your choice: ";
    cin >> cryptoChoice;
    cout << endl;
    if (cryptoChoice >= 1 && cryptoChoice <= numCryptos) {
        string cryptoName = crypto[cryptoChoice - 1].getName();
        double cryptoPrice = crypto[cryptoChoice - 1].getPrice();
        double sellAmount;
        if (cryptoName == "Bitcoin") {
            cout << "You have: " << fixed << setprecision(5) << user.getBitcoinAmount()  << " Bitcoins" << endl;
        } else if (cryptoName == "Dogecoin") {
            cout << "You have: " << setprecision(5) << user.getDogecoinAmount() << " Dogecoins" << endl;
        }
        cout << "The current price for 1 " << cryptoName << " is: " << setprecision(2) << fixed << cryptoPrice << endl;
        cout << "Enter the amount of " << cryptoName << " to sell: ";
        cin >> sellAmount;
        bool success = user.SellCrypto(cryptoName, sellAmount, cryptoPrice);
        cout << endl;
        if (success) {
            cout << "Sold " + cryptoName << " " << setprecision(5) << sellAmount << " for " << setprecision(2) << fixed << sellAmount * cryptoPrice << " $" << endl;
            cout << GREEN_TEXT << "Sale successful." << RESET_TEXT << endl;
        } else {
            cout << RED_TEXT << "You don't have enough crypto" << RESET_TEXT << endl;
        }
    } else {
        cout << "Wrong choice" << endl;
    }
    cout << endl;
}

//Funkcja wyświetlająca działanie pokazania dokonanych tranzakcji przez użytkownika
void Menu::checkTransactions() {
    cout << user.getUsername() << " Transactions: " << endl;
    const vector<Transaction>& userTransactions = user.getUserTransactions();
    for (const Transaction& transaction : userTransactions) {
        cout << fixed << setprecision(2) << transaction.getInfo() << " " << setprecision(2) << fixed << transaction.getAmount() << " $" << endl;
    }
    cout << endl;
}

//Funkcja wyświetlająca działanie pokazania dokonanych tranzakcji przez bota
void Menu::checkBotTransactions() {
    cout << "Bot Transactions: " << endl;
    const vector<Transaction>& botTransactions = bot.getBotTransactions();
    for (const Transaction& transaction : botTransactions) {
        cout << transaction.getInfo() << " " << setprecision(2) << fixed << transaction.getAmount() << " $" << endl;
    }
    cout << endl;
}

//Funkcja wyświetlająca działanie wyświetlenia całości dorobku przez użytkownika
void Menu::showTotalEquity() {
    double bitcoinPrice = crypto[0].getPrice();
    double dogecoinPrice = crypto[1].getPrice();
    
    cout << "Total Equity for " << user.getUsername() << ": "
         << setprecision(2) << fixed << user.CalculateTotalEquity(bitcoinPrice, dogecoinPrice) << " $" << endl;
    cout << endl;
}

//Funkcja wyświetlająca działanie wyświetlenia całości dorobku przez bota
void Menu::showTotalEquityofBot() {
    double bitcoinPrice = crypto[0].getPrice();
    double dogecoinPrice = crypto[1].getPrice();
    
    cout << "Total Equity for Bot: "
         << setprecision(2) << fixed << bot.CalculateTotalBotEquity(bitcoinPrice, dogecoinPrice) << " $" << endl;
    cout << endl;
}

//Funkcja wykonująca działania i uruchamiania menu w grze bez bota
void Menu::run1() {
    srand(static_cast<unsigned>(time(nullptr)));
    int numberOfDay = 1;
    int choice;
    while (true) {
        showMenu1();
        cin >> choice;
        cout << endl;
        if (choice == 0) {
            saveTransactions();
            break;
        }
        processChoice1(choice);
    }
}

//Funkcja wykonująca działanie i uruchamianie menu w grze z botem
void Menu::run2() {
    srand(static_cast<unsigned>(time(nullptr)));
    int numberOfDay = 1;
    int choice;
    while (true) {
        showMenu2();
        cin >> choice;
        cout << endl;
        if (choice == 0) {
            saveTransactions();
            break;
        }
        bot.symulation();
        processChoice2(choice);
    }
}

////Funkcja wyświetlająca działanie pokazania kursu danych kryptowalut oraz procenta zmiany w stosunku do ostatniego kursu
void Menu::showPrices() {
    cout << "Crypto Values: " << endl;
    for (int i = 0; i < numCryptos; ++i) {
        double percent = crypto[i].percentageChange(numberOfDay);
        if (percent > 0) {
            cout << crypto[i].getName() << ": $" << setprecision(2) << fixed << crypto[i].getPrice() << " " << GREEN_TEXT << percent << "%" << RESET_TEXT << endl;

        } else if (percent < 0) {
            cout << crypto[i].getName() << ": $" << setprecision(2) << fixed << crypto[i].getPrice() << " " << RED_TEXT << percent << "%" << RESET_TEXT << endl;
        } else {
            cout << crypto[i].getName() << ": $" << setprecision(2) << fixed << crypto[i].getPrice() << " " << percent << "%" << endl;
        }
    }
    cout << endl;

}

//Funckaj zapisująca wszystkie tranzakcje do pliku typu csv
void Menu::saveTransactions() const {
 auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm localTime;
    localtime_s(&localTime, &now);
    
    ostringstream oss;
    oss << "transactions_" << put_time(&localTime, "%Y-%m-%d_%H-%M-%S") << ".csv";
    string filename = oss.str();

    ofstream file(filename, ios::app); 

    if (file.is_open()) {
        file << "Who" << "\t" <<"Crypto" << "\t" << "Price" << endl;  //postać zapisu

        const vector<Transaction>& userTransactions = user.getUserTransactions();
        for (const Transaction& transaction : userTransactions) {
            file << user.getUsername() << "\t" << transaction.getInfo() << "\t" << transaction.getAmount() << "$" << endl;
        }

        const vector<Transaction>& botTransactions = bot.getBotTransactions();
        for (const Transaction& transaction : botTransactions) {
            file << "Bot" << "\t" << transaction.getInfo() << "\t" << transaction.getAmount() << " $" << endl;
        }

        cout << "Transactions saved to " << filename << endl;
    }
}


int main() {
    string name;
    cout << "What's your name? ";
    cin >> name;
    cout << "Hello " << name << " :) " << endl;
    cout << endl;


    UserAccount user(name, 10000.0); //Na start w potrfelu mamy 10k $
    
    Crypto crypto_info[] = {
        Crypto("Bitcoin", 28451.4),
        Crypto("Dogecoin", 1.01) //początkowe kursy
    };
    int numCryptos = 2;
    Bot bot(user, crypto_info, numCryptos);

    int numberOfDay = 1;

    int playMode; //wybór rodzaju gry
    cout << "Choose play mode:" << endl;
    cout << "1. Play yourself" << endl;
    cout << "2. Play with Bot" << endl;
    cout << "Enter your choice: ";
    cin >> playMode;
    cout << endl;
    
    Menu menu(user, crypto_info, numCryptos);

    srand(static_cast<unsigned>(time(nullptr)));

    if (playMode == 1) {
        menu.run1();
    } else if (playMode == 2) {
        menu.run2();
    }

    return 0;
}
